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
// No sniffed spawns exist for this map: every place is from its doodads (the prison's cages, the fel pits, the finale
// gateway) on the navigation mesh, staged after Wowhead's Destruction acquisition video (3:29-13:10).

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
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
#include "TaskScheduler.h"

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
    NPC_XELTOTH                         = 98834,
    NPC_JAGGANOTH                       = 98831,

    ITEM_TOME_OF_BLIGHTED_IMPLEMENTS    = 132512,

    SPELL_ENSLAVE_BESHTAL               = 199336,
    SPELL_EXIT_TELEPORT                 = 196340, // to the Circle of Wills (spell_target_position); credits 101150

    WORLD_SAFE_LOC_SCENARIO_GRAVEYARD   = 5342,

    // Ritssyn, prisoner
    SAY_RITSSYN_PRISONERS               = 0,  // 100641

    // Calydus
    SAY_CALYDUS_DONT_HURT               = 0,  // 100800
    SAY_CALYDUS_BESHTAL_GONE            = 1,  // 100801
    SAY_CALYDUS_SERVANT                 = 2,  // 100802
    SAY_CALYDUS_DISTRACT                = 3,  // 100803
    SAY_CALYDUS_FRIEND                  = 4,  // 100909
    SAY_CALYDUS_WEAPONS                 = 5,  // 100910
    SAY_CALYDUS_ARCHIVES                = 6,  // 100913
    SAY_CALYDUS_XELTOTH                 = 7,  // 101229
    SAY_CALYDUS_TOME                    = 8,  // 103457
    SAY_CALYDUS_TAKE_IT                 = 9,  // 103458
    SAY_CALYDUS_HELP_FRIEND             = 10, // 101259

    SAY_JAGGANOTH_NONE_CAN_ESCAPE       = 1,  // 101418

    DATA_SEND_EVENT                     = 1,

    POINT_CALYDUS_BARRIER               = 1,
    POINT_CALYDUS_ARCHIVE               = 2,
    POINT_CALYDUS_EXIT                  = 3,
    POINT_CAPTIVE_FREE                  = 4
};

Position const CalydusAppears  = { 2975.00f, 1122.00f, 206.68f, 1.57f };
Position const CalydusBarrier  = { 3060.15f, 1085.20f, 232.54f, 3.50f };
Position const CalydusArchive  = { 3055.60f,  929.87f, 251.43f, 0.20f };
Position const CalydusExit     = { 3087.87f,  853.33f, 244.36f, 4.70f };
Position const CaptivesGather  = { 3040.00f, 1088.00f, 224.81f, 0.00f };

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

            // Far From Home: Ritssyn speaks from his cell once the warlock stands in the prison
            if (IsOnStep(instance, STEP_FAR_FROM_HOME))
            {
                ObjectGuid playerGuid = player->GetGUID();
                _scheduler.Schedule(4s, [this, playerGuid](TaskContext /*context*/)
                {
                    Player* player = ObjectAccessor::GetPlayer(instance, playerGuid);
                    if (!player || !IsOnStep(instance, STEP_FAR_FROM_HOME))
                        return;
                    if (Creature* ritssyn = player->FindNearestCreature(NPC_RITSSYN_PRISONER, 60.0f))
                        ritssyn->AI()->Talk(SAY_RITSSYN_PRISONERS, player);
                    SendScenarioEvent(EVENT_LISTEN_TO_RITSSYN);
                });
            }
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (type == DATA_SEND_EVENT)
                SendScenarioEvent(data);
        }

        void Update(uint32 diff) override
        {
            _scheduler.Update(diff);
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
        TaskScheduler _scheduler;
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

// 98771 - Ritssyn Flamescowl, Jagganoth's prisoner: helps against Beshtal once the jailer is enslaved
struct npc_ritssyn_flamescowl_prisoner : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        if (Creature* beshtal = ObjectAccessor::GetCreature(*me, guid))
            AttackStart(beshtal);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }
};

