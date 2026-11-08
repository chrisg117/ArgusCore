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
#include "ClassHall.h"
#include "DB2Stores.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "GameTables.h"
#include "Item.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "PlayerChoice.h"
#include "QuestDef.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

enum APactOfNecessityData
{
    QUEST_A_PACT_OF_NECESSITY               = 40715,
    QUEST_FROST_CHOSEN                      = 40723,

    PLAYERCHOICE_RESPONSE_FROST             = 2531,

    SPELL_FROST_CHOSEN                      = 200078,
    SPELL_FROST_DEATH_KNIGHT_QUEST_LAUNCH   = 190092,

    SPELL_FORCE_SPECIALIZATION_BLOOD        = 211222,
    SPELL_FORCE_SPECIALIZATION_FROST        = 211223
};

// 253 - A Pact of Necessity: Frost (Blood and Unholy have no artifact quest yet)
class playerchoice_the_pact_of_necessity : public PlayerChoiceScript
{
public:
    playerchoice_the_pact_of_necessity() : PlayerChoiceScript("playerchoice_the_pact_of_necessity") { }

    void OnResponse(WorldObject* /*object*/, Player* player, PlayerChoice const* /*choice*/, PlayerChoiceResponse const* response) override
    {
        // The first pick credits the quest; a late click on the window must not pick a second weapon
        if (player->GetQuestStatus(QUEST_A_PACT_OF_NECESSITY) != QUEST_STATUS_INCOMPLETE)
            return;

        if (response->ResponseId == PLAYERCHOICE_RESPONSE_FROST)
            player->CastSpell(player, SPELL_FROST_CHOSEN, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }
};

// 211222 - Force Specialization (Blood)
// 211223 - Force Specialization (Frost)
// 211225 - Force Specialization (Unholy)
class spell_dk_force_specialization : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitUnit()->ToPlayer();
        if (!player)
            return;

        ChrSpecialization spec = ChrSpecialization::DeathKnightUnholy;
        if (GetSpellInfo()->Id == SPELL_FORCE_SPECIALIZATION_BLOOD)
            spec = ChrSpecialization::DeathKnightBlood;
        else if (GetSpellInfo()->Id == SPELL_FORCE_SPECIALIZATION_FROST)
            spec = ChrSpecialization::DeathKnightFrost;

