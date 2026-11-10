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
#include "GameTables.h"
#include "Item.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerChoice.h"
#include "QuestDef.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellAuras.h"
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

    // The Circle copies share CreatureDifficultyIDs 101973-101979 with Ritssyn 101456 and the Jailer 101460; Zinnin
    // (101454) is not seen in the Circle in the video
    NPC_SHINFEL_CIRCLE                  = 101455,
    NPC_JUBEKA_CIRCLE                   = 101457,
    NPC_ZELIFRAX_WOBBLEPOX              = 98797,
    NPC_JAGGANOTH_CIRCLE                = 101468,
    NPC_DREADSCAR_JAILER                = 101460,
    NPC_WORLD_TRIGGER                   = 22515,   // what the council's Fel Ritual channels on, in the gateway

    GO_RITUAL_PORTAL                    = 243679,  // the data's "Ritual Portal" (display 30007): ready = dark, active = the vortex

    SPELL_START_SCENARIO                = 199359, // teleport to Jagganoth's prison (map 1522); credits Ritual Performed
    SPELL_FEL_RITUAL                    = 199426, // the council's channel on the gateway (conditions: World Trigger)
    SPELL_GRAB_ZELIFRAX                 = 200016, // Jagganoth seizes the gnome (conditions: Zelifrax)
    SPELL_TOME_PLAYER_CHOICE            = 199411, // launches player choice 245
    SPELL_DESTRUCTION                   = 199409,

    PLAYERCHOICE_RESPONSE_DESTRUCTION   = 2451,

    // Ritssyn 101456
    SAY_RITSSYN_THE_SIXTH_IS_HERE       = 0, // 103743
    SAY_RITSSYN_ZELIFRAX_LEADS          = 1, // 103744
    SAY_RITSSYN_GREAT_POWER             = 2, // 103753
    SAY_RITSSYN_ZELIFRAX                = 3, // 103755
    // Zelifrax 98797
    SAY_ZELIFRAX_KNOWS_THE_JOB          = 0, // 103745
    SAY_ZELIFRAX_INCANTATION_1          = 1, // 103746
    SAY_ZELIFRAX_CHANNEL                = 2, // 103747
    SAY_ZELIFRAX_RITUAL_BEGINS          = 3, // 103750
    SAY_ZELIFRAX_INCANTATION_2          = 4, // 103764
    SAY_ZELIFRAX_INCANTATION_3          = 5, // 103752
    SAY_ZELIFRAX_INCANTATION_4          = 6, // 103754
    SAY_ZELIFRAX_MIXES_WORDS            = 7, // 103756
    SAY_ZELIFRAX_COMMANDS               = 8, // 103759
    SAY_ZELIFRAX_SCREAM                 = 9, // 103761
    // Shinfel 101455
    SAY_SHINFEL_WORRY_ABOUT_NEWCOMER    = 0, // 103748
    SAY_SHINFEL_BEGIN                   = 1, // 103749
    // Jubeka 101457
    SAY_JUBEKA_DEEPER                   = 0, // 103751
    SAY_JUBEKA_THE_PORTAL               = 1, // 103757
    // Jagganoth 101468
    SAY_JAGGANOTH_WHO_SUMMONS           = 0, // 103758
    SAY_JAGGANOTH_SCORNS                = 1, // 103760
    SAY_JAGGANOTH_FOOLISH_MORTALS       = 2, // 103762
    SAY_JAGGANOTH_SERVE_ME              = 3, // 103763

    GOSSIP_ACTION_BEGIN_RITUAL          = 1,
    GOSSIP_ACTION_REVIEW_WEAPONS        = 2
};

