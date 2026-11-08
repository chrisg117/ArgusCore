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
#include "DB2Stores.h"
#include "GameEventSender.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Map.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerChoice.h"
#include "QuestDef.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"

enum TheSixthData
{
    QUEST_THE_SIXTH                     = 40716,

    NPC_RISSTYNS_PORTAL                 = 103505,
    NPC_RITSSYN_THE_SIXTH               = 103506,

    SAY_RITSSYN_GREETING                = 0, // 106111 "A moment of your time, warlock..."
    SAY_RITSSYN_JOIN_US                 = 1, // 104179 "Join us at the Circle of Wills..."

    POINT_RITSSYN_PORTAL                = 1
};

// 103505 - Demonic Portal: summoned in Dalaran by 204858 (from the 7.0 Order Campaign aura 224240) for a warlock who
// has not taken The Sixth. Ritssyn steps out of it (204860, cast by the warlock so that he is the warlock's summon).
// The portal stays for its 5 minutes: a click casts 204857 Risstyn's Portal (npc_spellclick_spells), which takes the
// warlock to the Circle of Wills (Wowhead's acquisition video, 0:00-0:32).
struct npc_risstyns_portal : public ScriptedAI
{
    npc_risstyns_portal(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player || player->GetClass() != CLASS_WARLOCK || player->GetQuestStatus(QUEST_THE_SIXTH) == QUEST_STATUS_REWARDED)
        {
            me->DespawnOrUnsummon();
            return;
        }

        // Ritssyn only comes out for a warlock who has not taken the quest yet; one who has (and comes back) just gets
        // the portal
        if (player->GetQuestStatus(QUEST_THE_SIXTH) != QUEST_STATUS_NONE)
            return;

        // 204860 Summon Risstyn is the data's way (DEST_NEARBY_ENTRY the portal), but it is a self-range spell: this
        // core searches nearby entries within the spell's max range (0), so it never finds the portal 10 yd away and
        // summons nothing (2026-11-08, Claudeloc). Ritssyn is summoned here instead, as the warlock's summon, a step
        // out of the portal towards the warlock.
        ObjectGuid playerGuid = player->GetGUID();
        _scheduler.Schedule(1500ms, [this, playerGuid](TaskContext /*context*/)
        {
            Player* player = ObjectAccessor::GetPlayer(*me, playerGuid);
            if (!player)
                return;
            Position pos = me->GetPosition();
            float angle = me->GetAbsoluteAngle(player);
            me->MovePosition(pos, 1.5f, angle - me->GetOrientation());
            pos.SetOrientation(angle);
            player->SummonCreature(NPC_RITSSYN_THE_SIXTH, pos, TEMPSUMMON_TIMED_DESPAWN, 5min);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 103506 - Ritssyn Flamescowl: the copy that steps out of the portal, greets the warlock and offers The Sixth (fork
// data: the quest has no starter in the data). Once it is taken he points to his portal and steps back through it.
struct npc_ritssyn_flamescowl_the_sixth : public ScriptedAI
{
    npc_ritssyn_flamescowl_the_sixth(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        _summonerGuid = summoner->GetGUID();
        if (Player* player = summoner->ToPlayer())
            me->SetFacingToObject(player);
        _scheduler.Schedule(1s, [this](TaskContext /*context*/)
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, _summonerGuid))
                Talk(SAY_RITSSYN_GREETING, player);
        });
    }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() != QUEST_THE_SIXTH)
            return;

        Talk(SAY_RITSSYN_JOIN_US, player);
        _scheduler.Schedule(5s, [this](TaskContext /*context*/)
        {
            if (Creature* portal = FindOwnPortal())
                me->GetMotionMaster()->MovePoint(POINT_RITSSYN_PORTAL, portal->GetPosition());
            else
                me->DespawnOrUnsummon();
        });
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_RITSSYN_PORTAL)
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    // Only this warlock's own portal: another warlock's may stand nearby
    Creature* FindOwnPortal() const
    {
        std::list<Creature*> portals;
        me->GetCreatureListWithEntryInGrid(portals, NPC_RISSTYNS_PORTAL, 30.0f);
        for (Creature* portal : portals)
            if (portal->IsSummon() && portal->ToTempSummon()->GetSummonerGUID() == _summonerGuid)
                return portal;
        return nullptr;
    }

    TaskScheduler _scheduler;
    ObjectGuid _summonerGuid;
};

enum TheNewBloodData
{
    QUEST_THE_NEW_BLOOD                 = 40729,
    QUEST_OBJECTIVE_RITUAL_PERFORMED    = 281139,
    QUEST_THE_TOME_OF_BLIGHTED_IMPLEMENTS = 40684,
    QUEST_OBJECTIVE_ARTIFACT_CHOSEN     = 281063,

    NPC_ZINNIN_CIRCLE                   = 101454,
    NPC_SHINFEL_CIRCLE                  = 101455,
    NPC_JUBEKA_CIRCLE                   = 101457,
    NPC_ZELIFRAX_WOBBLEPOX              = 98797,
    NPC_JAGGANOTH_CIRCLE                = 98831,

    GO_RITUAL_GATEWAY                   = 1220004, // fork template; its look is the S2 lineup's pick

