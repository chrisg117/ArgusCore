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
#include "GameEventSender.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Player.h"
#include "ScenarioMgr.h"
#include "tirisfal_artifact_scenarios.h"

// The core only passes a game event on to the scenario when a player is its source, and never for a player
// in GM mode.
void TirisfalArtifactScenario::SendScenarioEvent(uint32 gameEventId) const
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

// The `scenarios` table can name one scenario for a map, and this map has three. So the table has none for
// it, and the instance gets the scenario that the first player to enter is on a quest for.
class instance_tirisfal_artifact_scenarios : public InstanceMapScript
{
public:
    instance_tirisfal_artifact_scenarios() : InstanceMapScript(TirisfalArtifactScenariosScriptName, MAP_TIRISFAL_ARTIFACT_SCENARIOS) { }

    struct instance_tirisfal_artifact_scenarios_InstanceMapScript : public InstanceScript
    {
        instance_tirisfal_artifact_scenarios_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);

            // Where players go when they release after dying; without one the core falls back to the
            // default graveyard in Westfall
            SetEntranceLocation(WORLD_SAFE_LOC_TIRISFAL_SCENARIO_GRAVEYARD);
        }

        void OnPlayerEnter(Player* player) override
        {
            if (!_scenario)
                StartScenarioOf(player);

            if (_scenario)
                _scenario->OnPlayerEnter(player);
        }

        void OnUnitDeath(Unit* unit) override
        {
            if (_scenario)
                _scenario->OnUnitDeath(unit);
        }

        void SetData(uint32 type, uint32 data) override
        {
            if (_scenario)
                _scenario->SetData(type, data);
        }

        void Update(uint32 diff) override
        {
            if (_scenario)
                _scenario->Update(diff);
        }

    private:
        void StartScenarioOf(Player const* player)
        {
            uint32 scenarioId = 0;
            if (player->GetQuestStatus(QUEST_BLADE_IN_TWILIGHT) == QUEST_STATUS_INCOMPLETE)
            {
                scenarioId = SCENARIO_BLADE_IN_TWILIGHT;
                _scenario = CreateBladeInTwilightScenario(instance);
            }

            if (!scenarioId)
                return;

            if (InstanceScenario* scenario = sScenarioMgr->CreateInstanceScenario(instance, scenarioId))
                instance->SetInstanceScenario(scenario);
        }

        std::unique_ptr<TirisfalArtifactScenario> _scenario;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_tirisfal_artifact_scenarios_InstanceMapScript(map);
    }
};

void AddSC_instance_tirisfal_artifact_scenarios()
{
    new instance_tirisfal_artifact_scenarios();
}
