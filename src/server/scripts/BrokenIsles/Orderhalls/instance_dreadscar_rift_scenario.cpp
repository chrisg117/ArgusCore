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

// Map 1522, the warlock's Dreadscar Rift scenario map. It carries two scenarios: 962 "Into the Mouth of the Nether"
// (The New Blood, 40729: Jagganoth's prison and the escape) and 998 "The Heart of the Dreadscar" (40731, the hall's
// claiming). The `scenarios` table holds one per map and difficulty (962); the instance script swaps in 998 for a
// warlock on The Heart of the Dreadscar.
// No spawn data exists for this map: every place is from its doodads (the prison's cages, the fel pits, the finale
// gateway) on the navigation mesh, staged after Wowhead's Destruction acquisition video (3:25-13:10) as the watcher's
// timeline gives it row by row (warlock re-do, 2026-11-10).

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Conversation.h"
#include "Creature.h"
#include "DB2Stores.h"
#include "GameEventSender.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScenarioMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Random.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include <algorithm>

#define DataHeader "DreadscarRiftScenario"

namespace DreadscarRiftScenario
{
enum Data
{
    MAP_DREADSCAR_RIFT_SCENARIO         = 1522,

    QUEST_THE_NEW_BLOOD                 = 40729,
    QUEST_THE_HEART_OF_THE_DREADSCAR    = 40731,

    SCENARIO_INTO_THE_MOUTH_OF_THE_NETHER = 962,
    SCENARIO_THE_HEART_OF_THE_DREADSCAR = 998,

    STEP_FAR_FROM_HOME                  = 1944,
    STEP_THE_DREAD_WARDEN               = 1945,
    STEP_THE_SHATTERED_COUNCIL          = 1949,
    STEP_JAILBREAK                      = 1953,
    STEP_THE_PIT_LORDS_SECRETS          = 1950,
    STEP_TO_FIGHT_ANOTHER_DAY           = 1952,

    EVENT_LISTEN_TO_RITSSYN             = 47152,
    EVENT_BESHTAL_ENSLAVED              = 47179,
    EVENT_ESCAPE_THE_JAILER             = 47333,
    EVENT_JUBEKA_FREED                  = 47181,
    EVENT_ZINNIN_FREED                  = 47182,
    EVENT_SHINFEL_FREED                 = 47183,
    EVENT_MEET_WITH_CALYDUS             = 47234,
    EVENT_ESCAPE_FROM_JAGGANOTHS_LAIR   = 47404,

    NPC_RITSSYN_PRISONER                = 98771,
    NPC_BESHTAL                         = 98836,
    NPC_CALYDUS_PRISON                  = 101050,
    NPC_JUBEKA_CAPTIVE                  = 99147,
    NPC_ZINNIN_CAPTIVE                  = 99145,
    NPC_SHINFEL_CAPTIVE                 = 99146,
    NPC_DREADSCAR_SUNDERER              = 99397,
    NPC_DREADSCAR_TORTURER              = 98878,
    NPC_NELACH                          = 99516,
    NPC_XELTOTH                         = 98834,
    NPC_JAGGANOTH                       = 98831,

    GO_TOME_OF_BLIGHTED_IMPLEMENTS      = 1522002, // fork chest on the archive's central lectern (loot 132512)

    SUMMON_PROPERTIES_COUNCIL           = 3738,    // the data's follower summon (195044/195065/195067): ally, follows

    SPELL_ENSLAVE_BESHTAL               = 199336,
    SPELL_EXIT_TELEPORT                 = 196340, // to the Circle of Wills (spell_target_position); credits 101150
    SPELL_WOUNDED                       = 200536, // Jubeka, left behind on the escape
    SPELL_VISUAL_FEL_BOMBARDMENT        = 48723,  // 193507's impact; played, not cast (193507 knocks back 100 yd)

    WORLD_SAFE_LOC_SCENARIO_GRAVEYARD   = 5342,

