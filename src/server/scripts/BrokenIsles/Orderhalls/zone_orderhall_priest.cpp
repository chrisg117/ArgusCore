/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Creature.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "GameTables.h"
#include "Item.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "QuestDef.h"
#include "SceneMgr.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include <span>
#include <string_view>

// How a priest comes to Netherlight Temple: back from the tomb of Tyr with the weapon, "Blade in Twilight" is
// handed in to Moira Thaurissan at the Antonidas Memorial in Dalaran, and Velen, who stands there with her and
// Alonsus Faol, gives "The Light and the Void". He walks ahead to the teleport pad in the middle of the city
// and, below it in Aegwynn's Gallery, on to the portal to the temple. What is said and when, and the way he
// takes, are from a video of the retail quests.
enum PriestOrderHallIntro
{
    QUEST_BLADE_IN_TWILIGHT                 = 40710,
    QUEST_THE_LIGHT_AND_THE_VOID            = 40938,

    // quest_objectives.ID
    OBJECTIVE_FOLLOW_VELEN_TO_THE_PORTAL    = 281419,

    NPC_MOIRA_THAURISSAN                    = 101310,
    NPC_PROPHET_VELEN                       = 101313,
    NPC_ALONSUS_FAOL                        = 102363,
    NPC_PROPHET_VELEN_GALLERY               = 102462,
    NPC_KILL_CREDIT_RETURN_TO_FAOL          = 102285,
    NPC_KILL_CREDIT_FOUND_PORTAL            = 102466,

    SAY_FAOL_HAS_RETURNED                   = 0,
    SAY_FAOL_SPEAK_WITH_VELEN               = 1,

    SAY_MOIRA_SHOWS_TALENT                  = 0,

    SAY_VELEN_WELL_DONE                     = 0,
    SAY_VELEN_FOLLOW_ME                     = 1,
    SAY_VELEN_THE_NAARU_HELPED_US           = 2,

    SAY_VELEN_GALLERY_A_FALLEN_NAARU        = 0,
    SAY_VELEN_GALLERY_A_PRISON              = 1,

    ACTION_VELEN_INVITE_AND_LEAD            = 1,
    ACTION_VELEN_LEAD                       = 2
};

// From the memorial down the street to the middle of the city, and through the north-eastern passage of the
// building there to its teleport pad
Position const VelenCityPath[] =
{
    { -709.5f,  4483.5f,  730.11f },
    { -723.0f,  4481.0f,  731.37f },
    { -737.0f,  4476.0f,  731.65f },
    { -756.0f,  4468.5f,  733.68f },
    { -770.0f,  4461.0f,  734.86f },
    { -789.0f,  4447.5f,  736.98f },
    { -806.0f,  4441.0f,  738.20f },
    { -820.0f,  4448.5f,  737.05f },
    { -834.0f,  4458.5f,  735.55f },
    { -844.65f, 4467.65f, 735.88f }
};

// He speaks of the naaru when he is here on it
constexpr std::size_t VelenCityPathPointOfTheNaaru = 2;

// In Aegwynn's Gallery: from the pad's room down the hall and the stairs into the Portrait Room, across it
// and down again to the room with the portal
Position const VelenGalleryStartPosition = { -786.5f, 4421.0f, 602.46f, 2.45f };

Position const VelenGalleryPath[] =
{
    { -799.0f,  4432.3f,  598.67f },
    { -812.9f,  4437.3f,  598.50f },
    { -823.1f,  4450.0f,  593.30f },
    { -829.0f,  4459.5f,  588.96f },
    { -844.65f, 4467.65f, 588.85f },
    { -855.7f,  4476.4f,  588.96f },
    { -863.2f,  4480.0f,  587.32f },
    { -871.5f,  4489.5f,  580.24f },
    { -878.3f,  4495.4f,  580.20f }
};

// The middle of the Portrait Room, where he speaks of the prison
constexpr std::size_t VelenGalleryPathPointOfThePrison = 4;