    SPELL_START_SCENARIO                = 199359, // teleport to Jagganoth's prison (map 1522); credits Ritual Performed
    SPELL_TOME_PLAYER_CHOICE            = 199411, // launches player choice 245
    SPELL_DESTRUCTION                   = 199409,

    PLAYERCHOICE_RESPONSE_DESTRUCTION   = 2451,

    SAY_RITSSYN_THE_SIXTH_IS_HERE       = 0, // 103743
    SAY_ZELIFRAX_STUDIED                = 0, // 103744 (Ritssyn's question to Zelifrax; Zelifrax opens the ritual)
    SAY_ZELIFRAX_CHANNEL                = 1, // 103747
    SAY_ZELIFRAX_RITUAL_BEGINS          = 2, // 103750
    SAY_JAGGANOTH_HURRY                 = 2, // 101038
    SAY_JAGGANOTH_FOOLISH_MORTALS       = 0, // 103762

    GOSSIP_ACTION_BEGIN_RITUAL          = 1,
    GOSSIP_ACTION_REVIEW_WEAPONS        = 2
};

// The Circle of Wills: where the council stands for the ritual, and the gateway Jagganoth comes through
Position const RitualZinnin   = { -850.00f, 4440.00f, 697.75f, 1.75f };
Position const RitualJubeka   = { -852.00f, 4462.00f, 696.26f, 4.90f };
Position const RitualShinfel  = { -838.00f, 4454.00f, 696.71f, 3.30f };
Position const RitualZelifrax = { -866.00f, 4446.00f, 696.88f, 0.40f };
Position const RitualGateway  = { -862.00f, 4470.00f, 696.75f, 5.30f };
Position const RitualJagganoth = { -859.50f, 4466.00f, 696.75f, 5.30f };