// 98836 - Beshtal, Jagganoth's Jailer. 199336 Enslave Beshtal (a click on him) binds him for a moment, which opens the
// cells (step 1945, event 47179); then he breaks free and is fought with Ritssyn's help (event 47333 on his death).
// Calydus steps out of hiding once he falls.
struct npc_beshtal : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void Reset() override
    {
        _enslaved = false;
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
        _scheduler.Schedule(4s, [this, playerGuid](TaskContext /*context*/)
        {
            Player* player = ObjectAccessor::GetPlayer(*me, playerGuid);
            if (!player)
                return;
            me->SetFaction(FACTION_MONSTER);
            me->SetImmuneToPC(false);
            AttackStart(player);
            if (Creature* ritssyn = me->FindNearestCreature(NPC_RITSSYN_PRISONER, 60.0f))
                ritssyn->AI()->SetGUID(me->GetGUID(), 0);
        });
    }

    void JustDied(Unit* /*killer*/) override
    {
        SendEvent(me, EVENT_ESCAPE_THE_JAILER);
        me->SummonCreature(NPC_CALYDUS_PRISON, CalydusAppears, TEMPSUMMON_MANUAL_DESPAWN);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
        UpdateVictim();
    }

private:
    TaskScheduler _scheduler;
    bool _enslaved = false;
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

// 101050 - Calydus, in Jagganoth's prison: the unlikely ally. He offers his help over Beshtal's body, waits past the fel
// barrier, leads to the archive and its Tome, then to the way out
struct npc_calydus_prison : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void IsSummonedBy(WorldObject* /*summoner*/) override
    {
        Player* player = me->SelectNearestPlayer(80.0f);
        ObjectGuid playerGuid = player ? player->GetGUID() : ObjectGuid::Empty;
        _scheduler.Schedule(1s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_DONT_HURT, playerGuid); })
            .Schedule(5s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_BESHTAL_GONE, playerGuid); })
            .Schedule(10s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_SERVANT, playerGuid); })
            .Schedule(16s, [this, playerGuid](TaskContext /*context*/)
            {
                Say(SAY_CALYDUS_DISTRACT, playerGuid);
                me->GetMotionMaster()->MovePoint(POINT_CALYDUS_BARRIER, CalydusBarrier);
            });
    }

    // Jailbreak: the warlock reached him past the barrier (the areatrigger there)
    void SetGUID(ObjectGuid const& playerGuid, int32 id) override
    {
        if (id == POINT_CALYDUS_BARRIER)
        {
            _scheduler.Schedule(1s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_FRIEND, playerGuid); })
                .Schedule(7s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_WEAPONS, playerGuid); })
                .Schedule(13s, [this, playerGuid](TaskContext /*context*/)
                {
                    Say(SAY_CALYDUS_ARCHIVES, playerGuid);
                    me->GetMotionMaster()->MovePoint(POINT_CALYDUS_ARCHIVE, CalydusArchive);
                })
                .Schedule(25s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_XELTOTH, playerGuid); });
        }
        else if (id == POINT_CALYDUS_ARCHIVE)
        {
            // Xel'toth is dead and the Tome is the warlock's
            _scheduler.Schedule(1s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_TOME, playerGuid); })
                .Schedule(6s, [this, playerGuid](TaskContext /*context*/) { Say(SAY_CALYDUS_TAKE_IT, playerGuid); })
                .Schedule(14s, [this, playerGuid](TaskContext /*context*/)
                {
                    if (Creature* jagganoth = me->FindNearestCreature(NPC_JAGGANOTH, 250.0f))
                        jagganoth->AI()->Talk(SAY_JAGGANOTH_NONE_CAN_ESCAPE);
                })
                .Schedule(19s, [this, playerGuid](TaskContext /*context*/)
                {
                    Say(SAY_CALYDUS_HELP_FRIEND, playerGuid);
                    me->GetMotionMaster()->MovePoint(POINT_CALYDUS_EXIT, CalydusExit);
                });
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    void Say(uint8 group, ObjectGuid playerGuid)
    {
        Talk(group, ObjectAccessor::GetPlayer(*me, playerGuid));
    }

    TaskScheduler _scheduler;
};