Position const NetherlightTemplePortalPosition = { -882.45f, 4498.21f, 580.31f };

// Whether the player still has to be led to the portal
bool NeedsVelenAsGuide(Player const* player)
{
    return player->GetQuestStatus(QUEST_THE_LIGHT_AND_THE_VOID) == QUEST_STATUS_INCOMPLETE
        && !player->IsQuestObjectiveComplete(QUEST_THE_LIGHT_AND_THE_VOID, OBJECTIVE_FOLLOW_VELEN_TO_THE_PORTAL);
}

// A Velen whom one player alone sees, and who walks ahead of that player from point to point. He waits at a
// point for a player who has fallen behind, and is gone when the player is.
struct npc_priest_order_hall_velen_guide : public ScriptedAI
{
    // He goes on from a point when the player is within the first distance, or within the second after having
    // waited; a player further away than the third has left him
    static constexpr float GoOnDistance = 35.0f;
    static constexpr float GoOnAfterWaitingDistance = 15.0f;
    static constexpr float LostDistance = 150.0f;

    npc_priest_order_hall_velen_guide(Creature* creature, std::span<Position const> path) : ScriptedAI(creature),
        _path(path), _nextPoint(0), _standsAtPoint(false), _hasWaited(false) { }

    bool IsGuide() const { return me->IsPrivateObject(); }

    Player* GetGuidedPlayer() const { return ObjectAccessor::GetPlayer(*me, me->GetPrivateObjectOwner()); }

    void StartWalk()
    {
        me->SetWalk(true);
        _nextPoint = 0;
        MoveToNextPoint();
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE || id != _nextPoint)
            return;

        OnPointReached(_nextPoint);
        ++_nextPoint;
        _standsAtPoint = true;
        _hasWaited = false;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!IsGuide())
            return;

        _scheduler.Update(diff);

        Player* player = GetGuidedPlayer();
        if (!player || me->GetExactDist(player) > LostDistance)
        {
            me->DespawnOrUnsummon();
            return;
        }

        if (!_standsAtPoint)
            return;

        if (_nextPoint == _path.size())
            OnPathEnded(player);
        else if (me->GetExactDist(player) <= (_hasWaited ? GoOnAfterWaitingDistance : GoOnDistance))
            MoveToNextPoint();
        else
            _hasWaited = true;
    }

protected:
    virtual void OnPointReached(std::size_t /*point*/) { }

    // Called on every update while he stands at the last point
    virtual void OnPathEnded(Player* /*player*/) { }

    TaskScheduler _scheduler;

private:
    void MoveToNextPoint()
    {
        _standsAtPoint = false;
        me->GetMotionMaster()->MovePoint(_nextPoint, _path[_nextPoint]);
    }

    std::span<Position const> _path;
    std::size_t _nextPoint;
    bool _standsAtPoint;
    bool _hasWaited;
};

// 101313 - Prophet Velen, at the Antonidas Memorial. Who takes his quest gets a Velen of his own in his place,
// who leads the way to the teleport pad and takes it when the player has come up to him.
struct npc_prophet_velen_antonidas_memorial : public npc_priest_order_hall_velen_guide
{
    static constexpr float TakesThePadDistance = 6.0f;

    npc_prophet_velen_antonidas_memorial(Creature* creature) : npc_priest_order_hall_velen_guide(creature, VelenCityPath) { }

    static void SummonGuide(Creature* velen, Player* player, bool withInvitation)
    {
        if (TempSummon* guide = velen->SummonPersonalClone(velen->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, player))
            guide->AI()->DoAction(withInvitation ? ACTION_VELEN_INVITE_AND_LEAD : ACTION_VELEN_LEAD);
    }