// The Circle of Wills, as the video shows it from the player's place in the middle of the grate facing the gateway:
// Ritssyn (his spawn, -842, 4447) at one edge with Zelifrax just ahead of him; Shinfel a third of the way to the other
// edge and Jubeka at it; the gateway across the grate. Nobody walks in: they stand there from the accept
Position const RitualGrateCentre = { -856.05f, 4453.25f, 696.67f, 0.0f };
Position const RitualZelifrax  = { -846.00f, 4451.00f, 696.90f, 2.60f };
Position const RitualShinfel   = { -861.70f, 4451.20f, 696.75f, 1.30f };
Position const RitualJubeka    = { -868.30f, 4448.90f, 696.75f, 1.10f };
Position const RitualGateway   = { -862.00f, 4470.00f, 696.75f, 5.00f };
Position const RitualJagganoth = { -860.80f, 4466.70f, 696.75f, 5.00f };
Position const RitualZelifraxAhead = { -858.40f, 4459.80f, 696.75f, 1.90f };

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
        ObjectGuid const p = player->GetGUID();
        _summons.clear();
        ObjectGuid const zelifrax = Summon(NPC_ZELIFRAX_WOBBLEPOX, RitualZelifrax);
        ObjectGuid const shinfel = Summon(NPC_SHINFEL_CIRCLE, RitualShinfel);
        ObjectGuid const jubeka = Summon(NPC_JUBEKA_CIRCLE, RitualJubeka);

        // Times are from the accept, as in Wowhead's video (accept 0:47, fade 3:15, loading 3:18)
        _scheduler.Schedule(3s, [this, p](TaskContext) { Talk(SAY_RITSSYN_THE_SIXTH_IS_HERE, Get(p)); })
            .Schedule(8s, [this, p](TaskContext) { Talk(SAY_RITSSYN_ZELIFRAX_LEADS, Get(p)); })
            .Schedule(13s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_KNOWS_THE_JOB, p); })
            .Schedule(19s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_INCANTATION_1, p); })
            .Schedule(25s, [this, zelifrax, p](TaskContext)
            {
                TalkFrom(zelifrax, SAY_ZELIFRAX_CHANNEL, p);
                // the gateway appears dark (ready); it ignites at 48 s
                if (GameObject* gateway = me->SummonGameObject(GO_RITUAL_PORTAL, RitualGateway, QuaternionData::fromEulerAnglesZYX(RitualGateway.GetOrientation(), 0.0f, 0.0f), 170s))
                {
                    gateway->SetGoState(GO_STATE_READY);
                    _gateway = gateway->GetGUID();
                }
            })
            .Schedule(34s, [this, shinfel, p](TaskContext) { TalkFrom(shinfel, SAY_SHINFEL_WORRY_ABOUT_NEWCOMER, p); })
            .Schedule(42s, [this, shinfel, p](TaskContext) { TalkFrom(shinfel, SAY_SHINFEL_BEGIN, p); })
            .Schedule(47s, [this, zelifrax, shinfel, jubeka](TaskContext)
            {
                // all channel on the gateway (the player's own glow in the video has no spell in the data)
                Creature* trigger = me->SummonCreature(NPC_WORLD_TRIGGER, RitualGateway.GetPositionX(), RitualGateway.GetPositionY(),
                    RitualGateway.GetPositionZ() + 6.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 110s);
                if (!trigger)
                    return;
                _summons.push_back(trigger->GetGUID());
                for (ObjectGuid const& guid : { me->GetGUID(), zelifrax, shinfel, jubeka })
                    if (Creature* member = ObjectAccessor::GetCreature(*me, guid))
                        member->CastSpell(trigger, SPELL_FEL_RITUAL, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            })
            .Schedule(48s, [this](TaskContext)
            {
                if (GameObject* gateway = ObjectAccessor::GetGameObject(*me, _gateway))
                    gateway->SetGoState(GO_STATE_ACTIVE);
            })
            .Schedule(49s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_RITUAL_BEGINS, p); })
            .Schedule(55s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_INCANTATION_2, p); })
            .Schedule(57s, [this, jubeka, p](TaskContext) { TalkFrom(jubeka, SAY_JUBEKA_DEEPER, p); })
            .Schedule(68s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_INCANTATION_3, p); })
            .Schedule(76s, [this, p](TaskContext) { Talk(SAY_RITSSYN_GREAT_POWER, Get(p)); })
            .Schedule(83s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_INCANTATION_4, p); })
            .Schedule(85s, [this, p](TaskContext) { Talk(SAY_RITSSYN_ZELIFRAX, Get(p)); })
            .Schedule(89s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_MIXES_WORDS, p); })
            .Schedule(100s, [this, jubeka, p](TaskContext) { TalkFrom(jubeka, SAY_JUBEKA_THE_PORTAL, p); })
            .Schedule(107s, [this, p](TaskContext)
            {
                // heard from inside the gateway before he is seen (the chat names him "Unknown")
                if (Creature* jagganoth = me->SummonCreature(NPC_JAGGANOTH_CIRCLE, RitualGateway, TEMPSUMMON_TIMED_DESPAWN, 60s))
                {
                    _jagganoth = jagganoth->GetGUID();
                    _summons.push_back(_jagganoth);
                    jagganoth->SetImmuneToAll(true);
                    jagganoth->AI()->Talk(SAY_JAGGANOTH_WHO_SUMMONS, Get(p));
                }
            })
            .Schedule(108s, [this](TaskContext)
            {
                // steps out to stand in the gateway's mouth, and no further
                if (Creature* jagganoth = ObjectAccessor::GetCreature(*me, _jagganoth))
                    jagganoth->GetMotionMaster()->MovePoint(0, RitualJagganoth);
            })
            .Schedule(114s, [this, zelifrax, p](TaskContext) { TalkFrom(zelifrax, SAY_ZELIFRAX_COMMANDS, p); })
            .Schedule(117s, [this, zelifrax](TaskContext)
            {
                if (Creature* gnome = ObjectAccessor::GetCreature(*me, zelifrax))
                {
                    gnome->InterruptNonMeleeSpells(false);
                    gnome->GetMotionMaster()->MovePoint(0, RitualZelifraxAhead);
                }
            })
            .Schedule(123s, [this, p](TaskContext)
            {
                if (Creature* jagganoth = ObjectAccessor::GetCreature(*me, _jagganoth))
                    jagganoth->AI()->Talk(SAY_JAGGANOTH_SCORNS, Get(p));
            })
            .Schedule(124s, [this, zelifrax](TaskContext)
            {
                Creature* jagganoth = ObjectAccessor::GetCreature(*me, _jagganoth);
                Creature* gnome = ObjectAccessor::GetCreature(*me, zelifrax);
                if (!jagganoth || !gnome)
                    return;
                jagganoth->SetFacingToObject(gnome);
                jagganoth->HandleEmoteCommand(EMOTE_ONESHOT_ATTACK1H);
                jagganoth->CastSpell(gnome, SPELL_GRAB_ZELIFRAX, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            })
            .Schedule(127s, [this, zelifrax, p](TaskContext)
            {
                TalkFrom(zelifrax, SAY_ZELIFRAX_SCREAM, p);
                if (Creature* gnome = ObjectAccessor::GetCreature(*me, zelifrax))
                    gnome->DespawnOrUnsummon(1s);
            })
            .Schedule(133s, [this, p](TaskContext)
            {
                if (Creature* jagganoth = ObjectAccessor::GetCreature(*me, _jagganoth))
                    jagganoth->AI()->Talk(SAY_JAGGANOTH_FOOLISH_MORTALS, Get(p));
            })
            .Schedule(142s, [this, p](TaskContext)
            {
                if (Creature* jagganoth = ObjectAccessor::GetCreature(*me, _jagganoth))
                    jagganoth->AI()->Talk(SAY_JAGGANOTH_SERVE_ME, Get(p));
            })
            .Schedule(143s, [this, shinfel, jubeka, p](TaskContext)
            {
                // four Jailers leap from the gateway onto the council and the warlock
                for (ObjectGuid const& guid : { me->GetGUID(), shinfel, jubeka, p })
                {
                    Unit* target = ObjectAccessor::GetUnit(*me, guid);
                    if (!target)
                        continue;
                    if (Creature* jailer = me->SummonCreature(NPC_DREADSCAR_JAILER, RitualGateway, TEMPSUMMON_TIMED_DESPAWN, 15s))
                    {
                        jailer->SetImmuneToAll(true);
                        Position landing = target->GetNearPosition(2.0f, target->GetRelativeAngle(RitualGateway));
                        jailer->GetMotionMaster()->MoveJump(landing, 25.0f, 12.0f);
                        _summons.push_back(jailer->GetGUID());
                    }
                }
            })
            .Schedule(148s, [this, p](TaskContext)
            {
                // the teleport's screen effect is the fade to black
                if (Player* player = Get(p))
                    if (player->GetQuestStatus(QUEST_THE_NEW_BLOOD) == QUEST_STATUS_INCOMPLETE)
                        player->CastSpell(player, SPELL_START_SCENARIO, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            })
            .Schedule(155s, [this](TaskContext)
            {
                for (ObjectGuid const& guid : _summons)
                    if (Creature* summon = ObjectAccessor::GetCreature(*me, guid))
                        summon->DespawnOrUnsummon();
                _summons.clear();
                if (GameObject* gateway = ObjectAccessor::GetGameObject(*me, _gateway))
                    gateway->DespawnOrUnsummon();
                _gateway.Clear();
                _jagganoth.Clear();
                _ritualRunning = false;
            });
    }

    ObjectGuid Summon(uint32 entry, Position const& pos)
    {
        if (Creature* summon = me->SummonCreature(entry, pos, TEMPSUMMON_TIMED_DESPAWN, 170s))
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
    ObjectGuid _jagganoth;
    bool _ritualRunning = false;
};

// 101097 - Calydus, back in the Circle of Wills after the escape: The Tome of Blighted Implements. Taking it opens the
// weapon choice (199411 launches player choice 245, as in the data); his gossip opens it again until a weapon is chosen
struct npc_calydus_circle_of_wills : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    enum
    {
        SAY_CALYDUS_THEY_DID_IT         = 0, // 103567 (spoken as the Circle loads)
        SAY_CALYDUS_FIND_THE_WEAPON     = 1  // 103568
    };

    // The warlock lands beside him from the prison: The New Blood rewarded by the scenario, the Tome not yet taken
    void MoveInLineOfSight(Unit* who) override
    {
        Player* player = who->ToPlayer();
        if (!player || player->IsGameMaster() || _greeted.count(player->GetGUID()) || !me->IsWithinDist(player, 30.0f))
            return;
        if (player->GetQuestStatus(QUEST_THE_NEW_BLOOD) != QUEST_STATUS_REWARDED
            || player->GetQuestStatus(QUEST_THE_TOME_OF_BLIGHTED_IMPLEMENTS) != QUEST_STATUS_NONE)
            return;
        _greeted.insert(player->GetGUID());
        Talk(SAY_CALYDUS_THEY_DID_IT, player);
        ObjectGuid const guid = player->GetGUID();
        _scheduler.Schedule(8s, [this, guid](TaskContext)
        {
            if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
                Talk(SAY_CALYDUS_FIND_THE_WEAPON, player);
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

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

private:
    TaskScheduler _scheduler;
    GuidUnorderedSet _greeted;
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

// ---------------------------------------------------------------------------------------------------------------------
// Ritual Ruination (43254): Gul'dan's ritual at the Tomb of Sargeras (map 1220, the Broken Shore). Flow first, after
// Wowhead's acquisition video 37:05-44:24; the lines are the client's broadcast texts.
namespace RitualRuination
{
enum Data
{
    QUEST_RITUAL_RUINATION              = 43254,
    QUEST_OBJECTIVE_ARTIFACT_LOOTED     = 285268,
    QUEST_OBJECTIVE_RITUAL_RUINED       = 285269,

    NPC_CREDIT_FEL_BAT_RIDE             = 110523,
    NPC_CREDIT_LISTENED_TO_GULDAN       = 110581,
    NPC_CREDIT_KILLED_ALLARIS           = 114337,
    NPC_GULDAN                          = 110533,
    NPC_ALLARIS_RITUAL                  = 110556,
    NPC_CALYDUS_ESCAPE                  = 109838,
    GO_RITUAL_PORTAL                    = 251763,

    ITEM_SCEPTER_OF_SARGERAS            = 128941,

    SPELL_PORTAL_OVERLOAD               = 219920, // periodic: 219923 credits Ruin the ritual and removes the guise
    SPELL_DEMONIC_PORTAL_TO_DALARAN     = 219705, // credits the escape; its periodic 221318 teleports (spell_target_position)

    SAY_GULDAN_REINFORCEMENTS           = 0, // 116381
    SAY_GULDAN_CONVENIENT               = 1, // 116382
    SAY_GULDAN_AMPLIFY                  = 2, // 117566
    SAY_GULDAN_ANCHOR                   = 3, // 117567
    SAY_GULDAN_ONLY_ONE                 = 4, // 117576
    SAY_GULDAN_THE_VICTOR               = 5, // 117590
    SAY_GULDAN_THE_TIME_HAS_COME        = 6, // 117591
    SAY_ALLARIS_WHICH_OF_US             = 0, // 117568
    SAY_CALYDUS_TIME_TO_GO              = 0  // 117620
};

Position const RitualArrival = { -690.0f, 2640.0f, 78.39f, 5.30f };

// 110480 - Fel Bat at Krasus' Landing: the ride to the ritual (a teleport for now; the flight is a fidelity item)
struct npc_fel_bat_ritual_ruination : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetGossipMenuId(me->GetEntry());
    }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_RITUAL_RUINATION) == QUEST_STATUS_INCOMPLETE)
        {
            player->KilledMonsterCredit(NPC_CREDIT_FEL_BAT_RIDE, me->GetGUID()); // 43254 has NO_CREDIT_FOR_PROXY: a credit needs a source guid
            player->TeleportTo(me->GetMapId(), RitualArrival.GetPositionX(), RitualArrival.GetPositionY(), RitualArrival.GetPositionZ(), RitualArrival.GetOrientation());
        }
        return true;
    }
};