// 101456 - Ritssyn Flamescowl, in the Circle of Wills. The New Blood (offered at The Sixth's turn-in) begins the ritual:
// the council gathers, Zelifrax leads it, the gateway opens, and Jagganoth comes through and takes the warlock to his
// prison (199359). Staging from Wowhead's acquisition video, 0:42-3:29; every line is the client's own broadcast text.
// A warlock who left the scenario without finishing it asks Ritssyn to begin again
struct npc_ritssyn_flamescowl_circle_of_wills : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_THE_NEW_BLOOD)
            BeginRitual(player);
    }

    bool OnGossipHello(Player* player) override
    {
        player->PrepareQuestMenu(me->GetGUID());
        if (player->GetQuestStatus(QUEST_THE_NEW_BLOOD) == QUEST_STATUS_INCOMPLETE && !_ritualRunning)
            AddGossipItemFor(player, GossipOptionNpc::None, "I am ready to begin the ritual.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_BEGIN_RITUAL);
        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
        CloseGossipMenuFor(player);
        if (action == GOSSIP_ACTION_BEGIN_RITUAL && player->GetQuestStatus(QUEST_THE_NEW_BLOOD) == QUEST_STATUS_INCOMPLETE)
            BeginRitual(player);
        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void BeginRitual(Player* player)
    {
        if (_ritualRunning)
            return;

        // Back from the prison unfinished: the ritual has been seen, so the warlock goes straight back
        if (player->GetQuestObjectiveData(QUEST_THE_NEW_BLOOD, QUEST_OBJECTIVE_RITUAL_PERFORMED) > 0)
        {
            player->CastSpell(player, SPELL_START_SCENARIO, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            return;
        }

        _ritualRunning = true;
        ObjectGuid const playerGuid = player->GetGUID();
        _summons.clear();
        Summon(NPC_ZINNIN_CIRCLE, RitualZinnin);
        Summon(NPC_JUBEKA_CIRCLE, RitualJubeka);
        Summon(NPC_SHINFEL_CIRCLE, RitualShinfel);
        ObjectGuid const zelifrax = Summon(NPC_ZELIFRAX_WOBBLEPOX, RitualZelifrax);

        _scheduler.Schedule(3s, [this, playerGuid](TaskContext /*context*/) { Talk(SAY_RITSSYN_THE_SIXTH_IS_HERE, Get(playerGuid)); })
            .Schedule(8s, [this, zelifrax, playerGuid](TaskContext /*context*/) { TalkFrom(zelifrax, SAY_ZELIFRAX_STUDIED, playerGuid); })
            .Schedule(14s, [this, zelifrax, playerGuid](TaskContext /*context*/)
            {
                TalkFrom(zelifrax, SAY_ZELIFRAX_CHANNEL, playerGuid);
                if (GameObject* gateway = me->SummonGameObject(GO_RITUAL_GATEWAY, RitualGateway, QuaternionData::fromEulerAnglesZYX(RitualGateway.GetOrientation(), 0.0f, 0.0f), 90s))
                    _gateway = gateway->GetGUID();
            })
            .Schedule(20s, [this, zelifrax, playerGuid](TaskContext /*context*/) { TalkFrom(zelifrax, SAY_ZELIFRAX_RITUAL_BEGINS, playerGuid); })
            .Schedule(26s, [this, playerGuid](TaskContext /*context*/)
            {
                if (Creature* jagganoth = me->SummonCreature(NPC_JAGGANOTH_CIRCLE, RitualJagganoth, TEMPSUMMON_TIMED_DESPAWN, 40s))
                {
                    _summons.push_back(jagganoth->GetGUID());
                    jagganoth->AI()->Talk(SAY_JAGGANOTH_HURRY, Get(playerGuid));
                }
            })
            .Schedule(32s, [this, playerGuid](TaskContext /*context*/)
            {
                if (Creature* jagganoth = me->FindNearestCreature(NPC_JAGGANOTH_CIRCLE, 60.0f))
                    jagganoth->AI()->Talk(SAY_JAGGANOTH_FOOLISH_MORTALS, Get(playerGuid));
            })
            .Schedule(39s, [this, playerGuid](TaskContext /*context*/)
            {
                if (Player* player = Get(playerGuid))
                    if (player->GetQuestStatus(QUEST_THE_NEW_BLOOD) == QUEST_STATUS_INCOMPLETE)
                        player->CastSpell(player, SPELL_START_SCENARIO, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            })
            .Schedule(45s, [this](TaskContext /*context*/)
            {
                for (ObjectGuid const& guid : _summons)
                    if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                        summon->DespawnOrUnsummon();
                _summons.clear();
                if (GameObject* gateway = ObjectAccessor::GetGameObject(*me, _gateway))
                    gateway->DespawnOrUnsummon();
                _ritualRunning = false;
            });
    }

    ObjectGuid Summon(uint32 entry, Position const& pos)
    {
        if (Creature* summon = me->SummonCreature(entry, pos, TEMPSUMMON_TIMED_DESPAWN, 60s))
        {
            _summons.push_back(summon->GetGUID());
            return summon->GetGUID();
        }
        return ObjectGuid::Empty;
    }

    void TalkFrom(ObjectGuid speaker, uint8 group, ObjectGuid playerGuid)
    {
        if (Creature* creature = ObjectAccessor::GetCreature(*me, speaker))
            creature->AI()->Talk(group, Get(playerGuid));
    }

    Player* Get(ObjectGuid guid) const { return ObjectAccessor::GetPlayer(*me, guid); }

    TaskScheduler _scheduler;
    std::vector<ObjectGuid> _summons;
    ObjectGuid _gateway;
    bool _ritualRunning = false;
};

// 101097 - Calydus, back in the Circle of Wills after the escape: The Tome of Blighted Implements. Taking it opens the
// weapon choice (199411 launches player choice 245, as in the data); his gossip opens it again until a weapon is chosen
struct npc_calydus_circle_of_wills : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_THE_TOME_OF_BLIGHTED_IMPLEMENTS)
            player->CastSpell(player, SPELL_TOME_PLAYER_CHOICE, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    bool OnGossipHello(Player* player) override
    {
        player->PrepareQuestMenu(me->GetGUID());
        if (player->GetQuestStatus(QUEST_THE_TOME_OF_BLIGHTED_IMPLEMENTS) == QUEST_STATUS_INCOMPLETE)
            AddGossipItemFor(player, GossipOptionNpc::None, "Let me see the weapons again.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_REVIEW_WEAPONS);
        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
        CloseGossipMenuFor(player);
        if (action == GOSSIP_ACTION_REVIEW_WEAPONS && player->GetQuestStatus(QUEST_THE_TOME_OF_BLIGHTED_IMPLEMENTS) == QUEST_STATUS_INCOMPLETE)
            player->CastSpell(player, SPELL_TOME_PLAYER_CHOICE, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        return true;
    }
};

// 245 - The Tome of Blighted Implements: Destruction only (Affliction and Demonology have no artifact quest yet)
class playerchoice_the_tome_of_blighted_implements : public PlayerChoiceScript
{
public:
    playerchoice_the_tome_of_blighted_implements() : PlayerChoiceScript("playerchoice_the_tome_of_blighted_implements") { }

    void OnResponse(WorldObject* /*object*/, Player* player, PlayerChoice const* /*choice*/, PlayerChoiceResponse const* response) override
    {
        if (player->GetQuestStatus(QUEST_THE_TOME_OF_BLIGHTED_IMPLEMENTS) != QUEST_STATUS_INCOMPLETE)
            return;

        if (response->ResponseId == PLAYERCHOICE_RESPONSE_DESTRUCTION)
            player->CastSpell(player, SPELL_DESTRUCTION, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }
};

enum WarlockForceSpecialization
{
    SPELL_FORCE_SPECIALIZATION_AFFLICTION   = 211250,
    SPELL_FORCE_SPECIALIZATION_DEMONOLOGY   = 211251,
    SPELL_FORCE_SPECIALIZATION_DESTRUCTION  = 211252
};

// 211250 / 211251 / 211252 - Force Specialization (Affliction / Demonology / Destruction)
class spell_warlock_force_specialization : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitUnit()->ToPlayer();
        if (!player)
            return;

        ChrSpecialization spec = ChrSpecialization::WarlockDestruction;
        if (GetSpellInfo()->Id == SPELL_FORCE_SPECIALIZATION_AFFLICTION)
            spec = ChrSpecialization::WarlockAffliction;
        else if (GetSpellInfo()->Id == SPELL_FORCE_SPECIALIZATION_DEMONOLOGY)
            spec = ChrSpecialization::WarlockDemonology;

        player->ActivateTalentGroup(sChrSpecializationStore.AssertEntry(AsUnderlyingType(spec)));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warlock_force_specialization::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

enum FindingTheScepterData
{
    QUEST_FINDING_THE_SCEPTER           = 43100,
    QUEST_OBJECTIVE_BOOK_OF_MEDIVH      = 285060,
    QUEST_OBJECTIVE_SPOKE_WITH_CALYDUS  = 285061,

    AREA_CAER_DARROW                    = 2298,

    NPC_CREDIT_GO_TO_CAER_DARROW        = 109659,
    NPC_CREDIT_SHADOW_COUNCIL_INFO      = 109693,
    NPC_CREDIT_SPOKE_WITH_CALYDUS       = 109696,

    ITEM_BOOK_OF_MEDIVH                 = 139035,

    SPELL_GUISE_OF_JERGOSH              = 218545,

    SAY_JERGOSH_THE_BOOK_IS_OURS        = 0, // 115699
    SAY_CALYDUS_TRICK_GULDAN            = 0, // 115708

    GOSSIP_ACTION_THE_BOOK              = 3
};

// Finding the Scepter: "Go to Caer Darrow" (109659) is the island itself, area 2298; nothing in the data credits it
class player_warlock_finding_the_scepter : public PlayerScript
{
public:
    player_warlock_finding_the_scepter() : PlayerScript("player_warlock_finding_the_scepter") { }

    void OnUpdateZone(Player* player, uint32 /*newZone*/, uint32 newArea) override
    {
        if (newArea == AREA_CAER_DARROW && player->GetQuestStatus(QUEST_FINDING_THE_SCEPTER) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_CREDIT_GO_TO_CAER_DARROW);
    }
};

// 251696 Council Notice, 251697 Forgotten Letter, 251698 Logbook of Ur'dan: the Shadow Council's papers on Caer Darrow
// (fork templates: the data has the objectives but no objects). Reading each counts its own hidden objective and one of
// "Find information on the Shadow Council" (109693, the credit 218330 gives in the data)
struct go_shadow_council_information : public GameObjectAI
{
    go_shadow_council_information(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_FINDING_THE_SCEPTER) != QUEST_STATUS_INCOMPLETE)
            return true;

        Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_FINDING_THE_SCEPTER);
        for (QuestObjective const& objective : quest->Objectives)
        {
            if (objective.Type != QUEST_OBJECTIVE_GAMEOBJECT || objective.ObjectID != int32(me->GetEntry()))
                continue;
            if (player->GetQuestObjectiveData(objective))
                return true; // read already
        }

        player->KillCreditGO(me->GetEntry(), me->GetGUID());
        player->KilledMonsterCredit(NPC_CREDIT_SHADOW_COUNCIL_INFO);
        return true;
    }
};

// 109608 - Jergosh the Invoker: the Shadow Council's man on Caer Darrow, with the Book of Medivh. He gloats as the warlock
// comes near and is fought; his death gives the Book (the bot cannot loot; real loot is a fidelity item)
struct npc_jergosh_the_invoker : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
        _gloated = false;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!_gloated && who->IsPlayer() && me->IsWithinDistInMap(who, 30.0f)
            && who->ToPlayer()->GetQuestStatus(QUEST_FINDING_THE_SCEPTER) == QUEST_STATUS_INCOMPLETE)
        {
            _gloated = true;
            Talk(SAY_JERGOSH_THE_BOOK_IS_OURS, who);
        }
        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* killer) override
    {
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (player && player->GetQuestStatus(QUEST_FINDING_THE_SCEPTER) == QUEST_STATUS_INCOMPLETE
            && !player->HasItemCount(ITEM_BOOK_OF_MEDIVH))
            player->AddItem(ITEM_BOOK_OF_MEDIVH, 1);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }

private:
    bool _gloated = false;
};

// 109698 - Calydus, on Caer Darrow: with the Book in hand, the warlock tells him, and he casts Guise of Jergosh (the
// Shadow Council will take the warlock for Jergosh; Portal Overload removes it in Ritual Ruination)
struct npc_calydus_caer_darrow : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    bool OnGossipHello(Player* player) override
    {
        player->PrepareQuestMenu(me->GetGUID());
        if (player->GetQuestStatus(QUEST_FINDING_THE_SCEPTER) == QUEST_STATUS_INCOMPLETE
            && player->GetQuestObjectiveData(QUEST_FINDING_THE_SCEPTER, QUEST_OBJECTIVE_BOOK_OF_MEDIVH) > 0
            && player->GetQuestObjectiveData(QUEST_FINDING_THE_SCEPTER, QUEST_OBJECTIVE_SPOKE_WITH_CALYDUS) == 0)
            AddGossipItemFor(player, GossipOptionNpc::None, "I have the Book of Medivh.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_THE_BOOK);
        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
        CloseGossipMenuFor(player);
        if (action == GOSSIP_ACTION_THE_BOOK && player->GetQuestStatus(QUEST_FINDING_THE_SCEPTER) == QUEST_STATUS_INCOMPLETE)
        {
            // A self-only aura (TARGET_UNIT_CASTER): the warlock wears it, so the warlock casts it
            player->CastSpell(player, SPELL_GUISE_OF_JERGOSH, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            player->KilledMonsterCredit(NPC_CREDIT_SPOKE_WITH_CALYDUS);
        }
        return true;
    }

    void OnQuestReward(Player* player, Quest const* quest, LootItemType /*type*/, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() == QUEST_FINDING_THE_SCEPTER)
            Talk(SAY_CALYDUS_TRICK_GULDAN, player);
    }
};