    void JustAppeared() override
    {
        // The guide gives and takes no quests
        if (IsGuide())
            me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
    }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (!IsGuide() && quest->GetQuestId() == QUEST_THE_LIGHT_AND_THE_VOID)
            SummonGuide(me, player, true);
    }

    void DoAction(int32 action) override
    {
        if (!IsGuide())
            return;

        switch (action)
        {
            case ACTION_VELEN_INVITE_AND_LEAD:
                _scheduler.Schedule(500ms, [this](TaskContext /*context*/)
                {
                    Talk(SAY_VELEN_FOLLOW_ME, GetGuidedPlayer());
                });
                _scheduler.Schedule(4500ms, [this](TaskContext /*context*/)
                {
                    StartWalk();
                });
                break;
            // For a player who lost him on the way and has come back to the memorial
            case ACTION_VELEN_LEAD:
                _scheduler.Schedule(1s, [this](TaskContext /*context*/)
                {
                    StartWalk();
                });
                break;
            default:
                break;
        }
    }

protected:
    void OnPointReached(std::size_t point) override
    {
        if (point == VelenCityPathPointOfTheNaaru)
            Talk(SAY_VELEN_THE_NAARU_HELPED_US, GetGuidedPlayer());
    }

    void OnPathEnded(Player* player) override
    {
        if (me->GetExactDist(player) <= TakesThePadDistance)
            me->DespawnOrUnsummon();
    }
};

// 102462 - Prophet Velen, in Aegwynn's Gallery: he is there for a player who comes down by the pad, and goes
// ahead to the portal to Netherlight Temple
struct npc_prophet_velen_aegwynns_gallery : public npc_priest_order_hall_velen_guide
{
    static constexpr float AtThePortalDistance = 20.0f;

    npc_prophet_velen_aegwynns_gallery(Creature* creature) : npc_priest_order_hall_velen_guide(creature, VelenGalleryPath), _credited(false) { }

    void JustAppeared() override
    {
        if (!IsGuide())
            return;

        _scheduler.Schedule(1500ms, [this](TaskContext /*context*/)
        {
            StartWalk();
        });
        _scheduler.Schedule(6s, [this](TaskContext /*context*/)
        {
            Talk(SAY_VELEN_GALLERY_A_FALLEN_NAARU, GetGuidedPlayer());
        });
    }

protected:
    void OnPointReached(std::size_t point) override
    {
        if (point == VelenGalleryPathPointOfThePrison)
            Talk(SAY_VELEN_GALLERY_A_PRISON, GetGuidedPlayer());
        else if (point + 1 == std::size(VelenGalleryPath))
            me->SetFacingTo(me->GetAbsoluteAngle(NetherlightTemplePortalPosition));
    }

    void OnPathEnded(Player* player) override
    {
        if (_credited || me->GetExactDist(player) > AtThePortalDistance)
            return;

        _credited = true;
        player->KilledMonsterCredit(NPC_KILL_CREDIT_FOUND_PORTAL);
    }

private:
    bool _credited;
};