    // Ritssyn 98771
    SAY_RITSSYN_NEW_BLOOD_WAKES         = 0,  // 100640
    SAY_RITSSYN_PRISONERS               = 1,  // 100641
    SAY_RITSSYN_SKIP_TORMENT            = 2,  // 100642
    SAY_RITSSYN_BARELY_FOCUS            = 3,  // 100673
    SAY_RITSSYN_CONTROL_THE_JAILER      = 4,  // 100674
    EMOTE_RITSSYN_SCREAM                = 5,  // 100680 (raid boss emote)
    SAY_RITSSYN_THAT_WAS_JUBEKA         = 6,  // 100681
    SAY_RITSSYN_FIND_THE_OTHERS         = 7,  // 100682
    SAY_RITSSYN_LEAVE_ME                = 8,  // 100683
    // Beshtal 98836
    SAY_BESHTAL_OBEYS                   = 0,  // 101101
    SAY_BESHTAL_FURIOUS                 = 1,  // 100678
    SAY_BESHTAL_DYING                   = 2,  // 122631
    // Calydus 101050
    SAY_CALYDUS_DONT_HURT               = 0,  // 100800
    SAY_CALYDUS_SERVANT                 = 1,  // 100802
    SAY_CALYDUS_DISTRACT                = 2,  // 100803
    SAY_CALYDUS_THIS_WAY                = 3,  // 100907
    SAY_CALYDUS_FRIEND                  = 4,  // 100909
    SAY_CALYDUS_ARCHIVES                = 5,  // 100913
    SAY_CALYDUS_TOME                    = 6,  // 103457
    SAY_CALYDUS_TAKE_IT                 = 7,  // 103458
    SAY_CALYDUS_FLEE                    = 8,  // 103429
    SAY_CALYDUS_WAY_OPEN                = 9,  // 103433
    // guards
    SAY_SUNDERER_ESCAPED                = 0,  // 100804
    SAY_TORTURER_SILENT_WORGEN          = 0,  // 100868 (Zinnin's)
    SAY_TORTURER_HER_MIND               = 1,  // 100866 (Shinfel's)
    // Jubeka 99147
    SAY_JUBEKA_HOUND_FOOD               = 0,  // 100871
    SAY_JUBEKA_WHERE_IS_RITSSYN         = 1,  // 100893
    SAY_JUBEKA_KEEP_MOVING              = 2,  // 100897
    SAY_JUBEKA_TRUST_IT                 = 3,  // 100912
    SAY_JUBEKA_BAD_FEELING              = 4,  // 100969
    SAY_JUBEKA_TOMES                    = 5,  // 103455
    SAY_JUBEKA_SHINFEL                  = 6,  // 103359
    SAY_JUBEKA_WARNS_ZINNIN             = 7,  // 103422
    SAY_JUBEKA_ZINNIN                   = 8,  // 103431
    SAY_JUBEKA_DISMAY                   = 9,  // 103440
    SAY_JUBEKA_CLOSE_THE_PORTAL         = 10, // 103364
    // Zinnin 99145
    SAY_ZINNIN_SILENCE_1                = 0,  // 100869
    SAY_ZINNIN_SILENCE_2                = 1,  // 100895
    // Shinfel 99146
    SAY_SHINFEL_WEAK_TORTURE            = 0,  // 100865
    SAY_SHINFEL_THANKS                  = 1,  // 100867
    SAY_SHINFEL_ORC_TOO_HURT            = 2,  // 100894
    SAY_SHINFEL_ENSLAVE_HIM             = 3,  // 100896
    SAY_SHINFEL_WRETCHED_THING          = 4,  // 100908
    SAY_SHINFEL_RULING                  = 5,  // 100911
    SAY_SHINFEL_POWER                   = 6,  // 103456
    SAY_SHINFEL_DISCOVERED              = 7,  // 103355
    SAY_SHINFEL_STRUGGLES               = 8,  // 103357
    // Nelach 99516
    SAY_NELACH_INTRUDERS                = 0,  // 103453
    SAY_NELACH_DEATH                    = 1,  // 103454
    // Jagganoth 98831 (map-wide yells: the video carries the later ones by talking head)
    SAY_JAGGANOTH_INSOLENT              = 0,  // 103354
    SAY_JAGGANOTH_SLIP_AWAY             = 1,  // 103356
    SAY_JAGGANOTH_COMMAND_MY_KIND       = 2,  // 103358
    SAY_JAGGANOTH_GNATS                 = 3,  // 103360
    SAY_JAGGANOTH_FEL_FIRE              = 4,  // 103361
    SAY_JAGGANOTH_TROPHY                = 5,  // 103363
    SAY_JAGGANOTH_ASH                   = 6,  // 103365
    SAY_JAGGANOTH_NOT_OVER              = 7,  // 103366
    // his talking-head lines on the way out (the video's portrait at 12:13 / 12:30 / 13:03): one-line conversations with
    // the client's own ConversationLine ids (2238 / 2240 / 2241) and a TalkingHead actor
    CONVERSATION_JAGGANOTH_TROPHY       = 15220001,
    CONVERSATION_JAGGANOTH_ASH          = 15220002,
    CONVERSATION_JAGGANOTH_NOT_OVER     = 15220003,

    DATA_SEND_EVENT                     = 1,
    DATA_GUARD_DIED                     = 2,
    DATA_CAPTIVE_FREED                  = 3,
    DATA_MET_CALYDUS                    = 4,
    DATA_NELACH_DEAD                    = 5,
    DATA_XELTOTH_DEAD                   = 6,

    ACTION_KNEEL                        = 1,
    ACTION_RELEASE                      = 2,
    ACTION_WAIT_PAST_THE_BARRIER        = 3,

    POINT_CALYDUS_RUNS_OFF              = 1,
    POINT_CALYDUS_ARCHIVE               = 2
};

// Map 1522: the round dais in the pit beside the landing cage, the ramp out of it, the three spike pillars on the upper
// level (the captives' spawns), the fel barrier, the archive and the exit gateway (7Lg_Legion_Gatewaysmall01)
Position const CalydusRampTop     = { 2996.00f, 1158.00f, 215.60f, 4.10f };
Position const CalydusRunsOff     = { 3018.13f, 1163.20f, 221.35f, 0.00f };
Position const CalydusBarrier     = { 3060.15f, 1085.20f, 232.54f, 3.50f };
Position const CalydusArchive     = { 3055.60f,  929.87f, 251.43f, 0.20f };
Position const JagganothArchive   = { 3083.07f,  917.33f, 250.76f, 3.30f };
Position const CalydusGatewayMouth = { 3086.50f,  840.00f, 245.00f, 1.57f };

static bool IsOnStep(Map const* map, uint32 stepId)
{
    InstanceMap const* instance = map->ToInstanceMap();
    InstanceScenario const* scenario = instance ? instance->GetInstanceScenario() : nullptr;
    return scenario && scenario->GetStep() && scenario->GetStep()->ID == stepId;
}

static void SendEvent(WorldObject* source, uint32 eventId)
{
    if (InstanceScript* instance = source->GetInstanceScript())
        instance->SetData(DATA_SEND_EVENT, eventId);
}

// A warlock on The New Blood or on The Heart of the Dreadscar belongs here; anyone else is sent back to the Circle
static bool BelongsInTheScenario(Player const* player)
{
    if (player->GetClass() != CLASS_WARLOCK)
        return false;
    QuestStatus const newBlood = player->GetQuestStatus(QUEST_THE_NEW_BLOOD);
    QuestStatus const heart = player->GetQuestStatus(QUEST_THE_HEART_OF_THE_DREADSCAR);
    return newBlood == QUEST_STATUS_INCOMPLETE || newBlood == QUEST_STATUS_COMPLETE || heart == QUEST_STATUS_INCOMPLETE;
}

// The staging of scenario 962 follows Wowhead's video (3:25-13:10), row by row as warlock-s1s2-timeline.md gives it:
// the instance watches the step and drives what spans several actors (the freed council, Calydus's stations, the
// archive and the escape); each actor's own reactions are in its AI below
class instance_dreadscar_rift_scenario : public InstanceMapScript
{
public:
    instance_dreadscar_rift_scenario() : InstanceMapScript("instance_dreadscar_rift_scenario", MAP_DREADSCAR_RIFT_SCENARIO) { }