// ---------------------------------------------------------------------------------------------------------------------
// An Eye for a Scepter (43153): scenario 1155 on map 1630 (the Tol Barad copy). Places from Wowhead's pins converted with
// WorldMapArea 1090 (TolBaradWarlockScenario); lines are the client's broadcast texts.
namespace TolBaradWarlockScenario
{
enum Data
{
    MAP_TOL_BARAD_WARLOCK_SCENARIO      = 1630,
    QUEST_AN_EYE_FOR_A_SCEPTER          = 43153,
    QUEST_OBJECTIVE_WON_SCENARIO        = 285112,

    STEP_WHERE_ARE_THEY                 = 2700,
    STEP_THE_OTHER_PLAYERS              = 2715,
    STEP_THE_THIRD_ONE                  = 2716,
    STEP_AN_UNFORTUNATE_ACCIDENT        = 2717,
    STEP_BACK_TO_SEARCHING              = 2718,
    STEP_A_SECRET_MEETING               = 2719,
    STEP_BACK_TO_THE_MANIFEST           = 2720,
    STEP_THE_FINAL_WING                 = 2724,
    STEP_BARADIN_HOLD                   = 2725,
    STEP_THE_EYE_OF_THE_BEAST           = 2726,

    EVENT_WHERE_ARE_THEY                = 52741,
    EVENT_THE_OTHER_PLAYERS             = 52859, // PlayerTriggerGameEvent (piece/criteria-player-trigger-game-event)
    EVENT_THE_THIRD_ONE                 = 52860,
    EVENT_AN_UNFORTUNATE_ACCIDENT       = 52861,
    EVENT_BACK_TO_SEARCHING             = 52869,
    EVENT_A_SECRET_MEETING              = 52870,
    EVENT_BACK_TO_THE_MANIFEST          = 52898,
    EVENT_THE_FINAL_WING                = 52899,
    EVENT_BARADIN_HOLD                  = 52900,
    EVENT_THE_EYE_OF_THE_BEAST          = 52901,

    NPC_NAGAZ                           = 109955,
    NPC_ALLARIS                         = 109956,
    NPC_TYRANIS_MALEM                   = 109964,
    NPC_CREDIT_WON_SCENARIO             = 109836,
    NPC_CREDIT_POST_SCENARIO            = 109837,

    WORLD_SAFE_LOC_DBLOCK_GRAVEYARD     = 5640,
    WORLD_SAFE_LOC_BARADIN_GRAVEYARD    = 5641,
    WORLD_SAFE_LOC_DALARAN_LANDING      = 5711,

    SAY_SURVIVOR_OTHERS_IN_DBLOCK       = 0, // 116314
    SAY_ALLARIS_THE_EYE_IS_HERE         = 0, // 116383
    SAY_ALLARIS_NAGAZ_SENT_DISCIPLE     = 1, // 116385
    SAY_ALLARIS_TYRANIS_TOO_LATE        = 2, // 116456
    SAY_ALLARIS_CAME_UP_EMPTY           = 3, // 116484
    SAY_ALLARIS_VOID_TERROR             = 4, // 116489
    SAY_ALLARIS_DANGEROUS_PRISONERS     = 5, // 116967
    SAY_ALLARIS_HORDE_AND_ALLIANCE      = 6, // 116968
    SAY_ALLARIS_NEVER_FACED             = 7, // 116969
    SAY_ALLARIS_THE_BEAST_FALLS         = 8, // 116972
    SAY_ALLARIS_ILL_RETRIEVE_THE_EYE    = 9, // 116973
    SAY_ALLARIS_I_HAVE_THE_EYE          = 10, // 116974
    SAY_NAGAZ_TYRANIS_WENT_RIGHT        = 0, // 116387
    SAY_TYRANIS_AT_GULDANS_SIDE         = 0, // 116459