// 110533 - Gul'dan at the ritual: he takes the warlock for Jergosh's stand-in, sets the two aspirants against each other,
// and once Allaris falls starts the ritual and goes through the portal
struct npc_guldan_ritual_ruination : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void MoveInLineOfSight(Unit* who) override
    {
        Player* player = who->ToPlayer();
        if (_started || !player || !me->IsWithinDistInMap(player, 35.0f)
            || player->GetQuestStatus(QUEST_RITUAL_RUINATION) != QUEST_STATUS_INCOMPLETE)
            return;

        _started = true;
        ObjectGuid const playerGuid = player->GetGUID();
        Talk(SAY_GULDAN_REINFORCEMENTS, player);
        _scheduler.Schedule(7s, [this, playerGuid](TaskContext /*context*/) { Talk(SAY_GULDAN_CONVENIENT, Get(playerGuid)); })
            .Schedule(14s, [this, playerGuid](TaskContext /*context*/) { Talk(SAY_GULDAN_AMPLIFY, Get(playerGuid)); })
            .Schedule(21s, [this, playerGuid](TaskContext /*context*/) { Talk(SAY_GULDAN_ANCHOR, Get(playerGuid)); })
            .Schedule(27s, [this, playerGuid](TaskContext /*context*/)
            {
                if (Creature* allaris = me->FindNearestCreature(NPC_ALLARIS_RITUAL, 60.0f))
                    allaris->AI()->Talk(SAY_ALLARIS_WHICH_OF_US, Get(playerGuid));
            })
            .Schedule(32s, [this, playerGuid](TaskContext /*context*/)
            {
                Player* player = Get(playerGuid);
                Talk(SAY_GULDAN_ONLY_ONE, player);
                if (!player)
                    return;
                player->KilledMonsterCredit(NPC_CREDIT_LISTENED_TO_GULDAN, me->GetGUID());
                if (Creature* allaris = me->FindNearestCreature(NPC_ALLARIS_RITUAL, 60.0f))
                {
                    allaris->SetFaction(FACTION_MONSTER);
                    allaris->AI()->AttackStart(player);
                }
            });
    }

    // Allaris is dead (npc_allaris_ritual_ruination)
    void SetGUID(ObjectGuid const& playerGuid, int32 /*id*/) override
    {
        _scheduler.Schedule(4s, [this, playerGuid](TaskContext /*context*/) { Talk(SAY_GULDAN_THE_VICTOR, Get(playerGuid)); })
            .Schedule(12s, [this, playerGuid](TaskContext /*context*/) { Talk(SAY_GULDAN_THE_TIME_HAS_COME, Get(playerGuid)); })
            .Schedule(18s, [this](TaskContext /*context*/) { me->DespawnOrUnsummon(0s, 5min); });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    Player* Get(ObjectGuid guid) const { return ObjectAccessor::GetPlayer(*me, guid); }
    TaskScheduler _scheduler;
    bool _started = false;
};