        player->ActivateTalentGroup(sChrSpecializationStore.AssertEntry(AsUnderlyingType(spec)));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_force_specialization::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 201261 - 7.0 Artifacts - Death Knight
// A Pact of Necessity's reward: sends the player on the quest of the weapon they chose
class spell_dk_artifacts_choice_reward : public SpellScript
{
    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        if (player->GetQuestStatus(QUEST_FROST_CHOSEN) == QUEST_STATUS_REWARDED)
            player->CastSpell(player, SPELL_FROST_DEATH_KNIGHT_QUEST_LAUNCH, true);
        else
            TC_LOG_ERROR("scripts", "spell_dk_artifacts_choice_reward: {} chose a weapon whose artifact quest is not built", player->GetGUID().ToString());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_artifacts_choice_reward::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum TheCallOfIcecrownData
{
    QUEST_THE_CALL_OF_ICECROWN              = 38990,
    QUEST_OBJECTIVE_USE_DEATH_GATE          = 278748,
    QUEST_OBJECTIVE_INTRO_SCENE             = 279705,

    NPC_DEATH_GATE                          = 95854,
    NPC_WAYGATE_CREDIT                      = 93970,
    NPC_INTRO_SCENE_CREDIT                  = 96227,

    SPELL_THE_BLADES_OF_THE_FALLEN_PRINCE   = 190789,
    SPELL_THE_BLADES_SCENE_END              = 190794,
    SPELL_LAUNCH_THE_BLADES_SCENARIO        = 188406,
    SPELL_ICECROWN_MOGRAINE_PING            = 188022,
    // No data names the blackout 190790 Cancel Blackout ends: a plain 7 s fade to black (screen effect 828) keeps the
    // player from seeing the landing between the Death Gate and the scene
    SPELL_FADE_TO_BLACK                     = 112885,

    MAP_ICECROWN_CITADEL_SCENARIO           = 1480,
    WORLD_SAFE_LOC_FROST_SCENARIO_START     = 5081,

    ZONE_ACHERUS                            = 7679
};

static constexpr float DeathGateDistance = 4.0f;
static constexpr float DeathGateAngle = float(M_PI / 2); // to the player's left, clear of the NPC they faced
static constexpr float DeathGateSearchRange = 50.0f;
static constexpr Seconds DeathGateDuration = 10min;
static constexpr Milliseconds AcherusArrivalDelay = 1s;
static constexpr Milliseconds IntroSceneFallback = 60s;
static constexpr Milliseconds FadeToBlackLimit = 10s;

// The quest's own summon (187409 -> 190089 -> 190088) aims at a target type the core does not implement, so the gate
// is placed here: beside the player, seen by them alone, and only one at a time
static void SummonDeathGate(Player* player)
{
    if (player->GetZoneId() == ZONE_ACHERUS || player->GetQuestObjectiveData(QUEST_THE_CALL_OF_ICECROWN, QUEST_OBJECTIVE_USE_DEATH_GATE) > 0)
        return;

    std::list<Creature*> gates;
    player->GetCreatureListWithEntryInGrid(gates, NPC_DEATH_GATE, DeathGateSearchRange);
    for (Creature* gate : gates)
        if (gate->GetPrivateObjectOwner() == player->GetGUID())
            gate->DespawnOrUnsummon();

    Position pos = player->GetNearPosition(DeathGateDistance, DeathGateAngle);
    pos.SetOrientation(pos.GetAbsoluteAngle(player));
    player->SummonCreature(NPC_DEATH_GATE, pos, TEMPSUMMON_TIMED_DESPAWN, DeathGateDuration, 0, 0, player->GetGUID());
}

// 38990 - The Call of Icecrown
class quest_the_call_of_icecrown : public QuestScript
{
public:
    quest_the_call_of_icecrown() : QuestScript("quest_the_call_of_icecrown") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus /*oldStatus*/, QuestStatus newStatus) override
    {
        if (newStatus == QUEST_STATUS_INCOMPLETE)
            SummonDeathGate(player);
    }
};

static bool HasSeenIntroScene(Player* player)
{
    return player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE
        || player->GetQuestObjectiveData(QUEST_THE_CALL_OF_ICECROWN, QUEST_OBJECTIVE_INTRO_SCENE) > 0;
}

// The entrance scene (190789, package 1467: bone drakes over the Spire, Acherus docked on Icecrown) is framed on the
// player's own position, so it plays on map 1480 at the landing, the first time the player arrives there. Its end
// (190794) credits the intro objective and sends event 45596, which starts Mograine. Each step is checked again when
// it is due: the zone updates of one arrival, or a login, cannot play it twice. A scene that never reports back (a
// client that drops it) is ended a minute after arrival
static void HandleScenarioArrival(Player* player)
{
    if (HasSeenIntroScene(player))
        return;

    // On the player's own event queue: a logout drops both
    player->m_Events.AddEventAtOffset([player]
    {
        if (HasSeenIntroScene(player) || player->GetMapId() != MAP_ICECROWN_CITADEL_SCENARIO || player->IsBeingTeleported()
            || player->HasAura(SPELL_THE_BLADES_OF_THE_FALLEN_PRINCE))
            return;

        player->RemoveAurasDueToSpell(SPELL_FADE_TO_BLACK);
        player->CastSpell(player, SPELL_THE_BLADES_OF_THE_FALLEN_PRINCE, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }, AcherusArrivalDelay);

    player->m_Events.AddEventAtOffset([player]
    {
        if (HasSeenIntroScene(player))
            return;

        player->RemoveAurasDueToSpell(SPELL_FADE_TO_BLACK);
        if (player->GetMapId() == MAP_ICECROWN_CITADEL_SCENARIO && !player->IsBeingTeleported())
            player->CastSpell(player, SPELL_THE_BLADES_SCENE_END, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        else
            player->KilledMonsterCredit(NPC_INTRO_SCENE_CREDIT);
    }, IntroSceneFallback);
}

// "While you are on this quest, you won't be able to conduct normal business in Acherus": a death knight on The Call of
// Icecrown who stands in Acherus is sent into the Icecrown Citadel scenario
static void HandleAcherusArrival(Player* player)
{
    if (player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE)
        return;

    player->m_Events.AddEventAtOffset([player]
    {
        if (player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE || player->GetZoneId() != ZONE_ACHERUS
            || player->IsBeingTeleported())
            return;

        player->CastSpell(player, SPELL_LAUNCH_THE_BLADES_SCENARIO, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }, AcherusArrivalDelay);
}

// The gate is a summon and does not survive a logout: a player who left before using it gets it back. Arriving or
// logging in in Acherus while the quest is under way leads into the scenario, and the first arrival there plays the
// entrance scene
class player_dk_the_call_of_icecrown : public PlayerScript
{
public:
    player_dk_the_call_of_icecrown() : PlayerScript("player_dk_the_call_of_icecrown") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        // A fade saved at logout would leave the player blind
        player->RemoveAurasDueToSpell(SPELL_FADE_TO_BLACK);
        if (player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE)
            return;

        SummonDeathGate(player);
        if (player->GetZoneId() == ZONE_ACHERUS)
            HandleAcherusArrival(player);
        else if (player->GetMapId() == MAP_ICECROWN_CITADEL_SCENARIO)
            HandleScenarioArrival(player);
    }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        if (player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE)
            return;

        if (newZone == ZONE_ACHERUS)
            HandleAcherusArrival(player);
        else if (player->GetMapId() == MAP_ICECROWN_CITADEL_SCENARIO)
            HandleScenarioArrival(player);
    }
};

// 187753 - Death Gate (to Acherus)
// The quest's first step: the summoned gate and the player's own Death Gate both cast it. With The Call of Icecrown
// under way it leads straight into the scenario instead of Acherus (one loading screen, then the entrance scene)
class spell_dk_the_call_of_icecrown_death_gate : public SpellScript
{
    void HandleTeleport(SpellEffIndex effIndex)
    {
        Player* player = GetHitPlayer();
        if (!player || player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE)
            return;

        PreventHitDefaultEffect(effIndex);
        player->KilledMonsterCredit(NPC_WAYGATE_CREDIT);
        if (!HasSeenIntroScene(player))
        {
            // Black from the gate to the scene; lifted when the scene starts, and in any case after a few seconds
            player->CastSpell(player, SPELL_FADE_TO_BLACK, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            player->m_Events.AddEventAtOffset([player]
            {
                player->RemoveAurasDueToSpell(SPELL_FADE_TO_BLACK);
            }, FadeToBlackLimit);
        }

        player->CastSpell(player, SPELL_LAUNCH_THE_BLADES_SCENARIO, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_the_call_of_icecrown_death_gate::HandleTeleport, EFFECT_0, SPELL_EFFECT_TELEPORT_UNITS);
    }
};

// Scene 1076 (package 1467, "Icecrown Entrance Scene"), played by 190789 on the first arrival on map 1480: watched or
// skipped, its end casts 190794, which lifts the scene's aura and phase and credits The Call of Icecrown's intro objective
class scene_dk_the_blades_of_the_fallen_prince : public SceneScript
{
public:
    scene_dk_the_blades_of_the_fallen_prince() : SceneScript("scene_dk_the_blades_of_the_fallen_prince") { }

    void OnSceneComplete(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        EndScene(player);
    }

    void OnSceneCancel(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        EndScene(player);
    }

private:
    static void EndScene(Player* player)
    {
        player->CastSpell(player, SPELL_THE_BLADES_SCENE_END, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }
};

// 45596 - sent by 190794, the end of The Call of Icecrown's entrance scene. On the landing it starts Mograine's walk
// (188022, aimed at him by conditions); anywhere else it sends the player into the scenario
class event_dk_the_blades_of_the_fallen_prince_scene_end : public EventScript
{
public:
    event_dk_the_blades_of_the_fallen_prince_scene_end() : EventScript("event_dk_the_blades_of_the_fallen_prince_scene_end") { }

    void OnTrigger(WorldObject* /*object*/, WorldObject* invoker, uint32 /*eventId*/) override
    {
        Player* player = Object::ToPlayer(invoker);
        if (!player || player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE)
            return;

        if (player->GetMapId() == MAP_ICECROWN_CITADEL_SCENARIO)
            player->CastSpell(nullptr, SPELL_ICECROWN_MOGRAINE_PING, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        else
            player->CastSpell(player, SPELL_LAUNCH_THE_BLADES_SCENARIO, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }
};

// 188406 - Launch The Blades of the Fallen Prince Scenario
// A dummy: the player goes to the scenario's start, the lower deck of Acherus beside Icecrown Citadel (map 1480)
class spell_dk_launch_the_blades_of_the_fallen_prince_scenario : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitPlayer();
        if (!player || player->GetMapId() == MAP_ICECROWN_CITADEL_SCENARIO)
            return;

        // WorldSafeLocs keep their facing in degrees
        if (WorldSafeLocsEntry const* start = sWorldSafeLocsStore.LookupEntry(WORLD_SAFE_LOC_FROST_SCENARIO_START))
            player->TeleportTo(MAP_ICECROWN_CITADEL_SCENARIO, start->Loc.X, start->Loc.Y, start->Loc.Z, start->Facing * float(M_PI) / 180.0f);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_launch_the_blades_of_the_fallen_prince_scenario::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

enum AcherusHallOpeningData
{
    QUEST_KEEPING_YOUR_EDGE                 = 39757,
    QUEST_ADVANCED_RUNECARVING              = 39761,

    NPC_TELEPORTER_BUNNY                    = 104848 // Keeping Your Edge: "Take the teleporter to the second level of Acherus"
};

// The two ends of the hall's teleporter: the deck end at 39757's objective POI, the upper end above it (the user's ping)
static Position const AcherusTeleporterDeck = { -1464.00f, 1124.00f, 270.50f, 0.0f };
static Position const AcherusTeleporterUpper = { -1460.06f, 1130.99f, 314.70f, 0.0f };
static constexpr float AcherusTeleporterSplitZ = 285.0f; // between the deck end and the upper end

// 1220002 - Acherus Teleporter (fork data: the data has no Legion teleporter object for Acherus)
// One object at each end; a click sends the death knight to the other end. Going up credits Keeping Your Edge
struct go_acherus_hall_teleporter : GameObjectAI
{
    go_acherus_hall_teleporter(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        bool const goingUp = me->GetPositionZ() < AcherusTeleporterSplitZ;
        Position const& destination = goingUp ? AcherusTeleporterUpper : AcherusTeleporterDeck;
        player->NearTeleportTo(destination.GetPositionX(), destination.GetPositionY(), destination.GetPositionZ(),
            player->GetOrientation());

        if (goingUp && player->GetQuestStatus(QUEST_KEEPING_YOUR_EDGE) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_TELEPORTER_BUNNY);

        return true;
    }
};

// 245433 - Soul Forge
// Advanced Runecarving has the first trait bought, and the blades have no artifact power when they are won: as at the
// priest's and the demon hunter's forges, the forge gives the first trait's cost to a weapon nothing has been bought for
struct go_acherus_soul_forge : GameObjectAI
{
    go_acherus_soul_forge(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_ADVANCED_RUNECARVING) != QUEST_STATUS_INCOMPLETE)
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

// The Command Map (102589) over the hall's mission table. The template's scale (8, set for the priest's hall) lets
// its model's sheen show through the green mist in Acherus; at 2 it stays inside the table and can still be clicked.
// The adventure map opens from the npcflag, not from this AI
static constexpr float AcherusCommandMapScale = 2.0f;

struct npc_acherus_command_map : ScriptedAI
{
    npc_acherus_command_map(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetObjectScale(AcherusCommandMapScale);
    }
};

// The Firstborn Rises (42484), in Durotar. The data stages the raise: Raise Horseman - Aura (213932, from spell_area)
// puts 213614 on the override bar; 213614 force-casts the 10 s Raise Horseman (213633), which summons the invisible
// anchor 107597 at the raise spot and gives the credit (213679). Nothing in the data seats Nazgrim on the anchor or plays
// the exchange, so Thassarian does it here. The speaker of each line is the one its voice file is named for
// (vo_703_thassarian_04/05, vo_703_nazgrim_02/03, vo_703_thassarian_06/07); the order follows the lines' sense
enum TheFirstbornRises
{
    QUEST_THE_FIRSTBORN_RISES           = 42484,

    NPC_THASSARIAN_DUROTAR              = 107554,
    NPC_NAZGRIM_RAISED                  = 107602,
    NPC_RAISE_ANCHOR_DEATH_GATE         = 95854,    // the gate 190088 summons; picked from the preview lineup

    SPELL_RAISE_HORSEMAN_BUTTON         = 213614,
    SPELL_NAZGRIM_GHOST_VISUAL          = 224864,   // Shadowy Ghost Visual State, Blue; picked from the preview lineup

    TALK_THASSARIAN_IT_IS_TIME          = 0,        // 112575, on accepting the quest
    TALK_THASSARIAN_DEATHLORD_CALLS     = 1,        // 112573, as the raise begins
    TALK_THASSARIAN_LONG_ENOUGH         = 2,        // 112588
    TALK_THASSARIAN_INDEED_YOU_SHALL    = 3,        // 112589
    TALK_NAZGRIM_I_LIVE_AGAIN           = 0,        // 112584
    TALK_NAZGRIM_SAY_NO_MORE            = 1,        // 112587

    ACTION_RAISE_BEGUN                  = 1,
    DATA_RAISE_ANCHOR                   = 1,
    POINT_NAZGRIM_DEATH_GATE            = 1
};

static constexpr Seconds RaiseDeathGateDuration = 90s;

// 107554 - Thassarian (Durotar)
struct npc_thassarian_the_firstborn_rises : public ScriptedAI
{
    npc_thassarian_the_firstborn_rises(Creature* creature) : ScriptedAI(creature), _sceneRunning(false) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_THE_FIRSTBORN_RISES)
            Talk(TALK_THASSARIAN_IT_IS_TIME, player);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_RAISE_BEGUN && !_sceneRunning)
            Talk(TALK_THASSARIAN_DEATHLORD_CALLS);
    }

    // The anchor has appeared: Nazgrim appears on it as a kneeling ghost, stands, speaks with Thassarian and walks out
    // through a Death Gate. The anchor, summoned with an infinite duration, goes with him. (Retail raises the kneeling
    // model straight up out of the ground; a spline makes the client drop the kneel, so he appears on the floor)
    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id != DATA_RAISE_ANCHOR)
            return;

        Creature* anchor = ObjectAccessor::GetCreature(*me, guid);
        if (!anchor)
            return;

        if (_sceneRunning)
        {
            anchor->DespawnOrUnsummon();
            return;
        }

        // Only the player who raised him sees the ghost and the gate
        ObjectGuid raiserGuid = anchor->IsSummon() ? anchor->ToTempSummon()->GetSummonerGUID() : ObjectGuid::Empty;
        Position pos = anchor->GetPosition();
        pos.SetOrientation(anchor->GetAbsoluteAngle(me));
        Creature* nazgrim = me->SummonCreature(NPC_NAZGRIM_RAISED, pos, TEMPSUMMON_MANUAL_DESPAWN, 0s, 0, 0, raiserGuid);
        if (!nazgrim)
        {
            anchor->DespawnOrUnsummon();
            return;
        }

        _sceneRunning = true;
        nazgrim->SetStandState(UNIT_STAND_STATE_KNEEL);
        nazgrim->CastSpell(nazgrim, SPELL_NAZGRIM_GHOST_VISUAL, true);
        me->SetFacingToObject(nazgrim);

        ObjectGuid nazgrimGuid = nazgrim->GetGUID();
        _scheduler.Schedule(5s, [this, nazgrimGuid](TaskContext /*context*/)
        {
            if (Creature* nazgrim = ObjectAccessor::GetCreature(*me, nazgrimGuid))
            {
                nazgrim->SetStandState(UNIT_STAND_STATE_STAND);
                nazgrim->AI()->Talk(TALK_NAZGRIM_I_LIVE_AGAIN);
            }
        });
        _scheduler.Schedule(11s, [this](TaskContext /*context*/) { Talk(TALK_THASSARIAN_LONG_ENOUGH); });
        _scheduler.Schedule(18s, [this, nazgrimGuid](TaskContext /*context*/)
        {
            if (Creature* nazgrim = ObjectAccessor::GetCreature(*me, nazgrimGuid))
                nazgrim->AI()->Talk(TALK_NAZGRIM_SAY_NO_MORE);
        });
        _scheduler.Schedule(23s, [this](TaskContext /*context*/) { Talk(TALK_THASSARIAN_INDEED_YOU_SHALL); });
        // The gate opens a few yards ahead of Nazgrim, turned to face him, and he walks into it
        _scheduler.Schedule(27s, [this, nazgrimGuid, raiserGuid](TaskContext /*context*/)
        {
            Creature* nazgrim = ObjectAccessor::GetCreature(*me, nazgrimGuid);
            if (!nazgrim)
                return;

            Position gatePos = nazgrim->GetNearPosition(4.0f, 0.0f);
            gatePos.SetOrientation(Position::NormalizeOrientation(nazgrim->GetOrientation() + float(M_PI)));
            // Summoned by Thassarian so it outlasts Nazgrim, and left clickable: once he is through, the raiser can take it
            // back to Acherus (its spellclick, 187753 Death Gate)
            me->SummonCreature(NPC_RAISE_ANCHOR_DEATH_GATE, gatePos, TEMPSUMMON_TIMED_DESPAWN, RaiseDeathGateDuration, 0, 0, raiserGuid);
            _gatePos = gatePos;
        });
        _scheduler.Schedule(28500ms, [this, nazgrimGuid](TaskContext /*context*/)
        {
            if (Creature* nazgrim = ObjectAccessor::GetCreature(*me, nazgrimGuid))
            {
                nazgrim->SetWalk(true);
                nazgrim->GetMotionMaster()->MovePoint(POINT_NAZGRIM_DEATH_GATE, _gatePos, false);
            }
        });
        // About 1.6 s of walking at 2.5 yd/s: he disappears as he reaches the gate
        _scheduler.Schedule(30200ms, [this, nazgrimGuid, guid](TaskContext /*context*/)
        {
            if (Creature* nazgrim = ObjectAccessor::GetCreature(*me, nazgrimGuid))
                nazgrim->DespawnOrUnsummon();
            if (Creature* anchor = ObjectAccessor::GetCreature(*me, guid))
                anchor->DespawnOrUnsummon();
            me->SetFacingTo(me->GetHomePosition().GetOrientation());
            _sceneRunning = false;
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    bool _sceneRunning;
    Position _gatePos;
};

// 107597 - Nazgrim Vehicle: the raise's invisible anchor, summoned by 213633 at the raise spot
struct npc_the_firstborn_rises_anchor : public ScriptedAI
{
    npc_the_firstborn_rises_anchor(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* /*summoner*/) override
    {
        if (Creature* thassarian = me->FindNearestCreature(NPC_THASSARIAN_DUROTAR, 60.0f))
            thassarian->AI()->SetGUID(me->GetGUID(), DATA_RAISE_ANCHOR);
        else
            me->DespawnOrUnsummon();
    }
};

// 213614 - Raise Horseman (the override bar button)
class spell_dk_the_firstborn_rises_raise_horseman : public SpellScript
{
    void HandleAfterCast()
    {
        if (Creature* thassarian = GetCaster()->FindNearestCreature(NPC_THASSARIAN_DUROTAR, 60.0f))
            thassarian->AI()->DoAction(ACTION_RAISE_BEGUN);
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_dk_the_firstborn_rises_raise_horseman::HandleAfterCast);
    }
};

// Spread the Word (43265): its objective is a kill credit for the tutorial mission (1210, Nazgrim sent to Dalaran), which
// nothing in the data gives; it is given when the mission succeeds, as the objective reads ("Complete the mission")
enum DeathKnightCampaignMissions
{
    MISSION_SPREAD_THE_WORD         = 1210,
    NPC_SENT_ON_TUTORIAL_MISSION    = 105479,
    QUEST_SPREAD_THE_WORD           = 43265,
    QUEST_TROOPS_IN_THE_FIELD       = 43267  // mission 1211, credited by the core (SucceedGarrisonMission)
};

class player_dk_campaign_tutorial_missions : public PlayerScript
{
public:
    player_dk_campaign_tutorial_missions() : PlayerScript("player_dk_campaign_tutorial_missions", { PLAYERHOOK_ON_CLASS_HALL_MISSION_SUCCEED, PLAYERHOOK_ON_QUEST_STATUS_CHANGE }) { }

    // Each tutorial mission's condition is met on accepting its quest; offer it now rather than at the next once-a-minute
    // look
    void OnQuestStatusChange(Player* player, uint32 questId) override
    {
        if (questId != QUEST_SPREAD_THE_WORD && questId != QUEST_TROOPS_IN_THE_FIELD)
            return;

        if (player->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
            return;

        if (ClassHall* classHall = player->GetClassHall())
            classHall->GetMissions().OfferMissions(true);
    }

    void OnClassHallMissionSucceed(Player* player, uint32 garrMissionId) override
    {
        if (garrMissionId == MISSION_SPREAD_THE_WORD)
            player->KilledMonsterCredit(NPC_SENT_ON_TUTORIAL_MISSION);
    }
};

void AddSC_orderhall_death_knight()
{
    // Quests
    new quest_the_call_of_icecrown();
    new player_dk_the_call_of_icecrown();

    // Playerchoice
    new playerchoice_the_pact_of_necessity();

    // Scenes
    new scene_dk_the_blades_of_the_fallen_prince();

    // Events
    new event_dk_the_blades_of_the_fallen_prince_scene_end();

    // Spellscripts
    RegisterSpellScript(spell_dk_force_specialization);
    RegisterSpellScript(spell_dk_artifacts_choice_reward);
    RegisterSpellScript(spell_dk_the_call_of_icecrown_death_gate);
    RegisterSpellScript(spell_dk_launch_the_blades_of_the_fallen_prince_scenario);
    RegisterGameObjectAI(go_acherus_hall_teleporter);
    RegisterGameObjectAI(go_acherus_soul_forge);
    RegisterCreatureAI(npc_acherus_command_map);
    new player_dk_campaign_tutorial_missions();
    RegisterCreatureAI(npc_thassarian_the_firstborn_rises);
    RegisterCreatureAI(npc_the_firstborn_rises_anchor);
    RegisterSpellScript(spell_dk_the_firstborn_rises_raise_horseman);
}
