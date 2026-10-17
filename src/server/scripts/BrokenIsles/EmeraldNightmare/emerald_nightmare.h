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
};

template <class AI, class T>
inline AI* GetEmeraldNightmareAI(T* obj)
{
    return GetInstanceAI<AI>(obj, ENScriptName);
}

#define RegisterEmeraldNightmareCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetEmeraldNightmareAI)

#endif