// 110556 - Allaris Narassin, the other aspirant: fought once Gul'dan says only one is needed
struct npc_allaris_ritual_ruination : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustDied(Unit* killer) override
    {
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!player)
            return;
        player->KilledMonsterCredit(NPC_CREDIT_KILLED_ALLARIS, me->GetGUID());
        if (Creature* guldan = me->FindNearestCreature(NPC_GULDAN, 80.0f))
            guldan->AI()->SetGUID(player->GetGUID(), 0);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }
};

// 219839 - Scepter of Sargeras (the click on the Scepter on the altar): FORCE_EQUIP_ITEM is not handled by this core,
// so the Scepter is given and equipped here. The credit (effect 3) is data; the scene 1365 waits for a preview
class spell_warlock_take_the_scepter : public SpellScript
{
    void HandleEquip(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitPlayer();
        if (!player || player->HasItemCount(ITEM_SCEPTER_OF_SARGERAS, 1, true))
            return;

        uint16 dest = 0;
        if (player->CanEquipNewItem(NULL_SLOT, dest, ITEM_SCEPTER_OF_SARGERAS, false) == EQUIP_ERR_OK)
            player->EquipNewItem(dest, ITEM_SCEPTER_OF_SARGERAS, ItemContext::NONE, true);
        else
            player->AddItem(ITEM_SCEPTER_OF_SARGERAS, 1);
    }