    struct instance_dreadscar_rift_scenario_InstanceMapScript : public InstanceScript
    {
        instance_dreadscar_rift_scenario_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);
            SetEntranceLocation(WORLD_SAFE_LOC_SCENARIO_GRAVEYARD);
        }

        void OnPlayerEnter(Player* player) override
        {
            if (player->IsGameMaster())
                return;

            if (!BelongsInTheScenario(player))
            {
                // A teleport during the enter would strand the player on the loading screen
                player->m_Events.AddEventAtOffset([player]
                {
                    if (player->GetMapId() == MAP_DREADSCAR_RIFT_SCENARIO && !player->IsBeingTeleported() && !BelongsInTheScenario(player))
                        player->CastSpell(player, SPELL_EXIT_TELEPORT, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
                }, 2s);
                return;
            }

            // The Heart of the Dreadscar (S6) runs on this map too
            if (player->GetQuestStatus(QUEST_THE_HEART_OF_THE_DREADSCAR) == QUEST_STATUS_INCOMPLETE)
            {
                InstanceScenario* scenario = instance->GetInstanceScenario();
                if (!scenario || scenario->GetEntry()->ID != SCENARIO_THE_HEART_OF_THE_DREADSCAR)
                    instance->SetInstanceScenario(sScenarioMgr->CreateInstanceScenario(instance, SCENARIO_THE_HEART_OF_THE_DREADSCAR));
                return;
            }

            _player = player->GetGUID();
            // Far From Home: Ritssyn speaks from the next cell, out of view at first (3:29-4:03)
            if (IsOnStep(instance, STEP_FAR_FROM_HOME) && !_monologue)
            {
                _monologue = true;
                _scheduler.Schedule(4s, [this](TaskContext) { RitssynSays(SAY_RITSSYN_NEW_BLOOD_WAKES); })
                    .Schedule(15s, [this](TaskContext) { RitssynSays(SAY_RITSSYN_PRISONERS); })
                    .Schedule(25s, [this](TaskContext) { RitssynSays(SAY_RITSSYN_SKIP_TORMENT); })
                    .Schedule(34s, [this](TaskContext) { RitssynSays(SAY_RITSSYN_BARELY_FOCUS); })
                    .Schedule(38s, [this](TaskContext) { RitssynSays(SAY_RITSSYN_CONTROL_THE_JAILER); })
                    .Schedule(40s, [this](TaskContext)
                    {
                        if (IsOnStep(instance, STEP_FAR_FROM_HOME))
                            SendScenarioEvent(EVENT_LISTEN_TO_RITSSYN);
                    });
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            switch (type)
            {
                case DATA_SEND_EVENT:
                    SendScenarioEvent(data);
                    break;
                case DATA_MET_CALYDUS:
                    MetCalydus();
                    break;
                case DATA_NELACH_DEAD:
                    NelachDead();
                    break;
                case DATA_XELTOTH_DEAD:
                    _xeltothDead = true;
                    XeltothDead();
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            return type == DATA_XELTOTH_DEAD ? uint32(_xeltothDead) : 0;
        }

        void SetGuidData(uint32 type, ObjectGuid guid) override
        {
            if (type == DATA_GUARD_DIED)
                GuardDied(guid);
            else if (type == DATA_CAPTIVE_FREED)
                CaptiveFreed(guid);
        }

        void Update(uint32 diff) override
        {
            _scheduler.Update(diff);

            InstanceScenario const* scenario = instance->GetInstanceScenario();
            uint32 const step = scenario && scenario->GetStep() ? scenario->GetStep()->ID : 0;
            if (step != _step)
            {
                _step = step;
                OnStepStart(step);
            }
        }

        void SendScenarioEvent(uint32 eventId)
        {
            for (MapReference const& ref : instance->GetPlayers())
                if (Player* player = ref.GetSource())
                    if (!player->IsGameMaster())
                    {
                        GameEvents::Trigger(eventId, player, nullptr);
                        return;
                    }
        }

    private:
        Player* GetWarlock() const { return ObjectAccessor::GetPlayer(instance, _player); }

        Creature* Find(uint32 entry, float range = 200.0f) const
        {
            Player* player = GetWarlock();
            return player ? player->FindNearestCreature(entry, range) : nullptr;
        }

        Creature* Follower(uint32 entry) const
        {
            auto itr = _followers.find(entry);
            return itr != _followers.end() ? instance->GetCreature(itr->second) : nullptr;
        }

        void RitssynSays(uint8 group)
        {
            if (Creature* ritssyn = Find(NPC_RITSSYN_PRISONER))
                ritssyn->AI()->Talk(group, GetWarlock());
        }

        void Say(uint32 entry, uint8 group)
        {
            Creature* speaker = Follower(entry);
            if (!speaker)
                speaker = Find(entry, 250.0f);
            if (speaker && speaker->IsAlive() && speaker->IsAIEnabled())
                speaker->AI()->Talk(group, GetWarlock());
        }

        void JagganothTalkingHead(uint32 conversationId)
        {
            if (Player* player = GetWarlock())
                Conversation::CreateConversation(conversationId, player, *player, player->GetGUID());
        }

        void JagganothSays(uint8 group)
        {
            if (Creature* jagganoth = instance->GetCreature(_jagganoth))
                jagganoth->AI()->Talk(group, GetWarlock());
        }

        Creature* SummonCalydus(Position const& pos)
        {
            if (Creature* old = instance->GetCreature(_calydus))
                old->DespawnOrUnsummon();
            Creature* calydus = instance->SummonCreature(NPC_CALYDUS_PRISON, pos);
            _calydus = calydus ? calydus->GetGUID() : ObjectGuid::Empty;
            return calydus;
        }

        void OnStepStart(uint32 step)
        {
            switch (step)
            {
                case STEP_THE_SHATTERED_COUNCIL:
                    // Calydus waits at the top of the ramp out of the pit; Ritssyn's lines come from his kneel (ACTION_KNEEL)
                    SummonCalydus(CalydusRampTop);
                    break;
                case STEP_JAILBREAK:
                    // the four stand and talk at the start of the passage (8:44-9:07), after Shinfel's thanks (her release
                    // starts this step, and her thanks come 7 s later); Calydus is already past the barrier
                    _scheduler.Schedule(9s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_WHERE_IS_RITSSYN); })
                        .Schedule(13s, [this](TaskContext) { Say(NPC_SHINFEL_CAPTIVE, SAY_SHINFEL_ORC_TOO_HURT); })
                        .Schedule(19s, [this](TaskContext) { Say(NPC_ZINNIN_CAPTIVE, SAY_ZINNIN_SILENCE_2); })
                        .Schedule(24s, [this](TaskContext) { Say(NPC_SHINFEL_CAPTIVE, SAY_SHINFEL_ENSLAVE_HIM); })
                        .Schedule(31s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_KEEP_MOVING); });
                    if (Creature* calydus = SummonCalydus(CalydusBarrier))
                        calydus->AI()->DoAction(ACTION_WAIT_PAST_THE_BARRIER);
                    break;
                case STEP_THE_PIT_LORDS_SECRETS:
                    // Calydus leads to the archive (10:05); Nelach waits at its entrance
                    _scheduler.Schedule(3s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_BAD_FEELING); });
                    if (Creature* calydus = instance->GetCreature(_calydus))
                    {
                        calydus->SetWalk(false);
                        calydus->GetMotionMaster()->MovePoint(POINT_CALYDUS_ARCHIVE, CalydusArchive);
                    }
                    break;
                case STEP_TO_FIGHT_ANOTHER_DAY:
                    TheEscape();
                    break;
                default:
                    break;
            }
        }

        // A captive is freed once no guard near her pillar is left alive (the video shows no click: she drops when the
        // player stands under her)
        void GuardDied(ObjectGuid guardGuid)
        {
            Creature* guard = instance->GetCreature(guardGuid);
            if (!guard)
                return;
            for (uint32 entry : { NPC_JUBEKA_CAPTIVE, NPC_ZINNIN_CAPTIVE, NPC_SHINFEL_CAPTIVE })
            {
                Creature* captive = guard->FindNearestCreature(entry, 25.0f);
                if (!captive || _followers.count(entry))
                    continue;
                std::list<Creature*> guards;
                captive->GetCreatureListWithEntryInGrid(guards, NPC_DREADSCAR_SUNDERER, 25.0f);
                captive->GetCreatureListWithEntryInGrid(guards, NPC_DREADSCAR_TORTURER, 25.0f);
                captive->GetCreatureListWithEntryInGrid(guards, guard->GetEntry(), 25.0f);
                bool const anyLeft = std::any_of(guards.begin(), guards.end(), [](Creature* g) { return g->IsAlive(); });
                if (!anyLeft)
                    captive->AI()->DoAction(ACTION_RELEASE);
            }
        }

        // The hanging captive has dropped: she joins the warlock as the data's follower summon (an ally that follows and
        // fights) and the pillar's copy goes
        void CaptiveFreed(ObjectGuid captiveGuid)
        {
            Creature* captive = instance->GetCreature(captiveGuid);
            Player* player = GetWarlock();
            if (!captive || !player || _followers.count(captive->GetEntry()))
                return;
            uint32 const entry = captive->GetEntry();
            SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(SUMMON_PROPERTIES_COUNCIL);
            if (TempSummon* follower = instance->SummonCreature(entry, captive->GetPosition(), properties, 0ms, player))
                _followers[entry] = follower->GetGUID();
            captive->DespawnOrUnsummon();

            if (entry == NPC_JUBEKA_CAPTIVE)
                _scheduler.Schedule(1s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_HOUND_FOOD); });
            else if (entry == NPC_SHINFEL_CAPTIVE)
                _scheduler.Schedule(5s, [this](TaskContext) { Say(NPC_SHINFEL_CAPTIVE, SAY_SHINFEL_THANKS); });
        }

        // Jailbreak: the group reaches Calydus past the barrier (9:32-10:01), then the step ends
        void MetCalydus()
        {
            if (_metCalydus)
                return;
            _metCalydus = true;
            _scheduler.Schedule(1s, [this](TaskContext) { Say(NPC_SHINFEL_CAPTIVE, SAY_SHINFEL_WRETCHED_THING); })
                .Schedule(3s, [this](TaskContext) { Say(NPC_CALYDUS_PRISON, SAY_CALYDUS_FRIEND); })
                .Schedule(17s, [this](TaskContext) { Say(NPC_SHINFEL_CAPTIVE, SAY_SHINFEL_RULING); })
                .Schedule(21s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_TRUST_IT); })
                .Schedule(27s, [this](TaskContext) { Say(NPC_CALYDUS_PRISON, SAY_CALYDUS_ARCHIVES); })
                .Schedule(30s, [this](TaskContext)
                {
                    if (IsOnStep(instance, STEP_JAILBREAK))
                        SendScenarioEvent(EVENT_MEET_WITH_CALYDUS);
                });
        }

        // Nelach is dead: the archive talks, and the Tome on the central lectern can be taken (10:46-11:08)
        void NelachDead()
        {
            if (Creature* calydus = instance->GetCreature(_calydus))
                calydus->NearTeleportTo(CalydusArchive);
            if (GameObject* tome = GetTome())
                tome->RemoveFlag(GO_FLAG_NOT_SELECTABLE);
            _scheduler.Schedule(6s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_TOMES); })
                .Schedule(14s, [this](TaskContext) { Say(NPC_SHINFEL_CAPTIVE, SAY_SHINFEL_POWER); })
                .Schedule(21s, [this](TaskContext) { Say(NPC_CALYDUS_PRISON, SAY_CALYDUS_TOME); })
                .Schedule(26s, [this](TaskContext) { Say(NPC_CALYDUS_PRISON, SAY_CALYDUS_TAKE_IT); });
        }

        GameObject* GetTome() const
        {
            Player* player = GetWarlock();
            return player ? player->FindNearestGameObject(GO_TOME_OF_BLIGHTED_IMPLEMENTS, 250.0f) : nullptr;
        }

        // To Fight Another Day (11:16-13:10): Jagganoth at the far side of the archive takes Shinfel; the rest run for the
        // exit gateway under fel bombardment; Zinnin falls, Jubeka stops wounded; Xel'toth bars the gateway
        void TheEscape()
        {
            if (Creature* jagganoth = instance->SummonCreature(NPC_JAGGANOTH, JagganothArchive))
            {
                _jagganoth = jagganoth->GetGUID();
                jagganoth->SetImmuneToAll(true);
            }
            _scheduler.Schedule(1s, [this](TaskContext) { JagganothSays(SAY_JAGGANOTH_INSOLENT); })
                .Schedule(4s, [this](TaskContext) { Say(NPC_SHINFEL_CAPTIVE, SAY_SHINFEL_DISCOVERED); })
                .Schedule(8s, [this](TaskContext) { JagganothSays(SAY_JAGGANOTH_SLIP_AWAY); })
                .Schedule(13s, [this](TaskContext)
                {
                    // he seizes Shinfel: she is pulled out of the group to him
                    Creature* shinfel = Follower(NPC_SHINFEL_CAPTIVE);
                    Creature* jagganoth = instance->GetCreature(_jagganoth);
                    if (!shinfel || !jagganoth)
                        return;
                    _followers.erase(NPC_SHINFEL_CAPTIVE);
                    shinfel->GetMotionMaster()->Clear();
                    shinfel->SetImmuneToAll(true);
                    shinfel->GetMotionMaster()->MoveJump(jagganoth->GetNearPosition(4.0f, 0.0f), 20.0f, 10.0f);
                    _shinfelTaken = shinfel->GetGUID();
                })
                .Schedule(15s, [this](TaskContext)
                {
                    if (Creature* shinfel = instance->GetCreature(_shinfelTaken))
                        shinfel->AI()->Talk(SAY_SHINFEL_STRUGGLES, GetWarlock());
                })
                .Schedule(19s, [this](TaskContext) { JagganothSays(SAY_JAGGANOTH_COMMAND_MY_KIND); })
                .Schedule(28s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_SHINFEL); })
                .Schedule(34s, [this](TaskContext) { JagganothSays(SAY_JAGGANOTH_GNATS); })
                .Schedule(40s, [this](TaskContext)
                {
                    JagganothSays(SAY_JAGGANOTH_FEL_FIRE);
                    if (Creature* shinfel = instance->GetCreature(_shinfelTaken))
                        shinfel->DespawnOrUnsummon();
                    Bombard();
                })
                .Schedule(45s, [this](TaskContext)
                {
                    // Calydus runs ahead to the gateway; he is next seen at its mouth
                    if (Creature* calydus = instance->GetCreature(_calydus))
                    {
                        calydus->AI()->Talk(SAY_CALYDUS_FLEE, GetWarlock());
                        calydus->DespawnOrUnsummon(4s);
                    }
                })
                .Schedule(48s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_WARNS_ZINNIN); })
                .Schedule(55s, [this](TaskContext) { JagganothTalkingHead(CONVERSATION_JAGGANOTH_TROPHY); })
                .Schedule(57s, [this](TaskContext)
                {
                    if (Creature* zinnin = Follower(NPC_ZINNIN_CAPTIVE))
                    {
                        _followers.erase(NPC_ZINNIN_CAPTIVE);
                        zinnin->KillSelf();
                    }
                })
                .Schedule(59s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_ZINNIN); })
                .Schedule(63s, [this](TaskContext) { Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_DISMAY); })
                .Schedule(66s, [this](TaskContext)
                {
                    Say(NPC_JUBEKA_CAPTIVE, SAY_JUBEKA_CLOSE_THE_PORTAL);
                    if (Creature* jubeka = Follower(NPC_JUBEKA_CAPTIVE))
                    {
                        _followers.erase(NPC_JUBEKA_CAPTIVE);
                        jubeka->GetMotionMaster()->Clear();
                        jubeka->GetMotionMaster()->MoveIdle();
                        jubeka->CastSpell(jubeka, SPELL_WOUNDED, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
                        jubeka->SetStandState(UNIT_STAND_STATE_KNEEL);
                    }
                })
                .Schedule(73s, [this](TaskContext) { JagganothTalkingHead(CONVERSATION_JAGGANOTH_ASH); });
        }

        // Fel impacts around the warlock on the way out, until Xel'toth falls (12:06)
        void Bombard()
        {
            _scheduler.Schedule(2500ms, [this](TaskContext context)
            {
                if (_xeltothDead || !IsOnStep(instance, STEP_TO_FIGHT_ANOTHER_DAY))
                    return;
                if (Player* player = GetWarlock())
                {
                    Position target = player->GetNearPosition(frand(6.0f, 14.0f), frand(0.0f, float(2 * M_PI)));
                    player->UpdateAllowedPositionZ(target.m_positionX, target.m_positionY, target.m_positionZ);
                    Position source(target.GetPositionX(), target.GetPositionY(), target.GetPositionZ() + 40.0f);
                    player->SendPlayOrphanSpellVisual(source, target, SPELL_VISUAL_FEL_BOMBARDMENT, 1.0f, true);
                }
                context.Repeat(2500ms);
            });
        }

        // Xel'toth is dead before the exit gateway: Calydus at its mouth, Jagganoth's last word (12:57-13:03)
        void XeltothDead()
        {
            _scheduler.Schedule(2s, [this](TaskContext)
            {
                if (Creature* calydus = SummonCalydus(CalydusGatewayMouth))
                    calydus->AI()->Talk(SAY_CALYDUS_WAY_OPEN, GetWarlock());
            })
                .Schedule(7s, [this](TaskContext) { JagganothTalkingHead(CONVERSATION_JAGGANOTH_NOT_OVER); });
        }

        TaskScheduler _scheduler;
        ObjectGuid _player;
        ObjectGuid _calydus;
        ObjectGuid _jagganoth;
        ObjectGuid _shinfelTaken;
        std::unordered_map<uint32, ObjectGuid> _followers;
        uint32 _step = 0;
        bool _monologue = false;
        bool _metCalydus = false;
        bool _xeltothDead = false;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_dreadscar_rift_scenario_InstanceMapScript(map);
    }
};