// 101310 - Moira Thaurissan, at the Antonidas Memorial
struct npc_moira_thaurissan_antonidas_memorial : public ScriptedAI
{
    npc_moira_thaurissan_antonidas_memorial(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestReward(Player* player, Quest const* quest, LootItemType /*type*/, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() != QUEST_BLADE_IN_TWILIGHT)
            return;

        _scheduler.Schedule(2s, [this, playerGuid = player->GetGUID()](TaskContext /*context*/)
        {
            Player* player = ObjectAccessor::GetPlayer(*me, playerGuid);
            if (!player)
                return;

            if (Creature* faol = me->FindNearestCreatureWithOptions(20.0f, { .CreatureId = NPC_ALONSUS_FAOL }))
                faol->AI()->Talk(SAY_FAOL_SPEAK_WITH_VELEN, player);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// A priest who comes to the three at the memorial with the weapon has returned to them, which is the last
// thing "Blade in Twilight" asks for
struct at_dalaran_antonidas_memorial_priests : AreaTriggerAI
{
    static constexpr float SearchDistance = 30.0f;

    at_dalaran_antonidas_memorial_priests(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player)
            return;

        if (player->GetQuestStatus(QUEST_BLADE_IN_TWILIGHT) == QUEST_STATUS_INCOMPLETE)
        {
            // The quest counts a credit only when it is told whose end it was (see the scenario's script)
            player->KilledMonsterCredit(NPC_KILL_CREDIT_RETURN_TO_FAOL, player->GetGUID());
            if (player->GetQuestStatus(QUEST_BLADE_IN_TWILIGHT) != QUEST_STATUS_COMPLETE)
                return;

            ObjectGuid playerGuid = player->GetGUID();
            _scheduler.Schedule(500ms, [this, playerGuid](TaskContext /*context*/)
            {
                Speak(NPC_ALONSUS_FAOL, SAY_FAOL_HAS_RETURNED, playerGuid);
            });
            _scheduler.Schedule(4500ms, [this, playerGuid](TaskContext /*context*/)
            {
                Speak(NPC_MOIRA_THAURISSAN, SAY_MOIRA_SHOWS_TALENT, playerGuid);
            });
            _scheduler.Schedule(7s, [this, playerGuid](TaskContext /*context*/)
            {
                Speak(NPC_PROPHET_VELEN, SAY_VELEN_WELL_DONE, playerGuid);
            });
        }
        // Velen leads again a player who took his quest, lost him on the way to the pad and has come back
        else if (NeedsVelenAsGuide(player))
        {
            if (player->FindNearestCreatureWithOptions(250.0f, { .CreatureId = NPC_PROPHET_VELEN, .IgnorePhases = true, .PrivateObjectOwnerGuid = player->GetGUID() }))
                return;

            if (Creature* velen = at->FindNearestCreatureWithOptions(SearchDistance, { .CreatureId = NPC_PROPHET_VELEN, .IsSummon = false, .IgnorePhases = true }))
                npc_prophet_velen_antonidas_memorial::SummonGuide(velen, player, false);
        }
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void Speak(uint32 creatureId, uint8 text, ObjectGuid playerGuid) const
    {
        Player* player = ObjectAccessor::GetPlayer(*at, playerGuid);
        if (!player)
            return;

        if (Creature* speaker = at->FindNearestCreatureWithOptions(SearchDistance, { .CreatureId = creatureId, .IsSummon = false, .IgnorePhases = true }))
            speaker->AI()->Talk(text, player);
    }

    TaskScheduler _scheduler;
};

// Where the city's teleport pad puts one down in Aegwynn's Gallery: Velen is there for a priest he leads
struct at_dalaran_aegwynns_gallery_velen : AreaTriggerAI
{
    at_dalaran_aegwynns_gallery_velen(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || !NeedsVelenAsGuide(player))
            return;

        if (player->FindNearestCreatureWithOptions(250.0f, { .CreatureId = NPC_PROPHET_VELEN_GALLERY, .IgnorePhases = true, .PrivateObjectOwnerGuid = player->GetGUID() }))
            return;

        player->SummonCreature(NPC_PROPHET_VELEN_GALLERY, VelenGalleryStartPosition, TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, player->GetGUID());
    }
};

// 202324 - Teleport: Priest Order Hall
// The spell of the portal in Aegwynn's Gallery. Its first effect takes the player into a dungeon of the
// dungeon finder ("Priest Order Formation", the temple while it is still Saraka's prison), which the core
// does not do; the place is the one the spell has in `spell_target_position`. Its second effect is the
// quest's credit for taking the portal.
class spell_orderhall_priest_teleport_priest_order_hall : public SpellScript
{
    void HandleTeleport(SpellEffIndex effIndex)
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        // Velen is the player's own and can be lost; a player who found the portal without him has found it
        if (NeedsVelenAsGuide(player))
            player->KilledMonsterCredit(NPC_KILL_CREDIT_FOUND_PORTAL);

        if (SpellTargetPosition const* destination = sSpellMgr->GetSpellTargetPosition(GetSpellInfo()->Id, effIndex))
            player->TeleportTo(*destination);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_orderhall_priest_teleport_priest_order_hall::HandleTeleport, EFFECT_0, SPELL_EFFECT_TELEPORT_TO_LFG_DUNGEON);
    }
};

// The founding of the Conclave, which is the rest of "The Light and the Void". The portal leads into the
// temple while it is still Saraka's prison (map 1629), and nearly all that is seen there is a scene of the
// client's (1191, script package 1556 "Naaru Reborn"): Faol, Velen and Moira, the priests at their rituals,
// the void god and the naaru it becomes, what is said, and the darkness at the end. The scene tells the
// server when it has something to do; the server's own part is the quest and the player's spell.
enum PriestOrderFormation
{
    // quest_objectives.ID
    OBJECTIVE_BE_A_PART_OF_THE_CEREMONY     = 281633,

