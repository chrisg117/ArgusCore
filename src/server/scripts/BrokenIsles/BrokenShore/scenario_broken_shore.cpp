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
#include "GameEventSender.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "broken_shore.h"

ObjectData const creatureData[] =
{
    { NPC_KING_VARIAN_WRYNN, DATA_KING_VARIAN_WRYNN },
    { 0,                     0                      }  // END
};

// How close a player has to come to King Varian Wrynn to have found him
static constexpr float VarianFoundDistance = 30.0f;

// "The Battle for Broken Shore". The scenario itself is attached to the map by the `scenarios` table
// and advances when its criteria receive the game events listed in broken_shore.h.
// The stages are being filled in one at a time; those that are still stubs say what they need.
class instance_broken_shore_scenario : public InstanceMapScript
{
public:
    instance_broken_shore_scenario() : InstanceMapScript(BrokenShoreScriptName, MAP_BROKEN_SHORE_SCENARIO) { }

    struct instance_broken_shore_scenario_InstanceMapScript : public InstanceScript
    {
        instance_broken_shore_scenario_InstanceMapScript(InstanceMap* map) : InstanceScript(map), _currentStep(nullptr), _checkTimer(0)
        {
            SetHeaders(DataHeader);
            LoadObjectData(creatureData, nullptr);

            // Where players go when they release after dying; without one the core falls back to the
            // default graveyard in Westfall.
            // TODO: the Horde's locations, and those of the stages after "Find Varian".
            SetEntranceLocation(WORLD_SAFE_LOC_ALLIANCE_BEACH);
        }

        // The core has no script hook for scenario step changes, so the current step is polled.
        void Update(uint32 diff) override
        {
            InstanceScenario const* scenario = instance->GetInstanceScenario();
            if (!scenario)
                return;

            ScenarioStepEntry const* step = scenario->GetStep();
            if (step != _currentStep)
            {
                if (_currentStep)
                    EndStage(BrokenShoreStages(_currentStep->OrderIndex));

                _currentStep = step;
                if (step)
                    StartStage(BrokenShoreStages(step->OrderIndex));
                else if (scenario->IsComplete())
                    CompleteScenario();
            }

            if (!step)
                return;

            switch (step->OrderIndex)
            {
                case STAGE_THE_BROKEN_SHORE:
                    UpdateTheBrokenShore(diff);
                    break;
                case STAGE_FIND_VARIAN:
                    UpdateFindVarian(diff);
                    break;
                default:
                    break;
            }
        }

        void OnUnitDeath(Unit* unit) override
        {
            Creature* creature = unit->ToCreature();
            if (!creature)
                return;

            switch (creature->GetEntry())
            {
                case NPC_FELSTALKER_DREADHOUND:
                case NPC_FELGUARD_LEGIONNAIRE_1:
                case NPC_FELGUARD_LEGIONNAIRE_2:
                case NPC_FELGUARD_LEGIONNAIRE_3:
                    SendScenarioEvent(GAME_EVENT_BEACH_DEMON_SLAIN);
                    break;
                case NPC_FEL_LORD_KURDUZ:
                case NPC_FEL_LORD_RAKKAN:
                case NPC_FEL_LORD_ZARDAK:
                    SendScenarioEvent(GAME_EVENT_FEL_LORD_SLAIN);
                    break;
                // TODO: a spire is held by several crystals and should fall with the last of them
                case NPC_ANCHORING_CRYSTAL:
                    SendScenarioEvent(GAME_EVENT_SPIRE_OF_WOE_DESTROYED);
                    break;
                case NPC_DREAD_COMMANDER_ARGANOTH:
                    SendScenarioEvent(GAME_EVENT_ARGANOTH_SLAIN);
                    break;
                case NPC_SHIELDED_ANCHOR:
                    SendScenarioEvent(GAME_EVENT_ANCHOR_SHATTERED);
                    break;
                default:
                    break;
            }
        }

        // The core only passes a game event on to the scenario when a player is its source,
        // and never for a player in GM mode.
        void SendScenarioEvent(uint32 gameEventId)
        {
            for (MapReference const& ref : instance->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || player->IsGameMaster())
                    continue;

                GameEvents::Trigger(gameEventId, player, nullptr);
                return;
            }
        }

        // The stages that wait for players to get somewhere look for them once a second
        bool IsTimeToCheckPlayers(uint32 diff)
        {
            if (_checkTimer > diff)
            {
                _checkTimer -= diff;
                return false;
            }

            _checkTimer = 1 * IN_MILLISECONDS;
            return true;
        }

        void StartStage(BrokenShoreStages stage)
        {
            TC_LOG_DEBUG("scripts", "Broken Shore scenario: stage {} started (instance {})", uint32(stage), instance->GetInstanceId());

            switch (stage)
            {
                case STAGE_THE_BROKEN_SHORE:
                    StartTheBrokenShore();
                    break;
                case STAGE_STORM_THE_BEACH:
                    StartStormTheBeach();
                    break;
                case STAGE_DEFEAT_THE_COMMANDER:
                    StartDefeatTheCommander();
                    break;
                case STAGE_FIND_VARIAN:
                    StartFindVarian();
                    break;
                case STAGE_DESTROY_THE_PORTAL:
                    StartDestroyThePortal();
                    break;
                case STAGE_RAZE_THE_BLACK_CITY:
                    StartRazeTheBlackCity();
                    break;
                case STAGE_THE_HIGHLORD:
                    StartTheHighlord();
                    break;
                case STAGE_KROSUS:
                    StartKrosus();
                    break;
                case STAGE_STOP_GULDAN:
                    StartStopGuldan();
                    break;
                default:
                    break;
            }
        }