// Scenario spawns that fight the warlock: the template's faction is a placeholder (35), so they turn hostile here
struct npc_dreadscar_scenario_hostile : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
    }
};

// 98771 - Ritssyn Flamescowl, Jagganoth's prisoner: helps against Beshtal from the dais's rim, then kneels there,
// wounded, and stays (he does not follow; 5:00-5:15)
struct npc_ritssyn_flamescowl_prisoner : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        if (Creature* beshtal = ObjectAccessor::GetCreature(*me, guid))
            AttackStart(beshtal);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_KNEEL)
            return;
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->GetMotionMaster()->Clear();
        me->SetStandState(UNIT_STAND_STATE_KNEEL);
        Player* player = me->SelectNearestPlayer(100.0f);
        ObjectGuid const p = player ? player->GetGUID() : ObjectGuid::Empty;
        _scheduler.Schedule(5s, [this, p](TaskContext) { Talk(EMOTE_RITSSYN_SCREAM, ObjectAccessor::GetPlayer(*me, p)); })
            .Schedule(6s, [this, p](TaskContext) { Talk(SAY_RITSSYN_THAT_WAS_JUBEKA, ObjectAccessor::GetPlayer(*me, p)); })
            .Schedule(10s, [this, p](TaskContext) { Talk(SAY_RITSSYN_FIND_THE_OTHERS, ObjectAccessor::GetPlayer(*me, p)); })
            .Schedule(20s, [this, p](TaskContext) { Talk(SAY_RITSSYN_LEAVE_ME, ObjectAccessor::GetPlayer(*me, p)); });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
        UpdateVictim();
    }

