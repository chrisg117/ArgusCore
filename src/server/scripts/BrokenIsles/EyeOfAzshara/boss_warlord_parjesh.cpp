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
#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "eye_of_azshara.h"

enum ParjeshSpells
{
    // Warlord Parjesh
    SPELL_CALL_REINFORCEMENTS           = 196563,   // Heroic and Mythic: one of each
    SPELL_CALL_REINFORCEMENTS_MELEE     = 192072,   // Normal; summons 97264
    SPELL_CALL_REINFORCEMENTS_CASTER    = 192073,   // Normal; summons 97269
    SPELL_CRASHING_WAVE                 = 191900,   // 100 energy; charge 191902, area triggers 4863 / 4888
    SPELL_CRASHING_WAVE_DAMAGE          = 191919,
    SPELL_QUICKSAND_DAMAGE              = 192053,
    SPELL_IMPALING_SPEAR_MARK           = 192094,
    SPELL_IMPALING_SPEAR                = 191977,
    SPELL_THROW_SPEAR                   = 192131,
    SPELL_ENRAGE                        = 197064,

    // Hatecoil Shellbreaker
    SPELL_BELLOWING_ROAR                = 192135,

    // Hatecoil Crestrider
    SPELL_LIGHTNING_STRIKE              = 192138,
    SPELL_RESTORATION                   = 197502
};

enum ParjeshTalks
{
    TALK_AGGRO                          = 0,    // 99890
    TALK_CALL_MELEE                     = 1,    // 99891
    TALK_CALL_CASTER                    = 2,    // 99892
    TALK_WAVE                           = 3,    // 99893
    TALK_SPEAR                          = 4,    // 99898, 99899
    TALK_KILL                           = 5,    // 99896, 99897
    TALK_WIPE                           = 6,    // 100462
    TALK_DEATH                          = 7,    // 99894
    EMOTE_SPEAR                         = 8,    // 99873, whispered to the marked player
    TALK_SPEAR_HIT_ADD                  = 9,    // 99895
    TALK_INTRO_LANDED                   = 10,   // 101992, when the first player enters (said through the instance)
    TALK_INTRO_NEAR                     = 11    // 101991, when a player comes near
};

enum ParjeshEvents
{
    EVENT_REINFORCEMENTS                = 1,
    EVENT_CRASHING_WAVE,
    EVENT_IMPALING_SPEAR,
    EVENT_THROW_SPEAR,
    EVENT_ENERGY,

    EVENT_BELLOWING_ROAR,
    EVENT_LIGHTNING_STRIKE,
    EVENT_RESTORATION
};

namespace
{
    // Calibrate: half the width of the spear's line, in yards.
    float constexpr SpearLineHalfWidth = 2.5f;

    uint32 constexpr AreaTriggerQuicksand = 4876;

    bool IsNormalDifficulty(Unit const* unit)
    {
        return unit->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL;
    }

    bool IsParjeshAdd(Unit const* unit)
    {
        return unit->GetEntry() == NPC_HATECOIL_SHELLBREAKER || unit->GetEntry() == NPC_HATECOIL_CRESTRIDER;
    }
}

// 91784 - Warlord Parjesh
// Timers follow DBM 7.3.31 (DBM-Party-Legion/EyeOfAzshara/Parjesh.lua): reinforcements 3 s after the pull,
// then 28 s (Heroic, Mythic: both adds at once) or alternating 33 s after the melee call and 20 s after the
// caster call (Normal); the spear mark at 28 s and every 28 s. Crashing Wave has no bar; the module's note puts it
// a few seconds before each spear (calibrate: 23 s, then 28 s).
struct boss_warlord_parjesh : public BossAI
{
    boss_warlord_parjesh(Creature* creature) : BossAI(creature, DATA_WARLORD_PARJESH) { }

