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

#ifndef DEF_EYE_OF_AZSHARA_H_
#define DEF_EYE_OF_AZSHARA_H_

#include "CreatureAIImpl.h"

#define DataHeader "EyeOfAzshara"
#define EOAScriptName "instance_eye_of_azshara"

uint32 const EncounterCount = 5;

enum EOADataTypes
{
    // Encounters, in journal order
    DATA_WARLORD_PARJESH            = 0,
    DATA_LADY_HATECOIL              = 1,
    DATA_KING_DEEPBEARD             = 2,
    DATA_SERPENTRIX                 = 3,
    DATA_WRATH_OF_AZSHARA           = 4,

    // Additional data, after the encounters
    DATA_ARCANISTS                  = 5,    // Hatecoil Arcanists dead, 0 to 4, saved
    DATA_RITUAL                     = 6,    // ritual casters dead, 0 to 4, saved
    DATA_WRATH_BUBBLE               = 7,    // object lookup
    DATA_KEYSTONE_START             = 8     // template H3, last additional id; a stub until Mythic+
};

enum EOACreatureIds
{
    // Bosses
    BOSS_WARLORD_PARJESH            = 91784,
    BOSS_LADY_HATECOIL              = 91789,
    BOSS_KING_DEEPBEARD             = 91797,
    BOSS_SERPENTRIX                 = 91808,
    BOSS_WRATH_OF_AZSHARA           = 96028,

    // Gates
    NPC_HATECOIL_ARCANIST           = 97171,
    NPC_MYSTIC_SSAVEH               = 98173,
    NPC_RITUALIST_LESHA             = 100248,
    NPC_CHANNELER_VARISZ            = 100249,
    NPC_BINDER_ASHIOI               = 100250,

    // Warlord Parjesh
    NPC_HATECOIL_SHELLBREAKER       = 97264,
    NPC_HATECOIL_CRESTRIDER         = 97269,

    // Lady Hatecoil
    NPC_SAND_DUNE                   = 97853,
    NPC_SALTSEA_GLOBULE             = 98293,
    NPC_MONSOON                     = 99852,

    // King Deepbeard
    NPC_CALL_THE_SEAS               = 97844,
    NPC_QUAKE                       = 97916,

    // Serpentrix
    NPC_SINKHOLE                    = 97263,
    NPC_BLAZING_HYDRA_SPAWN         = 97259,
    NPC_ARCANE_HYDRA_SPAWN          = 97260,

    // Wrath of Azshara
    NPC_MYSTIC_TORNADO              = 97673,
    NPC_ARCANE_BOMB                 = 97691,
    NPC_TIDAL_WAVE                  = 97739,
    NPC_WEATHERMAN                  = 97063,
    NPC_LIGHTNING_STALKER           = 97713
};

enum EOAGameObjectIds
{
    GO_DEEPBEARD_DOOR_1             = 246982,   // Water Door, south edge of his arena
    GO_DEEPBEARD_DOOR_2             = 246983,   // Water Door, east edge of his arena
    GO_WRATH_BUBBLE                 = 240788    // at the Wrath's spot while the ritual holds
};

enum EOASharedSpells
{
    SPELL_ARCANE_SHIELDING          = 197868    // on Lady Hatecoil, one stack per living Arcanist (max 3)
};

template <class AI, class T>
inline AI* GetEyeOfAzsharaAI(T* obj)
{
    return GetInstanceAI<AI>(obj, EOAScriptName);
}

#define RegisterEyeOfAzsharaCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetEyeOfAzsharaAI)

#endif