private:
    TaskScheduler _scheduler;
};

// 98836 - Beshtal, Jagganoth's Jailer. 199336 Enslave Beshtal (a click on him) binds him: he obeys, the warlock leaves
// the cage (step 1945, event 47179) and goes down to the dais with him; there the enslavement breaks and he is fought
// with Ritssyn's help (event 47333 on his death, 4:13-4:55)
struct npc_beshtal : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void Reset() override
    {
        _enslaved = false;
        _dyingLine = false;
        _scheduler.CancelAll();
    }

    void SetGUID(ObjectGuid const& playerGuid, int32 /*id*/) override
    {
        if (_enslaved || !IsOnStep(me->GetMap(), STEP_THE_DREAD_WARDEN))
            return;

        _enslaved = true;
        SendEvent(me, EVENT_BESHTAL_ENSLAVED);
        if (Player* player = ObjectAccessor::GetPlayer(*me, playerGuid))
            me->SetFacingToObject(player);
        _scheduler.Schedule(3s, [this, playerGuid](TaskContext) { Talk(SAY_BESHTAL_OBEYS, ObjectAccessor::GetPlayer(*me, playerGuid)); })
            .Schedule(23s, [this, playerGuid](TaskContext)
            {
                Player* player = ObjectAccessor::GetPlayer(*me, playerGuid);
                if (!player)
                    return;
                Talk(SAY_BESHTAL_FURIOUS, player);
                me->SetFaction(FACTION_MONSTER);
                me->SetImmuneToPC(false);
                AttackStart(player);
                if (Creature* ritssyn = me->FindNearestCreature(NPC_RITSSYN_PRISONER, 60.0f))
                    ritssyn->AI()->SetGUID(me->GetGUID(), 0);
            });
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (!_dyingLine && me->HealthBelowPctDamaged(15, damage))
        {
            _dyingLine = true;
            Talk(SAY_BESHTAL_DYING);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        SendEvent(me, EVENT_ESCAPE_THE_JAILER);
        if (Creature* ritssyn = me->FindNearestCreature(NPC_RITSSYN_PRISONER, 60.0f))
            ritssyn->AI()->DoAction(ACTION_KNEEL);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
        UpdateVictim();
    }

private:
    TaskScheduler _scheduler;
    bool _enslaved = false;
    bool _dyingLine = false;
};