    NPC_ALONSUS_FAOL_TEMPLE                 = 102655,
    NPC_EFFECTS_BUNNY                       = 102528,

    SPELL_PRIEST_ORDER_FORMATION            = 202457, // plays the scene for as long as it is on the player
    SPELL_CAN_CAST_RITUAL_SPELL             = 202835, // the credit for having listened to Faol
    SPELL_DARK_DRAIN_SPELL_BAR              = 202749, // the ritual's spell on an extra button, by the weapon
    SPELL_DISC_PRIEST_SPELL_BAR             = 216416,
    SPELL_HOLY_PRIEST_SPELL_BAR             = 216419,
    SPELL_PRIEST_RITUAL_EVENT               = 202881, // the credit for the ritual; takes the button away
    SPELL_FAOLS_LIFE_GRIP                   = 202982,
    SPELL_TELEPORT_POST_ORDER_FORMATION     = 202491, // the last credit, and on to the temple as it is afterwards

    // SpellVisualKit.db2
    SPELL_VISUAL_KIT_MAIN_CIRCLE            = 63724,

    SAY_FAOL_TEMPLE_OVER_HERE               = 0
};

// creature.StringId of the Effects Bunny in the middle of the main circle
constexpr std::string_view OrderFormationRitualSite = "PriestOrderFormationRitualSite";

// Whether the Conclave is still to be founded for the player
bool NeedsOrderFormation(Player const* player)
{
    return player->GetQuestStatus(QUEST_THE_LIGHT_AND_THE_VOID) == QUEST_STATUS_INCOMPLETE
        && !player->IsQuestObjectiveComplete(QUEST_THE_LIGHT_AND_THE_VOID, OBJECTIVE_BE_A_PART_OF_THE_CEREMONY);
}

// The light on the main circle from when the ritual waits for the player's spell until it is done. The scene
// plays it on a bunny of its own (spell visual 52832, of which this kit is all there is), and in play it was
// not to be seen; so the server plays the same kit in the same place, on a real bunny.
void SetMainCircleLit(Player* player, bool lit)
{
    Creature* ritualSite = player->FindNearestCreatureWithOptions(250.0f, { .CreatureId = NPC_EFFECTS_BUNNY, .StringId = OrderFormationRitualSite });
    if (!ritualSite)
        return;

    if (lit)
        ritualSite->SendPlaySpellVisualKit(SPELL_VISUAL_KIT_MAIN_CIRCLE, 0, 0);
    else
        ritualSite->SendCancelSpellVisualKit(SPELL_VISUAL_KIT_MAIN_CIRCLE);
}

// Where the portal puts one down in the prison: the scene begins. It begins anew for a player who comes
// back to this place with the Conclave not founded, having left the temple or lost the scene.
struct at_netherlight_temple_prison_order_formation : AreaTriggerAI
{
    at_netherlight_temple_prison_order_formation(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || !NeedsOrderFormation(player))
            return;

        player->RemoveAurasDueToSpell(SPELL_PRIEST_ORDER_FORMATION);
        player->CastSpell(player, SPELL_PRIEST_ORDER_FORMATION, true);
    }
};

// 1191 - the scene of the Conclave's founding
class scene_priest_order_formation : public SceneScript
{
public:
    scene_priest_order_formation() : SceneScript("scene_priest_order_formation") { }

