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
    // Its four criteria have no description in the client; each event is worth the points noted
    // and is sent for a demon slain in the city, the stronger the demon the more points.
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
    // The Alliance's leaders, who fight alongside players
    NPC_LADY_JAINA_PROUDMOORE           = 90714,
    NPC_GENN_GREYMANE                   = 90717,

    // The forces that land with them
    NPC_KIRIN_TOR_BATTLE_MAGE_1         = 91353,
    NPC_KIRIN_TOR_BATTLE_MAGE_2         = 97496,
    NPC_GILNEAN_ROYAL_GUARD             = 97486,
    NPC_GILNEAN_DRUID                   = 110627,
    NPC_DARNASSUS_SENTINEL              = 114466,

    // Storm the Beach (Alliance)
    NPC_FELSTALKER_DREADHOUND           = 90686,
    NPC_FEL_LORD_KURDUZ                 = 91588,
    NPC_ANCHORING_CRYSTAL               = 91704, // circles a Spire of Woe; three of them hold it in place
    NPC_SPIRE_OF_WOE                    = 97624, // at the top of a Spire of Woe, where its beam comes from
    NPC_SPIRE_OF_WOE_BEAM_TARGET        = 97629, // of the same name; on the ground, where a beam comes down
    NPC_FEL_LORD_RAKKAN                 = 109586,
    NPC_FEL_LORD_ZARDAK                 = 109587,
    NPC_FELGUARD_LEGIONNAIRE_1          = 109591,
    NPC_FELGUARD_LEGIONNAIRE_2          = 109592,
    NPC_FELGUARD_LEGIONNAIRE_3          = 109604,

    // Defeat the Commander (Alliance)
    NPC_DREAD_COMMANDER_ARGANOTH        = 90705,

    // Find Varian (Alliance)
    NPC_KING_VARIAN_WRYNN               = 90713,

    // Destroy the Portal (Alliance)
    NPC_SHIELDED_ANCHOR                 = 101667,

    // Raze the Black City (Alliance)
    NPC_INFERNAL_SIEGEBREAKER           = 91967,
    NPC_FELGUARD_INVADER                = 91970,
    NPC_LIVING_FELBLAZE                 = 94189,
    NPC_BURNING_SENTRY                  = 94190,
    NPC_BURNING_TERRORHOUND             = 94191,
    NPC_SOULBOUND_DESTRUCTOR            = 97510,
    NPC_MOTHER_VIRILA                   = 100621,
    NPC_MOARG_PAINBRINGER               = 102701,
    NPC_WRATHGUARD_DREADBLADE           = 102702,
    NPC_FEL_LORD_DUKAZ                  = 102703,
    NPC_FEL_LORD_ZARNOZ                 = 102704,
    NPC_FEL_LORD_RAKAZ                  = 102705,
    NPC_GRINNING_SHADOWSTALKER          = 102706,
    NPC_FELFIRE_IMP                     = 103896,
    NPC_FIERY_TRICKSTER                 = 103897,
    NPC_SHADOWFLAME_IMP                 = 103899,
    NPC_MALIFICUS                       = 110614,
    NPC_DARK_WORSHIPPER                 = 110616,
    NPC_SHADOWSWORN_HARBINGER           = 110617,

    // The Highlord (Alliance)
    NPC_HIGHLORD_TIRION_FORDRING        = 91951,

    // Krosus (Alliance)
    NPC_KROSUS                          = 90544,

    // Stop Gul'dan (Alliance)
    NPC_GULDAN                          = 94276,
    NPC_MOARG_SPINEBREAKER              = 105205,

    NPC_FINALE_KILL_CREDIT              = 90918, // quest objective "Broken Shore assaulted"
    NPC_CAPTAIN_ANGELICA                = 108920 // Stormwind Harbor, quest objective "Ship taken to the Broken Shore"
};

enum BrokenShoreGameObjectIds
{
    // Storm the Beach. Which of the templates of that name Blizzard spawns is not known
    GO_SPIRE_OF_WOE                     = 240194,

    // A custom template: the object Blizzard spawns for Jaina's bridge of ice is not known
    GO_ICE_BRIDGE                       = 1460001
};