// 199336 - Enslave Beshtal
class spell_warlock_enslave_beshtal : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        Creature* beshtal = GetHitCreature();
        Unit* caster = GetCaster();
        if (beshtal && caster && beshtal->IsAIEnabled())
            beshtal->AI()->SetGUID(caster->GetGUID(), 0);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_warlock_enslave_beshtal::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 101050 - Calydus, in Jagganoth's prison. Never seen walking between his stations: the instance places him at each.
// At the ramp top he begs, offers his help and runs off (5:26-5:52); past the barrier he calls the group over (9:27)
struct npc_calydus_prison : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void DoAction(int32 action) override
    {
        if (action == ACTION_WAIT_PAST_THE_BARRIER)
            _waitingPastTheBarrier = true;
    }

    void MoveInLineOfSight(Unit* who) override
    {
        Player* player = who->ToPlayer();
        if (!player || player->IsGameMaster())
            return;

        if (!_rampDone && IsOnStep(me->GetMap(), STEP_THE_SHATTERED_COUNCIL) && me->IsWithinDist(player, 15.0f))
        {
            _rampDone = true;
            ObjectGuid const p = player->GetGUID();
            me->SetFacingToObject(player);
            _scheduler.Schedule(0s, [this, p](TaskContext) { Talk(SAY_CALYDUS_DONT_HURT, ObjectAccessor::GetPlayer(*me, p)); })
                .Schedule(5s, [this, p](TaskContext) { Talk(SAY_CALYDUS_SERVANT, ObjectAccessor::GetPlayer(*me, p)); })
                .Schedule(20s, [this, p](TaskContext) { Talk(SAY_CALYDUS_DISTRACT, ObjectAccessor::GetPlayer(*me, p)); })
                .Schedule(26s, [this](TaskContext)
                {
                    me->SetWalk(false);
                    me->GetMotionMaster()->MovePoint(POINT_CALYDUS_RUNS_OFF, CalydusRunsOff);
                    me->DespawnOrUnsummon(4s);
                });
        }
        else if (_waitingPastTheBarrier && me->IsWithinDist(player, 35.0f))
        {
            _waitingPastTheBarrier = false;
            Talk(SAY_CALYDUS_THIS_WAY, player);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    bool _rampDone = false;
    bool _waitingPastTheBarrier = false;
};

// 99147 Jubeka, 99145 Zinnin, 99146 Shinfel: the shattered council, hanging chained on three spike pillars along the
// path (6:18, 7:29, 8:32). Each drops when her guards are dead and joins the warlock (the instance's CaptiveFreed)
struct npc_dreadscar_scenario_captive : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetDisableGravity(true);
        me->SetImmuneToAll(true);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        Player* player = who->ToPlayer();
        if (_spoke || !player || player->IsGameMaster() || !me->IsWithinDist(player, 30.0f))
            return;
        _spoke = true;
        if (me->GetEntry() == NPC_SHINFEL_CAPTIVE)
            Talk(SAY_SHINFEL_WEAK_TORTURE, player);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_RELEASE || _freed || !IsOnStep(me->GetMap(), STEP_THE_SHATTERED_COUNCIL))
            return;
        _freed = true;
        uint32 eventId = EVENT_JUBEKA_FREED;
        if (me->GetEntry() == NPC_ZINNIN_CAPTIVE)
            eventId = EVENT_ZINNIN_FREED;
        else if (me->GetEntry() == NPC_SHINFEL_CAPTIVE)
            eventId = EVENT_SHINFEL_FREED;
        SendEvent(me, eventId);
        me->SetDisableGravity(false);
        me->GetMotionMaster()->MoveFall();
        me->m_Events.AddEventAtOffset([this]
        {
            if (InstanceScript* instance = me->GetInstanceScript())
                instance->SetGuidData(DATA_CAPTIVE_FREED, me->GetGUID());
        }, 2s);
    }