    void JustAppeared() override
    {
        // The energy bar is cosmetic (addon aura 202146 empties it and stops regeneration); the event map is
        // the clock, and the bar fills towards each Crashing Wave, which costs 100.
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 0);
    }

    void Reset() override
    {
        _Reset();
        _nextCallIsMelee = true;
        _enraged = false;
        _energyTicks = 0;
        _waveInterval = 23;
        me->SetPower(POWER_ENERGY, 0);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        Talk(TALK_AGGRO);

        events.ScheduleEvent(EVENT_REINFORCEMENTS, 3s);
        events.ScheduleEvent(EVENT_CRASHING_WAVE, 23s);
        events.ScheduleEvent(EVENT_IMPALING_SPEAR, 28s);
        events.ScheduleEvent(EVENT_THROW_SPEAR, 8s);
        events.ScheduleEvent(EVENT_ENERGY, 1s);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(TALK_KILL);
    }

    // Intro (RP task): once per instance load, when a player first comes within 60 yd. Gates nothing.
    void MoveInLineOfSight(Unit* who) override
    {
        if (!_introNear && who->IsPlayer() && !who->ToPlayer()->IsGameMaster() && me->IsWithinDistInMap(who, 60.0f)
            && instance->GetBossState(DATA_WARLORD_PARJESH) != DONE)
        {
            _introNear = true;
            Talk(TALK_INTRO_NEAR);
        }

        BossAI::MoveInLineOfSight(who);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (!_enraged && me->HealthBelowPctDamaged(30, damage))
        {
            _enraged = true;
            DoCastSelf(SPELL_ENRAGE, true);
        }
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        Talk(TALK_WIPE);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        me->RemoveAllAreaTriggers();
        _EnterEvadeMode();
        _DespawnAtEvade();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(TALK_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        me->RemoveAllAreaTriggers();
        _JustDied();
    }

    void OnSpellCast(SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_CRASHING_WAVE)
        {
            _energyTicks = 0;
            _waveInterval = 28;
            me->SetPower(POWER_ENERGY, 0);
        }
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
                // The calls, the wave and Throw Spear share category 1152 ("Creature Special 1"): a cast refused by
                // its category cooldown is tried again 0.5 s later instead of being lost.
                case EVENT_REINFORCEMENTS:
                    if (IsNormalDifficulty(me))
                    {
                        // Calibrate: which call comes first on Normal (the module creates its bar on 192072).
                        bool const melee = _nextCallIsMelee;
                        if (DoCastSelf(melee ? SPELL_CALL_REINFORCEMENTS_MELEE : SPELL_CALL_REINFORCEMENTS_CASTER) != SPELL_CAST_OK)
                        {
                            events.Repeat(500ms);
                            break;
                        }
                        _nextCallIsMelee = !melee;
                        Talk(melee ? TALK_CALL_MELEE : TALK_CALL_CASTER);
                        events.Repeat(melee ? 33s : 20s);
                    }
                    else
                    {
                        if (DoCastSelf(SPELL_CALL_REINFORCEMENTS) != SPELL_CAST_OK)
                        {
                            events.Repeat(500ms);
                            break;
                        }
                        Talk(urand(0, 1) ? TALK_CALL_MELEE : TALK_CALL_CASTER);
                        events.Repeat(28s);
                    }
                    break;
                case EVENT_CRASHING_WAVE:
                    // The wave is a charge 25 yd in front of him: aimed down the line to his current victim.
                    if (Unit* victim = me->GetVictim())
                        me->SetFacingToObject(victim);
                    me->SetPower(POWER_ENERGY, 100);
                    if (DoCastSelf(SPELL_CRASHING_WAVE) != SPELL_CAST_OK)
                    {
                        events.Repeat(500ms);
                        break;
                    }
                    Talk(TALK_WAVE);
                    events.Repeat(28s);
                    break;
                case EVENT_IMPALING_SPEAR:
                {
                    // A non-tank; solo, the only player.
                    Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 0.0f, true);
                    if (!target)
                        target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                    if (target)
                    {
                        Talk(TALK_SPEAR);
                        Talk(EMOTE_SPEAR, target);
                        DoCast(target, SPELL_IMPALING_SPEAR_MARK);
                    }
                    events.Repeat(28s);
                    break;
                }
                case EVENT_THROW_SPEAR:
                    // Calibrate: no DBM timer; first 8 s, then every 10 s between other casts. Its 3 s category
                    // cooldown would hold back a call or a wave due right after it, so it waits for them.
                    if (events.GetTimeUntilEvent(EVENT_CRASHING_WAVE) < 4s || events.GetTimeUntilEvent(EVENT_REINFORCEMENTS) < 4s)
                    {
                        events.Repeat(5s);
                        break;
                    }
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 60.0f, true))
                        DoCast(target, SPELL_THROW_SPEAR);
                    events.Repeat(10s);
                    break;
                case EVENT_ENERGY:
                    ++_energyTicks;
                    me->SetPower(POWER_ENERGY, std::min<int32>(100, int32(_energyTicks * 100 / _waveInterval)));
                    events.Repeat(1s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    bool _introNear = false;
    bool _nextCallIsMelee = true;
    bool _enraged = false;
    uint32 _energyTicks = 0;
    uint32 _waveInterval = 23;
};

// 97264 - Hatecoil Shellbreaker
struct npc_parjesh_shellbreaker : public ScriptedAI
{
    npc_parjesh_shellbreaker(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        // Calibrate: no DBM timer.
        _events.ScheduleEvent(EVENT_BELLOWING_ROAR, 6s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_BELLOWING_ROAR)
            {
                DoCastSelf(SPELL_BELLOWING_ROAR);
                _events.Repeat(15s);
            }
        }
    }

private:
    EventMap _events;
};