enum BrokenShoreSpells
{
    // Players are given one stack of it by the instance script. On retail it is one of Genn Greymane's
    // abilities; when he casts it is not known.
    SPELL_FOR_THE_ALLIANCE              = 185265,

    // "Cosmetic - Green Cat Mark State (5.00)": a large green arrow that points down at a unit. A video of
    // the retail scenario shows such arrows over the Anchoring Crystals; which spell they are is not known.
    SPELL_GREEN_CAT_MARK_STATE          = 151205,

    // "Before We're Overrun: Fel Beam", named after a quest of Mardum, where Spires of Woe stand too. A
    // spire channels it at its target on the ground. A video of the retail scenario shows such beams; which
    // spell they are is not known.
    SPELL_FEL_BEAM                      = 192664,

    // The Alliance's forces
    SPELL_WRATH                         = 171773, // Gilnean Druid
    SPELL_FROSTFIRE_BOLT                = 199219  // Kirin Tor Battle-Mage
};

enum BrokenShoreSpellVisualKits
{
    // The burst of fel fire on the ground that the aura "Before We're Overrun: Spire of Woe - Area Trigger"
    // (192604) shows on its bearer
    SPELL_VISUAL_KIT_SPIRE_OF_WOE_FIRE  = 59267
};

// creature_text groups of the Alliance's leaders. Group 0 is the same for both.
enum BrokenShoreLeaderTexts
{
    SAY_LEADER_IN_A_FIGHT               = 0,

    SAY_JAINA_REINFORCEMENTS            = 1,
    SAY_JAINA_NOW_OR_NEVER              = 2,
    SAY_JAINA_CRYSTALS                  = 3,
    SAY_JAINA_IT_WORKED                 = 4,

    SAY_GENN_JUST_IN_TIME               = 1,
    SAY_GENN_CHARGE                     = 2,
    SAY_GENN_HOPE_YOU_ARE_RIGHT         = 3,
    SAY_GENN_ONE_DOWN                   = 4,
    SAY_GENN_ONE_MORE                   = 5
};

// Creatures the instance script keeps track of (ObjectData), and what it can be asked with GetData
enum BrokenShoreDataTypes
{
    DATA_KING_VARIAN_WRYNN              = 0,
    DATA_HIGHLORD_TIRION_FORDRING       = 1,
    DATA_GULDAN                         = 2,
    DATA_LADY_JAINA_PROUDMOORE          = 3,
    DATA_GENN_GREYMANE                  = 4,

    // Whether the Alliance's forces are to charge the Spires of Woe (1) or to stay where they are (0)
    DATA_ALLIANCE_FORCES_CHARGING       = 5
};

// spawn_group_template. Each stage's spawns are one group, spawned when the stage starts.
enum BrokenShoreSpawnGroups
{
    SPAWN_GROUP_STORM_THE_BEACH         = 14601,
    SPAWN_GROUP_DEFEAT_THE_COMMANDER    = 14602,
    SPAWN_GROUP_FIND_VARIAN             = 14603,
    SPAWN_GROUP_DESTROY_THE_PORTAL      = 14604,
    SPAWN_GROUP_RAZE_THE_BLACK_CITY     = 14605,
    SPAWN_GROUP_THE_HIGHLORD            = 14606,
    SPAWN_GROUP_KROSUS                  = 14607,
    SPAWN_GROUP_STOP_GULDAN             = 14608,

    // The bridges that Jaina makes are groups of their own, because they stay when their stage ends
    SPAWN_GROUP_BRIDGE_TO_THE_HIGHLORD  = 14609,
    SPAWN_GROUP_BRIDGE_TO_GULDAN        = 14610,

    // So are the Alliance's leaders and the forces that land with them, who go on from stage to stage
    SPAWN_GROUP_ALLIANCE_LEADERS        = 14611,
    SPAWN_GROUP_ALLIANCE_LANDING_FORCE  = 14612,

    // And the Spires of Woe, whose wreckage stays
    SPAWN_GROUP_SPIRES_OF_WOE           = 14613
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