    GOSSIP_ACTION_WHAT_DO_WE_KNOW       = 1,
    GOSSIP_ACTION_THE_MANIFEST          = 2,
    GOSSIP_ACTION_TO_BARADIN_HOLD       = 3,
    GOSSIP_ACTION_BREAK_THE_CHAIN       = 4,
    GOSSIP_ACTION_LEAVE_HIM             = 5
};

Position const BaradinHoldArrival = { -354.91f, -4528.85f, 170.43f, 0.0f };

static bool IsOnStep(Map const* map, uint32 stepId)
{
    InstanceMap const* instance = map->ToInstanceMap();
    InstanceScenario const* scenario = instance ? instance->GetInstanceScenario() : nullptr;
    return scenario && scenario->GetStep() && scenario->GetStep()->ID == stepId;
}

// Sent with the player as the source: GameEvents::Trigger then raises both PlayerTriggerGameEvent (step 2715) and
// AnyoneTriggerGameEventScenario (every other step)
static void SendEvent(Player* player, uint32 eventId)
{
    GameEvents::Trigger(eventId, player, nullptr);
}

static void TeleportToSafeLoc(Player* player, uint32 worldSafeLocId)
{
    if (WorldSafeLocsEntry const* loc = sWorldSafeLocsStore.LookupEntry(worldSafeLocId))
        player->TeleportTo(loc->MapID, loc->Loc.X, loc->Loc.Y, loc->Loc.Z, loc->Facing * float(M_PI) / 180.0f);
}

static void AllarisSay(WorldObject* near, uint8 group, Player* player)
{
    if (Creature* allaris = near->FindNearestCreature(NPC_ALLARIS, 200.0f))
        allaris->AI()->Talk(group, player);
}

static bool BelongsInTheScenario(Player const* player)
{
    return player->GetClass() == CLASS_WARLOCK && player->GetQuestStatus(QUEST_AN_EYE_FOR_A_SCEPTER) == QUEST_STATUS_INCOMPLETE;
}

class instance_tol_barad_warlock_scenario : public InstanceMapScript
{
public:
    instance_tol_barad_warlock_scenario() : InstanceMapScript("instance_tol_barad_warlock_scenario", MAP_TOL_BARAD_WARLOCK_SCENARIO) { }

    struct instance_tol_barad_warlock_scenario_InstanceMapScript : public InstanceScript
    {
        instance_tol_barad_warlock_scenario_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders("TolBaradWarlockScenario");
            SetEntranceLocation(WORLD_SAFE_LOC_DBLOCK_GRAVEYARD);
        }

        void OnPlayerEnter(Player* player) override
        {
            if (player->IsGameMaster() || BelongsInTheScenario(player))
                return;

            player->m_Events.AddEventAtOffset([player]
            {
                if (player->GetMapId() == MAP_TOL_BARAD_WARLOCK_SCENARIO && !player->IsBeingTeleported() && !BelongsInTheScenario(player))
                    TeleportToSafeLoc(player, WORLD_SAFE_LOC_DALARAN_LANDING);
            }, 2s);
        }

        void SetData(uint32 type, uint32 /*data*/) override
        {
            if (type == EVENT_BARADIN_HOLD)
                SetEntranceLocation(WORLD_SAFE_LOC_BARADIN_GRAVEYARD);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_tol_barad_warlock_scenario_InstanceMapScript(map);
    }
};

// 218595 - Teleport: Tol Barad (TELEPORT_TO_LFG_DUNGEON 1368, which this core does not handle): into the scenario at
// its entry, WorldSafeLocs 5562. Effect 1 credits "Take the Portal to Tol Barad" as data
class spell_warlock_teleport_tol_barad : public SpellScript
{
    void HandleTeleport(SpellEffIndex /*effIndex*/) const
    {
        if (Player* player = GetHitPlayer())
            TeleportToSafeLoc(player, 5562);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warlock_teleport_tol_barad::HandleTeleport, EFFECT_0, SPELL_EFFECT_TELEPORT_TO_LFG_DUNGEON);
    }
};

// 109952 - Dark-Sworn Warlock, the wounded survivor on the way in
struct npc_dark_sworn_survivor : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void MoveInLineOfSight(Unit* who) override
    {
        if (!_spoke && who->IsPlayer() && me->IsWithinDistInMap(who, 20.0f) && BelongsInTheScenario(who->ToPlayer()))
        {
            _spoke = true;
            Talk(SAY_SURVIVOR_OTHERS_IN_DBLOCK, who);
        }
    }

private:
    bool _spoke = false;
};