    // Effects 0 and 1 are the scene 1365 and its root: held back until the scene is previewed (advisor ruling 2026-11-08)
    void HoldBackScene(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void DropSceneAura()
    {
        PreventHitAura();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warlock_take_the_scepter::HoldBackScene, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
        OnEffectHitTarget += SpellEffectFn(spell_warlock_take_the_scepter::HoldBackScene, EFFECT_1, SPELL_EFFECT_APPLY_AURA);
        OnEffectHitTarget += SpellEffectFn(spell_warlock_take_the_scepter::HandleEquip, EFFECT_2, SPELL_EFFECT_FORCE_EQUIP_ITEM);
        AfterHit += SpellHitFn(spell_warlock_take_the_scepter::DropSceneAura);
    }
};

static bool CanRuinTheRitual(Player* player)
{
    return player->GetQuestStatus(QUEST_RITUAL_RUINATION) == QUEST_STATUS_INCOMPLETE
        && player->GetQuestObjectiveData(QUEST_RITUAL_RUINATION, QUEST_OBJECTIVE_ARTIFACT_LOOTED) > 0
        && player->GetQuestObjectiveData(QUEST_RITUAL_RUINATION, QUEST_OBJECTIVE_RITUAL_RUINED) == 0
        && !player->HasAura(SPELL_PORTAL_OVERLOAD);
}

// 251763 - Legion Portal, the ritual's portal: with the Scepter in hand, a click overloads it (the fallback to the
// Dimensional Rift below)
struct go_ritual_ruination_portal : public GameObjectAI
{
    go_ritual_ruination_portal(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (CanRuinTheRitual(player))
            player->CastSpell(player, SPELL_PORTAL_OVERLOAD, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        return true;
    }
};

// 196586 - Dimensional Rift (the Scepter's ability): cast near the ritual's portal, it overloads it
class spell_warlock_dimensional_rift_ritual : public SpellScript
{
    void HandleAfterCast() const
    {
        Player* player = GetCaster()->ToPlayer();
        if (player && CanRuinTheRitual(player) && player->FindNearestGameObject(GO_RITUAL_PORTAL, 40.0f))
            player->CastSpell(player, SPELL_PORTAL_OVERLOAD, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_warlock_dimensional_rift_ritual::HandleAfterCast);
    }
};

// 219923 - Portal Overload (the tick): the ritual is ruined; Calydus opens the way out ("Time to go, friend! Here!")
class spell_warlock_portal_overload : public SpellScript
{
    void HandleAfterHit() const
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_RITUAL_RUINATION) != QUEST_STATUS_INCOMPLETE)
            return;

