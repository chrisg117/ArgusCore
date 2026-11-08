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
#include "DB2Stores.h"
#include "GameObject.h"
#include "MotionMaster.h"
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

void AddSC_orderhall_warlock()
{
    RegisterCreatureAI(npc_risstyns_portal);
    RegisterCreatureAI(npc_ritssyn_flamescowl_the_sixth);
    RegisterCreatureAI(npc_ritssyn_flamescowl_circle_of_wills);
    RegisterCreatureAI(npc_calydus_circle_of_wills);
    new playerchoice_the_tome_of_blighted_implements();
    RegisterSpellScript(spell_warlock_force_specialization);
}