// 109956 - Allaris Narassin: the plan (step 2715, with Nagaz), the manifest (2724), the way to Baradin Hold (2725)
struct npc_allaris_tol_barad : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetGossipMenuId(me->GetEntry());
    }

    bool OnGossipHello(Player* player) override
    {
        Map* map = me->GetMap();
        if (IsOnStep(map, STEP_THE_OTHER_PLAYERS))
            AddGossipItemFor(player, GossipOptionNpc::None, "What do we know?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_WHAT_DO_WE_KNOW);
        else if (IsOnStep(map, STEP_THE_FINAL_WING))
            AddGossipItemFor(player, GossipOptionNpc::None, "What does the manifest say?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_THE_MANIFEST);
        else if (IsOnStep(map, STEP_BARADIN_HOLD))
            AddGossipItemFor(player, GossipOptionNpc::None, "Take us to Baradin Hold.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TO_BARADIN_HOLD);
        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        uint32 const action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
        CloseGossipMenuFor(player);
        ObjectGuid const playerGuid = player->GetGUID();
        switch (action)
        {
            case GOSSIP_ACTION_WHAT_DO_WE_KNOW:
                if (!IsOnStep(me->GetMap(), STEP_THE_OTHER_PLAYERS))
                    break;
                Talk(SAY_ALLARIS_THE_EYE_IS_HERE, player);
                _scheduler.Schedule(6s, [this, playerGuid](TaskContext /*context*/) { Talk(SAY_ALLARIS_NAGAZ_SENT_DISCIPLE, Get(playerGuid)); })
                    .Schedule(12s, [this, playerGuid](TaskContext /*context*/)
                    {
                        if (Creature* nagaz = me->FindNearestCreature(NPC_NAGAZ, 30.0f))
                            nagaz->AI()->Talk(SAY_NAGAZ_TYRANIS_WENT_RIGHT, Get(playerGuid));
                        if (Player* player = Get(playerGuid))
                            if (IsOnStep(me->GetMap(), STEP_THE_OTHER_PLAYERS))
                                SendEvent(player, EVENT_THE_OTHER_PLAYERS);
                    });
                break;
            case GOSSIP_ACTION_THE_MANIFEST:
                if (!IsOnStep(me->GetMap(), STEP_THE_FINAL_WING))
                    break;
                Talk(SAY_ALLARIS_VOID_TERROR, player);
                SendEvent(player, EVENT_THE_FINAL_WING);
                break;
            case GOSSIP_ACTION_TO_BARADIN_HOLD:
                if (IsOnStep(me->GetMap(), STEP_BARADIN_HOLD))
                    player->NearTeleportTo(BaradinHoldArrival);
                break;
            default:
                break;
        }
        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    Player* Get(ObjectGuid guid) const { return ObjectAccessor::GetPlayer(*me, guid); }
    TaskScheduler _scheduler;
};

// 109964 - Tyranis Malem, chained deep in D-Block: break his chain or leave him (step 2717); he does not survive either
struct npc_tyranis_malem : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetGossipMenuId(me->GetEntry());
    }

    bool OnGossipHello(Player* player) override
    {
        if (IsOnStep(me->GetMap(), STEP_AN_UNFORTUNATE_ACCIDENT))
        {
            AddGossipItemFor(player, GossipOptionNpc::None, "<Break the chain.>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_BREAK_THE_CHAIN);
            AddGossipItemFor(player, GossipOptionNpc::None, "<Leave him to his fate.>", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_LEAVE_HIM);
        }
        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (!IsOnStep(me->GetMap(), STEP_AN_UNFORTUNATE_ACCIDENT) || _done)
            return true;

        _done = true;
        Talk(SAY_TYRANIS_AT_GULDANS_SIDE, player);
        ObjectGuid const playerGuid = player->GetGUID();
        me->m_Events.AddEventAtOffset([this, playerGuid]
        {
            me->KillSelf();
            if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
            {
                AllarisSay(me, SAY_ALLARIS_TYRANIS_TOO_LATE, player);
                SendEvent(player, EVENT_AN_UNFORTUNATE_ACCIDENT);
            }
        }, 4s);
        return true;
    }

private:
    bool _done = false;
};

// 109955 - Nagaz at the end of the cell block: "help with something" is his trap (step 2719); he turns on the warlock
struct npc_nagaz_trap : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void SetGUID(ObjectGuid const& playerGuid, int32 /*id*/) override
    {
        if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
        {
            me->SetFaction(FACTION_MONSTER);
            AttackStart(player);
        }
    }

    void JustDied(Unit* killer) override
    {
        if (Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr)
            if (IsOnStep(me->GetMap(), STEP_A_SECRET_MEETING))
                SendEvent(player, EVENT_A_SECRET_MEETING);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }
};

// 109981 - Occul'tharon, the void terror in Baradin Hold that swallowed the Eye of Dalaran (step 2726). His death ends
// the scenario: Allaris takes the Eye, and the warlock goes back to Krasus' Landing (WorldSafeLocs 5711)
struct npc_occultharon : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
    }

    void JustEngagedWith(Unit* who) override
    {
        if (Player* player = who->GetCharmerOrOwnerPlayerOrPlayerItself())
            AllarisSay(me, SAY_ALLARIS_NEVER_FACED, player);
    }

    void JustDied(Unit* killer) override
    {
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!player || !IsOnStep(me->GetMap(), STEP_THE_EYE_OF_THE_BEAST))
            return;

        SendEvent(player, EVENT_THE_EYE_OF_THE_BEAST);
        player->KilledMonsterCredit(NPC_CREDIT_WON_SCENARIO);
        ObjectGuid const playerGuid = player->GetGUID();
        Creature* allaris = me->FindNearestCreature(NPC_ALLARIS, 200.0f);
        if (!allaris)
            return;
        allaris->AI()->Talk(SAY_ALLARIS_THE_BEAST_FALLS, player);
        allaris->m_Events.AddEventAtOffset([allaris, playerGuid]
        {
            allaris->AI()->Talk(SAY_ALLARIS_ILL_RETRIEVE_THE_EYE, ObjectAccessor::GetPlayer(*allaris, playerGuid));
        }, 5s);
        allaris->m_Events.AddEventAtOffset([allaris, playerGuid]
        {
            allaris->AI()->Talk(SAY_ALLARIS_I_HAVE_THE_EYE, ObjectAccessor::GetPlayer(*allaris, playerGuid));
        }, 11s);
        allaris->m_Events.AddEventAtOffset([allaris, playerGuid]
        {
            if (Player* player = ObjectAccessor::GetPlayer(*allaris, playerGuid))
                TeleportToSafeLoc(player, WORLD_SAFE_LOC_DALARAN_LANDING);
        }, 20s);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }
};

// The scenario's places (server-side areatriggers; each sends its step's event when the warlock arrives on that step)
template<uint32 Step, uint32 EventId>
struct at_tol_barad_warlock_step : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->IsGameMaster() || !IsOnStep(at->GetMap(), Step))
            return;

        SendEvent(player, EventId);
        OnStepDone(player);
    }

    virtual void OnStepDone(Player* /*player*/) { }
};