// 97269 - Hatecoil Crestrider
struct npc_parjesh_crestrider : public ScriptedAI
{
    npc_parjesh_crestrider(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _events.Reset();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        // Calibrate: no DBM timers. Restoration is in the journal's Heroic and Mythic section.
        _events.ScheduleEvent(EVENT_LIGHTNING_STRIKE, 2s);
        if (!IsNormalDifficulty(me))
            _events.ScheduleEvent(EVENT_RESTORATION, 10s);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);
        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_LIGHTNING_STRIKE:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 40.0f, true))
                        DoCast(target, SPELL_LIGHTNING_STRIKE);
                    _events.Repeat(6s);
                    break;
                case EVENT_RESTORATION:
                    DoCastSelf(SPELL_RESTORATION);
                    _events.Repeat(25s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    EventMap _events;
};

// 196563 - Call Reinforcements (Heroic, Mythic)
// Both effects are dummies: the call brings one of each add, through the Normal calls' own summon spells
// (192072: Shellbreaker, 192073: Crestrider; wild summons within 50 yd).
class spell_parjesh_call_reinforcements : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CALL_REINFORCEMENTS_MELEE, SPELL_CALL_REINFORCEMENTS_CASTER });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        caster->CastSpell(caster, SPELL_CALL_REINFORCEMENTS_MELEE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        caster->CastSpell(caster, SPELL_CALL_REINFORCEMENTS_CASTER, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_parjesh_call_reinforcements::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 192094 - Impaling Spear (the 5 s mark)
// When the mark ends, the spear (191977) hits the first unit on the line from Parjesh to the marked player:
// one of his reinforcements, another player, or the marked player when nobody is in the way.
class spell_parjesh_impaling_spear_mark : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_IMPALING_SPEAR });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Creature* parjesh = Object::ToCreature(GetCaster());
        Unit* marked = GetTarget();
        if (!parjesh || !parjesh->IsAlive() || !marked->IsAlive())
            return;

        float const length = parjesh->GetExactDist2d(marked);
        Unit* hit = marked;
        float best = length;

        std::list<Unit*> units;
        Trinity::AnyUnitInObjectRangeCheck check(parjesh, length);
        Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(parjesh, units, check);
        Cell::VisitAllObjects(parjesh, searcher, length);
        for (Unit* unit : units)
        {
            if (unit == marked || unit == parjesh || !unit->IsAlive())
                continue;
            if (!unit->IsPlayer() && !IsParjeshAdd(unit))
                continue;
            if (!unit->IsInBetween(parjesh, marked, SpearLineHalfWidth))
                continue;
            float const dist = parjesh->GetExactDist2d(unit);
            if (dist < best)
            {
                best = dist;
                hit = unit;
            }
        }

        if (hit != marked && IsParjeshAdd(hit))
            if (parjesh->IsAIEnabled())
                parjesh->AI()->Talk(TALK_SPEAR_HIT_ADD);

        parjesh->CastSpell(hit, SPELL_IMPALING_SPEAR, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_parjesh_impaling_spear_mark::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 4863 - Crashing Wave (attached to Parjesh while he charges; 4888 is its visual and has no script)
struct at_parjesh_crashing_wave : AreaTriggerAI
{
    at_parjesh_crashing_wave(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        if (Unit* caster = at->GetCaster())
            if (_struck.insert(unit->GetGUID()).second)
                caster->CastSpell(unit, SPELL_CRASHING_WAVE_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

private:
    GuidSet _struck;
};

struct QuicksandCheck
{
    bool operator()(AreaTrigger* at) const { return at->GetEntry() == AreaTriggerQuicksand; }
};

// 4876 - Quicksand (Heroic, Mythic; created by 192041 where each missile lands; 4887 is its visual)
struct at_parjesh_quicksand : AreaTriggerAI
{
    at_parjesh_quicksand(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        if (Unit* caster = at->GetCaster())
            caster->CastSpell(unit, SPELL_QUICKSAND_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void OnUnitExit(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        // Puddles overlap: keep the slow while the player still stands in another one.
        std::list<AreaTrigger*> puddles;
        QuicksandCheck check;
        Trinity::AreaTriggerListSearcher<QuicksandCheck> searcher(at, puddles, check);
        Cell::VisitAllObjects(at, searcher, 20.0f);
        for (AreaTrigger* other : puddles)
            if (other != at && other->GetInsideUnits().contains(unit->GetGUID()))
                return;

        unit->RemoveAurasDueToSpell(SPELL_QUICKSAND_DAMAGE);
    }
};

void AddSC_boss_warlord_parjesh()
{
    RegisterEyeOfAzsharaCreatureAI(boss_warlord_parjesh);
    RegisterEyeOfAzsharaCreatureAI(npc_parjesh_shellbreaker);
    RegisterEyeOfAzsharaCreatureAI(npc_parjesh_crestrider);
    RegisterSpellScript(spell_parjesh_call_reinforcements);
    RegisterSpellScript(spell_parjesh_impaling_spear_mark);
    RegisterAreaTriggerAI(at_parjesh_crashing_wave);
    RegisterAreaTriggerAI(at_parjesh_quicksand);
}
