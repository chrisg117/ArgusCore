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
#include "InstanceScript.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "emerald_nightmare.h"

enum NythendraSpells
{
    SPELL_INFESTED_BREATH           = 202977, // 3 s cast, 5 s aura on the target ticking every 500 ms
    SPELL_INFESTED_BREATH_DAMAGE    = 202978, // cone from the caster to a destination
    SPELL_ROT                       = 203096, // ticks 203097 every 3 s for 9 s
    SPELL_VOLATILE_ROT              = 204463, // detonates (204470, 204777) after 8 s
    SPELL_INFESTED_GROUND           = 203044, // creates area trigger 5887 at a destination
    SPELL_INFESTED_GROUND_DAMAGE    = 203045  // applied to units standing in a pool
};

enum NythendraEvents
{
    EVENT_ROT = 1,
    EVENT_VOLATILE_ROT,
    EVENT_INFESTED_BREATH
};

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

enum NythendraMisc
{
    ENERGY_PER_BREATH               = 50,
    VOLATILE_ROTS_PER_PHASE         = 3,
    POOLS_ON_ROT_EXPIRE             = 3,
    POOLS_ON_VOLATILE_ROT_EXPIRE    = 5
};

// Drops Infested Ground pools around a point. The boss casts them so they are
// hers: Unit::GetAreaTriggers finds them again in Phase 2.
static void SpawnInfestedGround(Unit* caster, Position const& center, uint32 count, float minDist, float maxDist)
{
    for (uint32 i = 0; i < count; ++i)
    {
        Position pos = center;
        caster->MovePosition(pos, frand(minDist, maxDist), frand(0.0f, 2.0f * float(M_PI)));
        caster->CastSpell(pos, SPELL_INFESTED_GROUND, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }
}

// 102672 - Nythendra
struct boss_nythendra : public BossAI
{
    boss_nythendra(Creature* creature) : BossAI(creature, DATA_NYTHENDRA), _rotTargets(1), _volatileRotCount(0) { }

    void JustAppeared() override
    {
        me->SetMaxPower(POWER_ENERGY, 100); // power is set to 0 in Creature::UpdateMaxPower
        me->SetPower(POWER_ENERGY, 100);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        StartPhaseOne();
    }

    void StartPhaseOne()
    {
        _rotTargets = 1;
        _volatileRotCount = 0;
        events.SetPhase(PHASE_ONE);
        events.ScheduleEvent(EVENT_ROT, 8s, 0, PHASE_ONE);
        events.ScheduleEvent(EVENT_VOLATILE_ROT, 20s, 0, PHASE_ONE);
        events.ScheduleEvent(EVENT_INFESTED_BREATH, 45s, 0, PHASE_ONE);
    }

    void CleanUp()
    {
        summons.DespawnAll();
        me->RemoveAllAreaTriggers();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ROT);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VOLATILE_ROT);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_INFESTED_BREATH);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

        CleanUp();
        _EnterEvadeMode();
        me->SetPower(POWER_ENERGY, 100);
        _DespawnAtEvade();
    }

    void JustDied(Unit* /*killer*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanUp();
        _JustDied();
    }

    void OnSpellCast(SpellInfo const* spell) override
    {
        if (spell->Id != SPELL_INFESTED_BREATH)
            return;

        // Each breath costs 50 energy; Rot reaches a second raider after the first.
        int32 energy = std::max<int32>(me->GetPower(POWER_ENERGY) - ENERGY_PER_BREATH, 0);
        me->SetPower(POWER_ENERGY, energy);
        _rotTargets = 2;

        if (energy > 0)
            events.ScheduleEvent(EVENT_INFESTED_BREATH, 45s, 0, PHASE_ONE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ROT:
                {
                    std::list<Unit*> targets;
                    SelectTargetList(targets, _rotTargets, SelectTargetMethod::Random, 0, 0.0f, true, false, -int32(SPELL_ROT));
                    if (targets.empty())
                        if (Unit* victim = me->GetVictim())
                            targets.push_back(victim); // solo: the tank is the only raider

                    for (Unit* target : targets)
                    {
                        Talk(TALK_ROT, target);
                        DoCast(target, SPELL_ROT, true);
                    }
                    events.Repeat(16s);
                    break;
                }
                case EVENT_VOLATILE_ROT:
                    if (Unit* victim = me->GetVictim())
                    {
                        Talk(TALK_VOLATILE_ROT, victim);
                        DoCast(victim, SPELL_VOLATILE_ROT, true);
                    }
                    if (++_volatileRotCount < VOLATILE_ROTS_PER_PHASE)
                        events.Repeat(30s);
                    break;
                case EVENT_INFESTED_BREATH:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true))
                    {
                        Talk(TALK_INFESTED_BREATH);
                        DoCast(target, SPELL_INFESTED_BREATH);
                    }
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    uint32 _rotTargets;
    uint32 _volatileRotCount;
};

// 202977 - Infested Breath
// The client data ticks a missile (203537) whose follow-up spell is missing,
// and nothing casts the cone damage, so the ticks are driven here: every
// second tick (once a second) casts the cone, the others drop a pocket of
// Infested Ground inside it. The cone stays aimed where the target stood when
// the breath began.
class spell_nythendra_infested_breath : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_INFESTED_BREATH_DAMAGE, SPELL_INFESTED_GROUND });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        _aim = GetTarget()->GetPosition();
    }

    void HandlePeriodic(AuraEffect const* aurEff)
    {
        PreventDefaultAction();

        Unit* caster = GetCaster();
        if (!caster)
            return;

        if (aurEff->GetTickNumber() % 2 == 0)
        {
            caster->CastSpell(_aim, SPELL_INFESTED_BREATH_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
            return;
        }

        Position pos = caster->GetPosition();
        float angle = caster->GetAbsoluteAngle(_aim) + frand(-0.35f, 0.35f);
        caster->MovePosition(pos, frand(8.0f, std::max(10.0f, caster->GetExactDist2d(_aim))), angle - caster->GetOrientation());
        caster->CastSpell(pos, SPELL_INFESTED_GROUND, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_nythendra_infested_breath::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_nythendra_infested_breath::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }

    Position _aim;
};

// 203096 - Rot
// 204463 - Volatile Rot
// The pools left when the debuff runs out are not in the spell data.
template<uint32 PoolCount, uint32 MinDist, uint32 MaxDist>
class spell_nythendra_rot_pools : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_INFESTED_GROUND });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        if (Unit* caster = GetCaster())
            SpawnInfestedGround(caster, GetTarget()->GetPosition(), PoolCount, float(MinDist), float(MaxDist));
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_nythendra_rot_pools::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 5887 - Infested Ground (created by 203044)
struct at_nythendra_infested_ground : AreaTriggerAI
{
    at_nythendra_infested_ground(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        if (Unit* caster = at->GetCaster())
            caster->CastSpell(unit, SPELL_INFESTED_GROUND_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void OnUnitExit(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        // Pools overlap: keep the damage while the player still stands in another one.
        if (Unit* caster = at->GetCaster())
            for (AreaTrigger* other : caster->GetAreaTriggers(SPELL_INFESTED_GROUND))
                if (other != at && other->GetInsideUnits().contains(unit->GetGUID()))
                    return;

        unit->RemoveAurasDueToSpell(SPELL_INFESTED_GROUND_DAMAGE, at->GetCasterGuid());
    }
};

void AddSC_boss_nythendra()
{
    RegisterEmeraldNightmareCreatureAI(boss_nythendra);
    RegisterSpellScript(spell_nythendra_infested_breath);
    RegisterSpellScriptWithArgs((spell_nythendra_rot_pools<POOLS_ON_ROT_EXPIRE, 2, 5>), "spell_nythendra_rot");
    RegisterSpellScriptWithArgs((spell_nythendra_rot_pools<POOLS_ON_VOLATILE_ROT_EXPIRE, 3, 9>), "spell_nythendra_volatile_rot");
    RegisterAreaTriggerAI(at_nythendra_infested_ground);
}
