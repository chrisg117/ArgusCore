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

#ifndef DEF_TIRISFAL_ARTIFACT_SCENARIOS_H_
#define DEF_TIRISFAL_ARTIFACT_SCENARIOS_H_

#include "Define.h"
#include <memory>

class InstanceMap;
class Player;
class Unit;

#define DataHeader "TirisfalArtifactScenarios"
#define TirisfalArtifactScenariosScriptName "instance_tirisfal_artifact_scenarios"

// The map is a copy of western Tirisfal Glades with the tomb of Tyr under its lake. Three classes get an
// artifact on it, each in a scenario of its own: the shadow priest ("Blade in Twilight"), the arms warrior
// and the holy paladin. Only the priest's is scripted.
enum TirisfalArtifactScenariosMisc
{
    MAP_TIRISFAL_ARTIFACT_SCENARIOS             = 1539,

    // Scenario.db2
    SCENARIO_BLADE_IN_TWILIGHT                  = 991,

    QUEST_BLADE_IN_TWILIGHT                     = 40710,

    // WorldSafeLocs.db2
    WORLD_SAFE_LOC_TIRISFAL_SCENARIO_GRAVEYARD  = 5282
};

// What scripts tell the instance (InstanceScript::SetData), which passes it on to its scenario
enum TirisfalArtifactScenariosData
{
    // "Blade in Twilight"
    DATA_BOSS_REVEAL_SCENE_OVER                 = 1, // the cutscene before the prison chamber is over
    DATA_FARTHING_FAILING                       = 2, // the Twilight Deacon is all but dead
    DATA_ZAKAJZ_DRAINED                         = 3  // a player's "Dark Drain" has run its time on Zakajz
};

// What one of the map's scenarios does in an instance of it. The instance script makes the one that the
// first player to enter is on a quest for, and passes on to it what happens in the instance.
class TirisfalArtifactScenario
{
public:
    explicit TirisfalArtifactScenario(InstanceMap* map) : instance(map) { }
    virtual ~TirisfalArtifactScenario() = default;

    virtual void OnPlayerEnter(Player* /*player*/) { }
    virtual void OnUnitDeath(Unit* /*unit*/) { }
    virtual void SetData(uint32 /*type*/, uint32 /*data*/) { }
    virtual void Update(uint32 /*diff*/) { }

protected:
    void SendScenarioEvent(uint32 gameEventId) const;

    InstanceMap* const instance;
};

std::unique_ptr<TirisfalArtifactScenario> CreateBladeInTwilightScenario(InstanceMap* map);

#endif