        Position pos = player->GetPosition();
        player->MovePosition(pos, 3.0f, 0.0f);
        if (Creature* calydus = player->SummonCreature(NPC_CALYDUS_ESCAPE, pos, TEMPSUMMON_TIMED_DESPAWN, 30s))
            calydus->AI()->Talk(SAY_CALYDUS_TIME_TO_GO, player);
        player->m_Events.AddEventAtOffset([player]
        {
            if (player->GetQuestStatus(QUEST_RITUAL_RUINATION) == QUEST_STATUS_INCOMPLETE)
                player->CastSpell(player, SPELL_DEMONIC_PORTAL_TO_DALARAN, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        }, 4s);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_warlock_portal_overload::HandleAfterHit);
    }
};
} // namespace RitualRuination

// ---------------------------------------------------------------------------------------------------------------------
// The hall opening: The Heart of the Dreadscar's way in from the Circle (40731), the Felblood Altar (40821 Power
// Overwhelming) and Ritssyn's soul flame (40823 Rebuilding the Council) in the Dreadscar Rift (map 1107)
namespace DreadscarRiftOpening
{
enum Data
{
    QUEST_THE_HEART_OF_THE_DREADSCAR    = 40731,
    QUEST_POWER_OVERWHELMING            = 40821,
    QUEST_REBUILDING_THE_COUNCIL        = 40823,

