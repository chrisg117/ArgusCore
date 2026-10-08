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
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Log.h"
#include "Map.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "broken_shore.h"

// Scaffold for "The Battle for Broken Shore". The scenario itself is attached to the map by the
// `scenarios` table and advances when its criteria receive the game events listed in broken_shore.h.
// Nothing is spawned yet: each stage below is a stub to be filled in with its spawns and scripting.
class instance_broken_shore_scenario : public InstanceMapScript
{
public:
    instance_broken_shore_scenario() : InstanceMapScript(BrokenShoreScriptName, MAP_BROKEN_SHORE_SCENARIO) { }

    struct instance_broken_shore_scenario_InstanceMapScript : public InstanceScript
    {
        instance_broken_shore_scenario_InstanceMapScript(InstanceMap* map) : InstanceScript(map), _currentStep(nullptr)
        {
            SetHeaders(DataHeader);
        }

        // The core has no script hook for scenario step changes, so the current step is polled.
        void Update(uint32 /*diff*/) override
        {
            InstanceScenario const* scenario = instance->GetInstanceScenario();
            if (!scenario)
                return;

            ScenarioStepEntry const* step = scenario->GetStep();
            if (step == _currentStep)
                return;

            _currentStep = step;
            if (step)
                StartStage(BrokenShoreStages(step->OrderIndex));
            else if (scenario->IsComplete())
                CompleteScenario();
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

        // "Travel to the Broken Shore." Needs GAME_EVENT_ARRIVED_AT_BROKEN_SHORE.
        // TODO: the faction ship and its arrival; players currently land straight on the beach.
        void StartTheBrokenShore() { }

        // "Destroy all demons and structures on the beach." Needs 33 GAME_EVENT_BEACH_DEMON_SLAIN,
        // 3 GAME_EVENT_FEL_LORD_SLAIN and 3 GAME_EVENT_SPIRE_OF_WOE_DESTROYED.
        // TODO: beach demons, Fel Lords, Spires of Woe and the allied landing force.
        void StartStormTheBeach() { }

        // "Slay Dread Commander Arganoth." Needs GAME_EVENT_ARGANOTH_SLAIN.
        // TODO: NPC_DREAD_COMMANDER_ARGANOTH (Fel Commander Azgalor for the Horde).
        void StartDefeatTheCommander() { }

        // "Locate King Varian Wrynn." Needs GAME_EVENT_VARIAN_FOUND.
        // TODO: the faction leaders and the trigger for reaching them.
        void StartFindVarian() { }

        // "Destroy the demon portal to stop reinforcements." Needs 4 GAME_EVENT_ANCHOR_SHATTERED.
        // TODO: the portal, its four shielded anchors and their guards.
        void StartDestroyThePortal() { }

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