// 99147 Jubeka, 99145 Zinnin, 99146 Shinfel: the shattered council, chained on the prison's upper level. A click frees
// each (step 1949), and they go to wait by the way out
struct npc_dreadscar_scenario_captive : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    // The core strips UNIT_NPC_FLAG_GOSSIP from what the client sees when the creature has no gossip menu
    // (Player::CanSeeGossipOn), and the click never reaches OnGossipHello. A menu id set by script counts as a menu;
    // the hello is then answered here, with no window
    void JustAppeared() override
    {
        me->SetGossipMenuId(me->GetEntry());
    }

    bool OnGossipHello(Player* player) override
    {
        if (_freed || !IsOnStep(me->GetMap(), STEP_THE_SHATTERED_COUNCIL))
            return true;

        _freed = true;
        uint32 eventId = EVENT_JUBEKA_FREED;
        if (me->GetEntry() == NPC_ZINNIN_CAPTIVE)
            eventId = EVENT_ZINNIN_FREED;
        else if (me->GetEntry() == NPC_SHINFEL_CAPTIVE)
            eventId = EVENT_SHINFEL_FREED;
        SendEvent(me, eventId);

        me->ReplaceAllNpcFlags(UNIT_NPC_FLAG_NONE);
        me->SetFacingToObject(player);
        me->HandleEmoteCommand(EMOTE_ONESHOT_CHEER);
        me->m_Events.AddEventAtOffset([this]
        {
            me->SetWalk(false);
            me->GetMotionMaster()->MovePoint(POINT_CAPTIVE_FREE, CaptivesGather);
        }, 2s);
        return true;
    }

private:
    bool _freed = false;
};

// 98834 - Xel'toth, the archive's keeper: his death gives the warlock the Tome of Blighted Implements (step 1950 counts
// the item) and Calydus speaks
struct npc_xeltoth : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void JustAppeared() override
    {
        me->SetFaction(FACTION_MONSTER);
    }

    void JustDied(Unit* killer) override
    {
        Player* player = killer ? killer->GetCharmerOrOwnerPlayerOrPlayerItself() : nullptr;
        if (!player)
            player = me->SelectNearestPlayer(80.0f);
        if (!player)
            return;

        if (!player->HasItemCount(ITEM_TOME_OF_BLIGHTED_IMPLEMENTS))
            player->AddItem(ITEM_TOME_OF_BLIGHTED_IMPLEMENTS, 1);

        if (Creature* calydus = me->FindNearestCreature(NPC_CALYDUS_PRISON, 150.0f))
            calydus->AI()->SetGUID(player->GetGUID(), POINT_CALYDUS_ARCHIVE);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }
};

// Past the fel barrier, where Calydus waits: Jailbreak (step 1953)
struct at_dreadscar_scenario_jailbreak : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->IsGameMaster() || !IsOnStep(at->GetMap(), STEP_JAILBREAK))
            return;

        SendEvent(at, EVENT_MEET_WITH_CALYDUS);
        if (Creature* calydus = player->FindNearestCreature(NPC_CALYDUS_PRISON, 100.0f))
            calydus->AI()->SetGUID(player->GetGUID(), POINT_CALYDUS_BARRIER);
    }
};

// The way out through the gateway: To Fight Another Day (step 1952). The step's reward is The New Blood itself
// (Scenario::CompleteStep rewards ScenarioStep.RewardQuestID); then Exit Teleport takes the warlock to the Circle
struct at_dreadscar_scenario_way_out : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->IsGameMaster() || !IsOnStep(at->GetMap(), STEP_TO_FIGHT_ANOTHER_DAY))
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
    RegisterCreatureAI(npc_xeltoth);
    RegisterAreaTriggerAI(at_dreadscar_scenario_jailbreak);
    RegisterAreaTriggerAI(at_dreadscar_scenario_way_out);
}