    NPC_CREDIT_RITSSYN_RESURRECTED      = 101969,

    MAP_DREADSCAR_RIFT_SCENARIO         = 1522
};

// The Finale's front entrance (WorldSafeLocs 5333), where 998 starts
Position const HeartOfTheDreadscarEntrance = { 3088.24f, 832.72f, 244.97f, 1.37f };

// Ritual Gateway (fork template 1220015, seen with 40731 in the log): the Circle's fel gateway, the way into 998
// (video 44:10). The S2 ritual's own gateway (1220004) is a 90 s summon and stays a plain object
struct go_dreadscar_heart_gateway : public GameObjectAI
{
    go_dreadscar_heart_gateway(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetClass() == CLASS_WARLOCK && player->GetQuestStatus(QUEST_THE_HEART_OF_THE_DREADSCAR) == QUEST_STATUS_INCOMPLETE)
            player->TeleportTo(MAP_DREADSCAR_RIFT_SCENARIO, HeartOfTheDreadscarEntrance.GetPositionX(), HeartOfTheDreadscarEntrance.GetPositionY(),
                HeartOfTheDreadscarEntrance.GetPositionZ(), HeartOfTheDreadscarEntrance.GetOrientation());
        return true;
    }
};

// Felblood Altar (fork template 1107001, an artifact forge): Power Overwhelming asks for the first trait, so while the
// quest is open and nothing is bought the altar tops the Scepter up to the first trait's cost (the DK Soul Forge pattern)
struct go_felblood_altar : public GameObjectAI
{
    go_felblood_altar(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_POWER_OVERWHELMING) != QUEST_STATUS_INCOMPLETE)
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

// Soul flame on its pedestal (fork template 1107002) in the bone cave (video 48:48): touching it raises Ritssyn, who is
// seen by entry once Rebuilding the Council is complete
struct go_ritssyn_soul_flame : public GameObjectAI
{
    go_ritssyn_soul_flame(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_REBUILDING_THE_COUNCIL) == QUEST_STATUS_INCOMPLETE)
        {
            player->KilledMonsterCredit(NPC_CREDIT_RITSSYN_RESURRECTED, me->GetGUID());
            player->UpdateObjectVisibility();
        }
        return true;
    }
};
} // namespace DreadscarRiftOpening

void AddSC_orderhall_warlock()
{
    {
        using namespace DreadscarRiftOpening;
        RegisterGameObjectAI(go_dreadscar_heart_gateway);
        RegisterGameObjectAI(go_felblood_altar);
        RegisterGameObjectAI(go_ritssyn_soul_flame);
    }
    {
        using namespace RitualRuination;
        RegisterCreatureAI(npc_fel_bat_ritual_ruination);
        RegisterCreatureAI(npc_guldan_ritual_ruination);
        RegisterCreatureAI(npc_allaris_ritual_ruination);
        RegisterSpellScript(spell_warlock_take_the_scepter);
        RegisterGameObjectAI(go_ritual_ruination_portal);
        RegisterSpellScript(spell_warlock_dimensional_rift_ritual);
        RegisterSpellScript(spell_warlock_portal_overload);
    }
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
