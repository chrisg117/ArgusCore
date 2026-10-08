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

#ifndef DEF_BROKEN_SHORE_H_
#define DEF_BROKEN_SHORE_H_

#include "CreatureAIImpl.h"

#define DataHeader "BrokenShoreScenario"
#define BrokenShoreScriptName "instance_broken_shore_scenario"

enum BrokenShoreMisc
{
    MAP_BROKEN_SHORE_SCENARIO           = 1460,

    // Scenario.db2, both named "The Battle for Broken Shore"
    SCENARIO_BROKEN_SHORE_ALLIANCE      = 786,
    SCENARIO_BROKEN_SHORE_HORDE         = 1189,

    QUEST_THE_BATTLE_FOR_BROKEN_SHORE   = 42740
};

// ScenarioStep.OrderIndex. Both factions' scenarios have nine steps in this order;
// the Horde names differ for steps 3 ("Find The Others") and 8 ("Hold The Ridge").
enum BrokenShoreStages : uint8
{
    STAGE_THE_BROKEN_SHORE              = 0,
    STAGE_STORM_THE_BEACH               = 1,
    STAGE_DEFEAT_THE_COMMANDER          = 2,
    STAGE_FIND_VARIAN                   = 3,
    STAGE_DESTROY_THE_PORTAL            = 4,
    STAGE_RAZE_THE_BLACK_CITY           = 5,
    STAGE_THE_HIGHLORD                  = 6,
    STAGE_KROSUS                        = 7,
    STAGE_STOP_GULDAN                   = 8
};

// Game events counted by the Alliance scenario's criteria (CriteriaType::AnyoneTriggerGameEventScenario).
// The amount each step needs is noted where it is more than one.
enum BrokenShoreGameEvents
{
    GAME_EVENT_ARRIVED_AT_BROKEN_SHORE  = 44060,
    GAME_EVENT_BEACH_DEMON_SLAIN        = 44095, // 33
    GAME_EVENT_FEL_LORD_SLAIN           = 52643, // 3
    GAME_EVENT_SPIRE_OF_WOE_DESTROYED   = 44077, // 3
    GAME_EVENT_ARGANOTH_SLAIN           = 45131,
    GAME_EVENT_VARIAN_FOUND             = 45228,
    GAME_EVENT_ANCHOR_SHATTERED         = 45288, // 4
    // "Raze the Black City" is a progress bar ("Black City razed") that fills at 300 points.
    // Its four criteria have no description in the client; each event is worth the points noted.
    GAME_EVENT_BLACK_CITY_1             = 44384, // 1 point
    GAME_EVENT_BLACK_CITY_2             = 53062, // 2 points
    GAME_EVENT_BLACK_CITY_3             = 53063, // 5 points
    GAME_EVENT_BLACK_CITY_4             = 53064, // 10 points
    GAME_EVENT_TIRION_FOUND             = 50027,
    GAME_EVENT_KROSUS_SLAIN             = 44669,
    GAME_EVENT_GULDAN_CONFRONTED        = 44826
};

enum BrokenShoreCreatureIds
{
    // Storm the Beach (Alliance)
    NPC_FELSTALKER_DREADHOUND           = 90686,
    NPC_FEL_LORD_KURDUZ                 = 91588,
    NPC_ANCHORING_CRYSTAL               = 91704, // holds a Spire of Woe in place
    NPC_FEL_LORD_RAKKAN                 = 109586,
    NPC_FEL_LORD_ZARDAK                 = 109587,
    NPC_FELGUARD_LEGIONNAIRE_1          = 109591,
    NPC_FELGUARD_LEGIONNAIRE_2          = 109592,
    NPC_FELGUARD_LEGIONNAIRE_3          = 109604,

    // Defeat the Commander (Alliance)
    NPC_DREAD_COMMANDER_ARGANOTH        = 90705,

    // Find Varian (Alliance)
    NPC_KING_VARIAN_WRYNN               = 90713,

    NPC_FINALE_KILL_CREDIT              = 90918, // quest objective "Broken Shore assaulted"
    NPC_CAPTAIN_ANGELICA                = 108920 // Stormwind Harbor, quest objective "Ship taken to the Broken Shore"
};

// Creatures the instance script keeps track of (ObjectData)
enum BrokenShoreDataTypes
{
    DATA_KING_VARIAN_WRYNN              = 0
};

// spawn_group_template. Each stage's spawns are one group, spawned when the stage starts.
enum BrokenShoreSpawnGroups
{
    SPAWN_GROUP_STORM_THE_BEACH         = 14601,
    SPAWN_GROUP_DEFEAT_THE_COMMANDER    = 14602,
    SPAWN_GROUP_FIND_VARIAN             = 14603
};

// WorldSafeLocs.db2. The "Start" locations are relative to the faction's ship, not to the map.
enum BrokenShoreWorldSafeLocs
{
    WORLD_SAFE_LOC_ALLIANCE_START       = 4981,
    WORLD_SAFE_LOC_ALLIANCE_BEACH       = 5021,
    WORLD_SAFE_LOC_ALLIANCE_PORTAL      = 5024,
    WORLD_SAFE_LOC_ALLIANCE_CITY        = 5025,
    WORLD_SAFE_LOC_ALLIANCE_CREVASSE    = 5026,
    WORLD_SAFE_LOC_ALLIANCE_TOMB        = 5027,

    WORLD_SAFE_LOC_HORDE_START          = 5456,
    WORLD_SAFE_LOC_HORDE_BEACH          = 5682,
    WORLD_SAFE_LOC_HORDE_PORTAL         = 5683,
    WORLD_SAFE_LOC_HORDE_CITY           = 5684,
    WORLD_SAFE_LOC_HORDE_CREVASSE       = 5685,
    WORLD_SAFE_LOC_HORDE_TOMB           = 5686
};

template <class AI, class T>
inline AI* GetBrokenShoreAI(T* obj)
{
    return GetInstanceAI<AI>(obj, BrokenShoreScriptName);
}

#define RegisterBrokenShoreCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetBrokenShoreAI)

#endif