    void OnSceneTriggerEvent(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/, std::string const& triggerName) override
    {
        // Faol has said his piece at the main circle and waits for the player's spell
        if (triggerName == "ReadyForSpell")
        {
            player->CastSpell(player, SPELL_CAN_CAST_RITUAL_SPELL, true);
            player->CastSpell(player, GetRitualSpellBar(player), true);
            SetMainCircleLit(player, true);
        }
        // Faol, on his way to the dais, pulls the player after him
        else if (triggerName == "LifeGrip")
            player->CastSpell(player, SPELL_FAOLS_LIFE_GRIP, true);
        // The screen has gone dark
        else if (triggerName == "PortPlayer")
        {
            player->CastSpell(player, SPELL_TELEPORT_POST_ORDER_FORMATION, true);
            player->RemoveAurasDueToSpell(SPELL_PRIEST_ORDER_FORMATION);
        }
    }

    void OnSceneCancel(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        RemoveRitualSpellBars(player);
    }

    void OnSceneComplete(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        RemoveRitualSpellBars(player);
    }

private:
    static uint32 GetRitualSpellBar(Player const* player)
    {
        switch (player->GetPrimarySpecialization())
        {
            case ChrSpecialization::PriestDiscipline:
                return SPELL_DISC_PRIEST_SPELL_BAR;
            case ChrSpecialization::PriestHoly:
                return SPELL_HOLY_PRIEST_SPELL_BAR;
            default:
                return SPELL_DARK_DRAIN_SPELL_BAR;
        }
    }

    static void RemoveRitualSpellBars(Player* player)
    {
        SetMainCircleLit(player, false);
        player->RemoveAurasDueToSpell(SPELL_DARK_DRAIN_SPELL_BAR);
        player->RemoveAurasDueToSpell(SPELL_DISC_PRIEST_SPELL_BAR);
        player->RemoveAurasDueToSpell(SPELL_HOLY_PRIEST_SPELL_BAR);
    }
};

// 202747 - Dark Drain
// The shadow priest's part in the ritual, channelled for fourteen seconds at the void god. The void god is
// the scene's, so the spell's target is the effects bunny that stands in its place, and the aura's last
// tick would have that bunny cast "Priest Ritual Event" on itself: the ritual's credit for nobody. The
// player casts it when the channel has run its time.
class spell_orderhall_priest_dark_drain : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
    }

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Unit* caster = GetCaster();
        if (!caster)
            return;

        caster->CastSpell(caster, SPELL_PRIEST_RITUAL_EVENT, true);
        if (Player* player = caster->ToPlayer())
            SetMainCircleLit(player, false);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_orderhall_priest_dark_drain::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_orderhall_priest_dark_drain::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// Where one arrives in the temple as it is after the founding (map 1512): Faol calls from the main circle
// to a priest who has the quest to hand in
struct at_netherlight_temple_conclave_founded : AreaTriggerAI
{
    at_netherlight_temple_conclave_founded(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_THE_LIGHT_AND_THE_VOID) != QUEST_STATUS_COMPLETE)
            return;

        if (Creature* faol = at->FindNearestCreatureWithOptions(150.0f, { .CreatureId = NPC_ALONSUS_FAOL_TEMPLE }))
            faol->AI()->Talk(SAY_FAOL_TEMPLE_OVER_HERE, player);
    }
};

// The first things done in the temple, which make an artifact of the weapon. Faol sends the priest to Betild
// Deepanvil ("Artifacts Need Artificers"), who has made the Altar of Light and Shadow on the dais into the
// temple's forge and has it used once ("Empowering Your Artifact").
enum PriestOrderHallForge
{
    QUEST_ARTIFACTS_NEED_ARTIFICERS         = 41015,
    QUEST_EMPOWERING_YOUR_ARTIFACT          = 41017,

    NPC_BETILD_DEEPANVIL                    = 102709,
    NPC_SAARA                               = 111241,

    SAY_SAARA_THE_ALTAR_IS_COMPLETE         = 0,

    SAY_BETILD_A_MIRACLE_WORKER             = 0
};

