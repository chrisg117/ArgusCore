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

#ifndef DEF_EMERALD_NIGHTMARE_H_
#define DEF_EMERALD_NIGHTMARE_H_

#include "CreatureAIImpl.h"
#include <array>

#define DataHeader "EmeraldNightmare"
#define ENScriptName "instance_emerald_nightmare"

uint32 const EncounterCount = 7;

enum ENDataTypes
{
    // Encounters, in journal order
    DATA_NYTHENDRA                  = 0,
    DATA_ILGYNOTH,
    DATA_ELERETHE_RENFERAL,
    DATA_URSOC,
    DATA_DRAGONS_OF_NIGHTMARE,
    DATA_CENARIUS,
    DATA_XAVIUS,

    // Extra data, after the encounters
    DATA_EYE_OF_ILGYNOTH,
    DATA_ILGYNOTH_FORCEFIELD,
};

enum ENCreatureIds
{
    // Bosses
    BOSS_NYTHENDRA                  = 102672,
    BOSS_ILGYNOTH                   = 105393,
    BOSS_ELERETHE_RENFERAL          = 106087,
    BOSS_URSOC                      = 100497,
    BOSS_YSONDRE                    = 102679,
    BOSS_TAERAR                     = 102681,
    BOSS_LETHON                     = 102682,
    BOSS_EMERISS                    = 102683,
    BOSS_CENARIUS                   = 104636,
    BOSS_XAVIUS                     = 103769,

    // Nythendra
    NPC_CORRUPTED_VERMIN            = 111005,

    // Il'gynoth
    NPC_EYE_OF_ILGYNOTH             = 105906,
    NPC_DOMINATOR_TENTACLE          = 105304,
    NPC_DEATHGLARE_TENTACLE         = 105322,
    NPC_CORRUPTOR_TENTACLE          = 105383,
    NPC_NIGHTMARE_HORROR            = 105591,
    NPC_NIGHTMARE_ICHOR             = 105721,

    // Elerethe Renferal
    NPC_VENOMOUS_SPIDERLING         = 106311,
    NPC_SKITTERING_SPIDERLING       = 108542,
    NPC_TWISTING_SHADOWS            = 106350,
    NPC_ELERETHE_EGG_SAC            = 108540,
};

enum ENGameObjectIds
{
    GO_NYTHENDRA_ROOT_WALL          = 254985, // NightmareBoss1DungeonDoor, the widest of the 16
    GO_NYTHENDRA_EXIT_ROOT_WALL     = 254987, // NightmareBoss1DungeonDoor, the way on to Il'gynoth
    GO_EYE_OF_ILGYNOTH_FORCEFIELD   = 251555  // a thin collision wall at the hollow mouth
};

// Elerethe Renferal's room, from the .gps readings of 2026-10-19 (map 1520). Shared by the
// instance (the boundary) and the boss script (landings, flight paths).
namespace EleretheRoom
{
    struct Point { float x, y, z; };

    // Where she lands on each of the three platforms (platform circle centres, at the floor z).
    constexpr std::array<Point, 3> Landings =
    {{
        { 11401.604f, 11404.183f, -85.51545f },
        { 11372.929f, 11223.446f, -101.980934f },
        { 11540.084f, 11301.364f, -87.167656f }
    }};

    // The middle of each curved bridge web (P1 to P2, P2 to P3), on the web.
    constexpr std::array<Point, 2> BridgeMiddles =
    {{
        { 11370.096f, 11318.254f, -98.345215f },
        { 11457.219f, 11233.29f, -93.97142f }
    }};

    // Calibrate: the platform radii (yd), used by the boss script; the room boundary is
    // fitted to a walked trail (see the instance script).
    constexpr std::array<float, 3> PlatformRadii = { 45.0f, 45.0f, 45.0f };
}

template <class AI, class T>
inline AI* GetEmeraldNightmareAI(T* obj)
{
    return GetInstanceAI<AI>(obj, ENScriptName);
}

#define RegisterEmeraldNightmareCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetEmeraldNightmareAI)

#endif