        void EndStage(BrokenShoreStages stage)
        {
            switch (stage)
            {
                case STAGE_STORM_THE_BEACH:
                    instance->SpawnGroupDespawn(SPAWN_GROUP_STORM_THE_BEACH, true);
                    break;
                // His death ends the stage, so only respawning is stopped and his corpse stays
                case STAGE_DEFEAT_THE_COMMANDER:
                    instance->SetSpawnGroupInactive(SPAWN_GROUP_DEFEAT_THE_COMMANDER);
                    break;
                case STAGE_DESTROY_THE_PORTAL:
                    instance->SpawnGroupDespawn(SPAWN_GROUP_DESTROY_THE_PORTAL, true);
                    break;
                default:
                    break;
            }
        }

        // "Travel to the Broken Shore." Needs GAME_EVENT_ARRIVED_AT_BROKEN_SHORE.
        // TODO: the faction ship and its arrival; players currently land straight on the beach.
        void StartTheBrokenShore() { }

        // Until there is a ship to arrive on, a player being on the map counts as having arrived
        void UpdateTheBrokenShore(uint32 diff)
        {
            if (IsTimeToCheckPlayers(diff))
                SendScenarioEvent(GAME_EVENT_ARRIVED_AT_BROKEN_SHORE);
        }

        // "Destroy all demons and structures on the beach." Needs 33 GAME_EVENT_BEACH_DEMON_SLAIN,
        // 3 GAME_EVENT_FEL_LORD_SLAIN and 3 GAME_EVENT_SPIRE_OF_WOE_DESTROYED, sent from OnUnitDeath.
        // TODO: the Spires of Woe themselves, the allied landing force and the demons' abilities.
        // The beach demons respawn in place of the reinforcements that should keep arriving.
        void StartStormTheBeach()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_STORM_THE_BEACH, true);
        }

        // "Slay Dread Commander Arganoth." Needs GAME_EVENT_ARGANOTH_SLAIN, sent from OnUnitDeath.
        // TODO: his arrival, lines and abilities, and Fel Commander Azgalor for the Horde.
        void StartDefeatTheCommander()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_DEFEAT_THE_COMMANDER, true);
        }

        // "Locate King Varian Wrynn." Needs GAME_EVENT_VARIAN_FOUND, sent when a player reaches him.
        // He stays where he is afterwards, because the next stage opens with him.
        // TODO: Genn and Jaina leading the way over the hill, the cutscene at its crest, the bombardment
        // on the way down, the forces fighting at Varian's side and the Horde's "Find The Others".
        void StartFindVarian()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_FIND_VARIAN, true);

            // Players who release from here on go to the graveyard at Varian's camp
            SetEntranceLocation(WORLD_SAFE_LOC_ALLIANCE_PORTAL);
        }

        void UpdateFindVarian(uint32 diff)
        {
            if (!IsTimeToCheckPlayers(diff))
                return;

            Creature* varian = GetCreature(DATA_KING_VARIAN_WRYNN);
            if (!varian)
                return;

            for (MapReference const& ref : instance->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || player->IsGameMaster() || !player->IsAlive() || !player->IsWithinDist(varian, VarianFoundDistance))
                    continue;

                GameEvents::Trigger(GAME_EVENT_VARIAN_FOUND, player, nullptr);
                return;
            }
        }

        // "Destroy the demon portal to stop reinforcements." Needs 4 GAME_EVENT_ANCHOR_SHATTERED,
        // sent from OnUnitDeath.
        // TODO: the portal itself, the shield an anchor has until its guard dies, and Varian's forces.
        void StartDestroyThePortal()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_DESTROY_THE_PORTAL, true);
        }

        // "Assault the demon city." A progress bar: 300 points from GAME_EVENT_BLACK_CITY_1 to _4,
        // worth 1, 2, 5 and 10 points each.
        // TODO: identify what each of the four events is sent by, then the city's demons.
        void StartRazeTheBlackCity() { }

        // "Get to Tirion." Needs GAME_EVENT_TIRION_FOUND.
        // TODO: Tirion, Gul'dan and the trigger for reaching them.
        void StartTheHighlord() { }

        // "Kill Krosus." Needs GAME_EVENT_KROSUS_SLAIN.
        // TODO: Krosus.
        void StartKrosus() { }

        // "Stop Gul'dan from summoning the Legion." Needs GAME_EVENT_GULDAN_CONFRONTED.
        // TODO: Gul'dan's summoning event (the Horde holds the ridge instead).
        void StartStopGuldan() { }

        // TODO: the finale cinematic and the return to Stormwind.
        void CompleteScenario()
        {
            instance->DoOnPlayers([](Player* player)
            {
                player->KilledMonsterCredit(NPC_FINALE_KILL_CREDIT);
            });
        }

    private:
        ScenarioStepEntry const* _currentStep;
        uint32 _checkTimer;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_broken_shore_scenario_InstanceMapScript(map);
    }
};

// 108920 - Captain Angelica
struct npc_captain_angelica_broken_shore : public ScriptedAI
{
    npc_captain_angelica_broken_shore(Creature* creature) : ScriptedAI(creature) { }

    // Her gossip menu has a single option, shown while the quest is in the log
    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (player->GetQuestStatus(QUEST_THE_BATTLE_FOR_BROKEN_SHORE) != QUEST_STATUS_INCOMPLETE)
            return true;

        // TODO: the ship ride to the Broken Shore; for now players are placed on the beach.
        player->KilledMonsterCredit(NPC_CAPTAIN_ANGELICA);
        player->TeleportTo(sWorldSafeLocsStore.AssertEntry(WORLD_SAFE_LOC_ALLIANCE_BEACH));
        return true;
    }
};

void AddSC_scenario_broken_shore()
{
    new instance_broken_shore_scenario();
    RegisterCreatureAI(npc_captain_angelica_broken_shore);
}