// 2700: the council's survivors at the entrance of D-Block
struct at_tol_barad_where_are_they : at_tol_barad_warlock_step<STEP_WHERE_ARE_THEY, EVENT_WHERE_ARE_THEY>
{
    using at_tol_barad_warlock_step::at_tol_barad_warlock_step;
};

// 2716: Tyranis, deeper in D-Block
struct at_tol_barad_the_third_one : at_tol_barad_warlock_step<STEP_THE_THIRD_ONE, EVENT_THE_THIRD_ONE>
{
    using at_tol_barad_warlock_step::at_tol_barad_warlock_step;
};

// 2718: the rest of the cell block, where Nagaz waits; his trap (2719) springs at once
struct at_tol_barad_back_to_searching : at_tol_barad_warlock_step<STEP_BACK_TO_SEARCHING, EVENT_BACK_TO_SEARCHING>
{
    using at_tol_barad_warlock_step::at_tol_barad_warlock_step;

    void OnStepDone(Player* player) override
    {
        AllarisSay(at, SAY_ALLARIS_CAME_UP_EMPTY, player);
        ObjectGuid const playerGuid = player->GetGUID();
        if (Creature* nagaz = at->FindNearestCreature(NPC_NAGAZ, 40.0f))
            nagaz->m_Events.AddEventAtOffset([nagaz, playerGuid] { nagaz->AI()->SetGUID(playerGuid, 0); }, 3s);
    }
};

// 2720: the final wing, where Allaris looks for the manifest
struct at_tol_barad_back_to_the_manifest : at_tol_barad_warlock_step<STEP_BACK_TO_THE_MANIFEST, EVENT_BACK_TO_THE_MANIFEST>
{
    using at_tol_barad_warlock_step::at_tol_barad_warlock_step;
};

// 2725: inside Baradin Hold
struct at_tol_barad_baradin_hold : at_tol_barad_warlock_step<STEP_BARADIN_HOLD, EVENT_BARADIN_HOLD>
{
    using at_tol_barad_warlock_step::at_tol_barad_warlock_step;

    void OnStepDone(Player* player) override
    {
        if (InstanceScript* instance = at->GetInstanceScript())
            instance->SetData(EVENT_BARADIN_HOLD, 1);
        AllarisSay(at, SAY_ALLARIS_DANGEROUS_PRISONERS, player);
        ObjectGuid const playerGuid = player->GetGUID();
        if (Creature* allaris = at->FindNearestCreature(NPC_ALLARIS, 100.0f))
            allaris->m_Events.AddEventAtOffset([allaris, playerGuid]
            {
                allaris->AI()->Talk(SAY_ALLARIS_HORDE_AND_ALLIANCE, ObjectAccessor::GetPlayer(*allaris, playerGuid));
            }, 7s);
    }
};

// Krasus' Landing: "Return to Calydus in Dalaran" (109837), once the scenario is won
struct at_krasus_landing_calydus_post_scenario : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (player && player->GetQuestStatus(QUEST_AN_EYE_FOR_A_SCEPTER) == QUEST_STATUS_INCOMPLETE
            && player->GetQuestObjectiveData(QUEST_AN_EYE_FOR_A_SCEPTER, QUEST_OBJECTIVE_WON_SCENARIO) > 0)
            player->KilledMonsterCredit(NPC_CREDIT_POST_SCENARIO);
    }
};
} // namespace TolBaradWarlockScenario

void AddSC_orderhall_warlock()
{
    {
        using namespace TolBaradWarlockScenario;
        new instance_tol_barad_warlock_scenario();
        RegisterSpellScript(spell_warlock_teleport_tol_barad);
        RegisterCreatureAI(npc_dark_sworn_survivor);
        RegisterCreatureAI(npc_allaris_tol_barad);
        RegisterCreatureAI(npc_tyranis_malem);
        RegisterCreatureAI(npc_nagaz_trap);
        RegisterCreatureAI(npc_occultharon);
        RegisterAreaTriggerAI(at_tol_barad_where_are_they);
        RegisterAreaTriggerAI(at_tol_barad_the_third_one);
        RegisterAreaTriggerAI(at_tol_barad_back_to_searching);
        RegisterAreaTriggerAI(at_tol_barad_back_to_the_manifest);
        RegisterAreaTriggerAI(at_tol_barad_baradin_hold);
        RegisterAreaTriggerAI(at_krasus_landing_calydus_post_scenario);
    }
    new player_warlock_finding_the_scepter();
    RegisterGameObjectAI(go_shadow_council_information);
    RegisterCreatureAI(npc_jergosh_the_invoker);
    RegisterCreatureAI(npc_calydus_caer_darrow);
    RegisterCreatureAI(npc_risstyns_portal);
    RegisterCreatureAI(npc_ritssyn_flamescowl_the_sixth);
    RegisterCreatureAI(npc_ritssyn_flamescowl_circle_of_wills);
    RegisterCreatureAI(npc_calydus_circle_of_wills);
    new playerchoice_the_tome_of_blighted_implements();
    RegisterSpellScript(spell_warlock_force_specialization);
}