// Around the altar: Saa'ra, from the main circle, and Betild speak of it when a priest whom Faol has sent to
// her comes up to it, once for each
struct at_netherlight_temple_altar_of_light_and_shadow : AreaTriggerAI
{
    at_netherlight_temple_altar_of_light_and_shadow(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player)
            return;

        // Only for one who has the quest and has not handed it in
        QuestStatus status = player->GetQuestStatus(QUEST_ARTIFACTS_NEED_ARTIFICERS);
        if (status != QUEST_STATUS_INCOMPLETE && status != QUEST_STATUS_COMPLETE)
            return;

        ObjectGuid playerGuid = player->GetGUID();
        if (!_heard.insert(playerGuid).second)
            return;

        if (Creature* saara = at->FindNearestCreatureWithOptions(150.0f, { .CreatureId = NPC_SAARA }))
            saara->AI()->Talk(SAY_SAARA_THE_ALTAR_IS_COMPLETE, player);

        _scheduler.Schedule(4500ms, [this, playerGuid](TaskContext /*context*/)
        {
            Player* player = ObjectAccessor::GetPlayer(*at, playerGuid);
            if (!player)
                return;

            if (Creature* betild = at->FindNearestCreatureWithOptions(30.0f, { .CreatureId = NPC_BETILD_DEEPANVIL }))
                betild->AI()->Talk(SAY_BETILD_A_MIRACLE_WORKER, player);
        });
    }

    void OnUpdate(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    GuidUnorderedSet _heard;
};

// 246320 - Altar of Light and Shadow
// The temple's forge. "Empowering Your Artifact" has the first trait bought at it, and that takes artifact
// power, of which the weapon has none when it is won. In the video of the retail quests the forge's window
// shows as much as the first trait costs when it is first opened, and nothing before it says where that came
// from; here the altar gives it, to a weapon that nothing has been bought for yet.
struct go_netherlight_temple_altar_of_light_and_shadow : GameObjectAI
{
    go_netherlight_temple_altar_of_light_and_shadow(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_EMPOWERING_YOUR_ARTIFACT) != QUEST_STATUS_INCOMPLETE)
            return false;

        Aura const* artifactAura = player->GetAura(ARTIFACTS_ALL_WEAPONS_GENERAL_WEAPON_EQUIPPED_PASSIVE);
        Item* artifact = artifactAura ? player->GetItemByGuid(artifactAura->GetCastItemGUID()) : nullptr;
        if (!artifact || artifact->GetTotalPurchasedArtifactPowers())
            return false;

        if (GtArtifactLevelXPEntry const* firstTrait = sArtifactLevelXPGameTable.GetRow(1))
        {
            uint64 cost = uint64(firstTrait->XP);
            uint64 artifactPower = artifact->GetUInt64Value(ITEM_FIELD_ARTIFACT_XP);
            if (artifactPower < cost)
                artifact->GiveArtifactXp(cost - artifactPower, nullptr, 0);
        }

        // The forge's window opens as it would
        return false;
    }
};

void AddSC_orderhall_priest()
{
    RegisterCreatureAI(npc_prophet_velen_antonidas_memorial);
    RegisterCreatureAI(npc_prophet_velen_aegwynns_gallery);
    RegisterCreatureAI(npc_moira_thaurissan_antonidas_memorial);
    RegisterAreaTriggerAI(at_dalaran_antonidas_memorial_priests);
    RegisterAreaTriggerAI(at_dalaran_aegwynns_gallery_velen);
    RegisterSpellScript(spell_orderhall_priest_teleport_priest_order_hall);
    RegisterAreaTriggerAI(at_netherlight_temple_prison_order_formation);
    new scene_priest_order_formation();
    RegisterSpellScript(spell_orderhall_priest_dark_drain);
    RegisterAreaTriggerAI(at_netherlight_temple_conclave_founded);
    RegisterAreaTriggerAI(at_netherlight_temple_altar_of_light_and_shadow);
    RegisterGameObjectAI(go_netherlight_temple_altar_of_light_and_shadow);
}