private:
    bool _spoke = false;
    bool _freed = false;
};

// 99397 Dreadscar Sunderer, 98813 Bloodscent Felhound, 98878 Dreadscar Torturer: the captives' guards
struct npc_dreadscar_captive_guard : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        Player* player = who->ToPlayer();
        if (!_spoke && player && !player->IsGameMaster() && me->IsWithinDist(player, 30.0f))
        {
            _spoke = true;
            ObjectGuid const p = player->GetGUID();
            if (me->GetEntry() == NPC_DREADSCAR_SUNDERER)
                Talk(SAY_SUNDERER_ESCAPED, player);
            else if (me->GetEntry() == NPC_DREADSCAR_TORTURER)
            {
                if (Creature* zinnin = me->FindNearestCreature(NPC_ZINNIN_CAPTIVE, 25.0f))
                {
                    Talk(SAY_TORTURER_SILENT_WORGEN, player);
                    ObjectGuid const z = zinnin->GetGUID();
                    _scheduler.Schedule(2s, [this, z, p](TaskContext)
                    {
                        if (Creature* captive = ObjectAccessor::GetCreature(*me, z))
                            captive->AI()->Talk(SAY_ZINNIN_SILENCE_1, ObjectAccessor::GetPlayer(*me, p));
                    });
                }
                else
                    _scheduler.Schedule(4s, [this, p](TaskContext) { Talk(SAY_TORTURER_HER_MIND, ObjectAccessor::GetPlayer(*me, p)); });
            }
        }
        ScriptedAI::MoveInLineOfSight(who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (InstanceScript* instance = me->GetInstanceScript())
            instance->SetGuidData(DATA_GUARD_DIED, me->GetGUID());
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
        if (!UpdateVictim())
            return;
    }

private:
    TaskScheduler _scheduler;
    bool _spoke = false;
};

// 99516 - Nelach, the All-Seeing, Keeper of the Archives: the archive's fight (10:30-10:40)
struct npc_nelach_the_all_seeing : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
    }

    void JustEngagedWith(Unit* who) override
    {
        Talk(SAY_NELACH_INTRUDERS, who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(SAY_NELACH_DEATH);
        if (InstanceScript* instance = me->GetInstanceScript())
            instance->SetData(DATA_NELACH_DEAD, 1);
    }
};

