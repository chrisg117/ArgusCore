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

#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "TemporarySummon.h"
#include "emerald_nightmare.h"

enum NythendraTalks
{
    TALK_INFESTED_BREATH    = 0, // raid boss emote, broadcast 106119
    TALK_ROT                = 1, // raid boss whisper to the target, broadcast 105376
    TALK_VOLATILE_ROT       = 2  // raid boss whisper to the target, broadcast 105969
};

enum NythendraPhases
{
    PHASE_ONE = 1,
    PHASE_TWO
};

// 102672 - Nythendra
struct boss_nythendra : public BossAI
{
    boss_nythendra(Creature* creature) : BossAI(creature, DATA_NYTHENDRA) { }

    void JustAppeared() override
    {
        me->SetMaxPower(POWER_ENERGY, 100); // power is set to 0 in Creature::UpdateMaxPower
        me->SetPower(POWER_ENERGY, 100);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        summons.DespawnAll();
        _EnterEvadeMode();
        me->SetPower(POWER_ENERGY, 100);
        _DespawnAtEvade();
    }

    void JustDied(Unit* /*killer*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        _JustDied();
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (!UpdateVictim())
            return;
    }
};

void AddSC_boss_nythendra()
{
    RegisterEmeraldNightmareCreatureAI(boss_nythendra);
}
