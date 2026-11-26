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

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Creature.h"
#include "ScriptMgr.h"
#include "SpellInfo.h"
#include "eye_of_azshara.h"

enum EOATrashSpells
{
    SPELL_REJUVENATING_WATERS_HEAL      = 195060
};

// 5149 - Rejuvenating Waters (Hatecoil Oracle, 195046; 15 s): the Oracle's allies standing in the pool heal.
struct at_eoa_rejuvenating_waters : AreaTriggerAI
{
    at_eoa_rejuvenating_waters(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUnitEnter(Unit* unit) override
    {
        Unit* caster = at->GetCaster();
        if (!caster || unit->IsPlayer() || !caster->IsFriendlyTo(unit))
            return;

        unit->CastSpell(unit, SPELL_REJUVENATING_WATERS_HEAL, CastSpellExtraArgs(TRIGGERED_FULL_MASK).SetOriginalCaster(caster->GetGUID()));
    }

    void OnUnitExit(Unit* unit) override
    {
        if (!unit->IsPlayer())
            unit->RemoveAurasDueToSpell(SPELL_REJUVENATING_WATERS_HEAL);
    }
};

void AddSC_eye_of_azshara_trash()
{
    RegisterAreaTriggerAI(at_eoa_rejuvenating_waters);
}