// 98834 - Xel'toth, fought before the exit gateway at the end of the escape (12:35)
struct npc_xeltoth : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (InstanceScript* instance = me->GetInstanceScript())
            instance->SetData(DATA_XELTOTH_DEAD, 1);
    }
};

// Past the fel barrier, where Calydus waits: the group reaches him, talks, and Jailbreak ends (step 1953)
struct at_dreadscar_scenario_jailbreak : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->IsGameMaster() || !IsOnStep(at->GetMap(), STEP_JAILBREAK))
            return;
        if (InstanceScript* instance = at->GetInstanceScript())
            instance->SetData(DATA_MET_CALYDUS, 1);
    }
};

// The way out through the gateway, once Xel'toth is dead: To Fight Another Day (step 1952). The step's reward is The
// New Blood itself (Scenario::CompleteStep rewards ScenarioStep.RewardQuestID); then Exit Teleport takes the warlock to
// the Circle
struct at_dreadscar_scenario_way_out : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        InstanceScript* instance = at->GetInstanceScript();
        if (!player || player->IsGameMaster() || !instance || !IsOnStep(at->GetMap(), STEP_TO_FIGHT_ANOTHER_DAY)
            || !instance->GetData(DATA_XELTOTH_DEAD))
            return;
        SendEvent(at, EVENT_ESCAPE_FROM_JAGGANOTHS_LAIR);
        player->m_Events.AddEventAtOffset([player]
        {
            if (player->GetMapId() == MAP_DREADSCAR_RIFT_SCENARIO && !player->IsBeingTeleported())
                player->CastSpell(player, SPELL_EXIT_TELEPORT, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        }, 3s);
    }
};

// ---------------------------------------------------------------------------------------------------------------------
// The Heart of the Dreadscar (40731), scenario 998 on this map (video 44:10-47:40). Nothing in the data sends its events:
// Jagganoth's death sends 48186 and gives the heart (2058 is the data's AcquireItem); the altar in the Seat of the
// Overlord takes the heart (48182), then commands the armies (48187), and the Order Hall Transition (201205) takes the
// warlock to the hall and credits Dreadscar Rift claimed
enum HeartOfTheDreadscar
{
    STEP_BECOMING_THE_OVERLORD          = 2059,

    EVENT_JAGGANOTH_DEFEATED            = 48186,
    EVENT_HEART_PLACED                  = 48182,
    EVENT_ARMIES_COMMANDED              = 48187,

    ITEM_HEART_OF_THE_OVERLORD          = 132880,
    SPELL_ORDER_HALL_TRANSITION         = 201205  // TELEPORT_UNITS (spell_target_position: the hall by Calydus) + credit 101524
};

// 101892 - Jagganoth, Overlord of the Dreadscar, 998's boss (98831 is 962's, hidden from a warlock on 40731). The
// template's spell list is empty and his one named spell (199150 Smash) is a jump to a marker, so he fights in melee
// for now; his abilities are for the combat-log pass
struct boss_jagganoth_heart_of_the_dreadscar : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
    }

    void JustDied(Unit* killer) override
    {
        SendEvent(me, EVENT_JAGGANOTH_DEFEATED);
        // The bot cannot loot, and the heart is the scenario's item: it is given on the kill (the S3 Jergosh pattern)
        if (Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr)
            if (!player->HasItemCount(ITEM_HEART_OF_THE_OVERLORD, 1, true))
                player->AddItem(ITEM_HEART_OF_THE_OVERLORD, 1);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }
};

// The altar in the Seat of the Overlord (fork template): the first use places the heart, the second commands the armies
struct go_seat_of_the_overlord_altar : public GameObjectAI
{
    go_seat_of_the_overlord_altar(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (!IsOnStep(me->GetMap(), STEP_BECOMING_THE_OVERLORD))
            return true;

        if (!_heartPlaced)
        {
            if (!player->HasItemCount(ITEM_HEART_OF_THE_OVERLORD, 1))
                return true;
            player->DestroyItemCount(ITEM_HEART_OF_THE_OVERLORD, 1, true);
            _heartPlaced = true;
            SendEvent(me, EVENT_HEART_PLACED);
            return true;
        }

        if (_armiesCommanded)
            return true;
        _armiesCommanded = true;
        SendEvent(me, EVENT_ARMIES_COMMANDED);
        player->m_Events.AddEventAtOffset([player]
        {
            if (player->GetMapId() == MAP_DREADSCAR_RIFT_SCENARIO && !player->IsBeingTeleported())
                player->CastSpell(player, SPELL_ORDER_HALL_TRANSITION, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        }, 3s);
        return true;
    }

private:
    bool _heartPlaced = false;
    bool _armiesCommanded = false;
};
} // namespace DreadscarRiftScenario

void AddSC_instance_dreadscar_rift_scenario()
{
    using namespace DreadscarRiftScenario;
    new instance_dreadscar_rift_scenario();
    RegisterCreatureAI(boss_jagganoth_heart_of_the_dreadscar);
    RegisterGameObjectAI(go_seat_of_the_overlord_altar);
    RegisterCreatureAI(npc_dreadscar_scenario_hostile);
    RegisterCreatureAI(npc_ritssyn_flamescowl_prisoner);
    RegisterCreatureAI(npc_beshtal);
    RegisterSpellScript(spell_warlock_enslave_beshtal);
    RegisterCreatureAI(npc_calydus_prison);
    RegisterCreatureAI(npc_dreadscar_scenario_captive);
    RegisterCreatureAI(npc_dreadscar_captive_guard);
    RegisterCreatureAI(npc_nelach_the_all_seeing);
    RegisterCreatureAI(npc_xeltoth);
    RegisterAreaTriggerAI(at_dreadscar_scenario_jailbreak);
    RegisterAreaTriggerAI(at_dreadscar_scenario_way_out);
}
