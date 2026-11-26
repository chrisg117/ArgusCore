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
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "eye_of_azshara.h"
#include <cmath>

enum WrathSpells
{
    SPELL_MYSTIC_TORNADO                = 192680,   // instant dummy at players within 150 yd
    SPELL_MYSTIC_TORNADO_SUMMON         = 193014,   // the tornado stalker (97673), 180 s
    SPELL_MYSTIC_TORNADO_AURA           = 192673,   // on the stalker: area trigger 4930
    SPELL_MYSTIC_TORNADO_HIT            = 192675,
    SPELL_MASSIVE_DELUGE                = 192617,
    SPELL_ARCANE_BOMB                   = 192706,   // the 15 s debuff
    SPELL_ARCANE_BOMB_BLAST             = 192708,   // the bomb stalker's 15 s cast
    SPELL_RAGING_STORMS                 = 192696,
    SPELL_HEAVING_SANDS                 = 197164,
    SPELL_CRUSHING_DEPTHS               = 197365,
    SPELL_MAGIC_RESONANCE               = 196665,
    SPELL_FROST_RESONANCE               = 196666,
    SPELL_CRY_OF_WRATH                  = 192985,
    SPELL_VIOLENT_WINDS                 = 191797,
    SPELL_LIGHTNING_STRIKES             = 192796,   // missile, lands 192794
    SPELL_TIDAL_WAVE                    = 192801
};

enum WrathTalks
{
    TALK_AWAKEN                         = 0,    // 99950, said by the instance when the ritual breaks
    TALK_AGGRO                          = 1,    // 100046
    TALK_DELUGE                         = 2,    // 99948
    TALK_CRUSHING_DEPTHS                = 3,    // 99947
    TALK_TORNADO                        = 4,    // 99949
    TALK_ARCANE_BOMB                    = 5,    // 99956, 99957
    TALK_CRY_OF_WRATH                   = 6,    // 99943, 99944
    TALK_KILL                           = 7,    // 99945, 99946
    TALK_DEATH                          = 8,    // 100477
    EMOTE_ARCANE_BOMB                   = 9     // 99877, names the target and links 192708 for DBM
};

enum WrathEvents
{
    EVENT_MYSTIC_TORNADO                = 1,
    EVENT_MASSIVE_DELUGE,
    EVENT_ARCANE_BOMB,
    EVENT_CRUSHING_DEPTHS,
    EVENT_RAGING_STORMS_CHECK,
    EVENT_CRY_OF_WRATH,
    EVENT_STORM_WINDS,
    EVENT_STORM_LIGHTNING,
    EVENT_STORM_WAVE
};

enum WrathPhases
{
    PHASE_ONE                           = 1,
    PHASE_TWO                           = 2
};

namespace
{
    // Calibrate: seconds with no player in melee reach before Raging Storms.
    uint32 constexpr RagingStormsDelaySeconds = 3;
    // Calibrate: the Tidal Wave stalker's path length and the width of its wave.
    float constexpr TidalWaveRange = 45.0f;
    float constexpr TidalWaveWidth = 4.0f;

    bool IsNormalDifficulty(Unit const* unit)
    {
        return unit->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL;
    }

    ObjectGuid WindForceId(Unit const* caster)
    {
        return ObjectGuid::Create<HighGuid::AreaTrigger>(caster->GetMapId(), SPELL_VIOLENT_WINDS, caster->GetGUID().GetCounter());
    }
}

// 96028 - Wrath of Azshara
// Timers follow DBM 7.3.31 (DBM-Party-Legion/EyeOfAzshara/WrathofAzshara.lua): Mystic Tornado 8.5 s after the pull
// and every 25 s (15 s in phase 2, on UNIT_SPELLCAST_SUCCEEDED boss1), Massive Deluge 12 s and every 50 s (35 s in
// phase 2), Arcane Bomb 23 s and every 23 s (restarted by the boss emote naming spell 192708). Cry of Wrath at 10%
// starts phase 2 and the storm: Violent Winds every 10 s (the Serpentrix module's note), Lightning Strikes every 5 s
// (the Lightning Stalker's 192737 period), Tidal Waves (calibrate). The storm is the only zone weather built in the
// first pass (user ruling 2026-11-26).
struct boss_wrath_of_azshara : public BossAI
{
    boss_wrath_of_azshara(Creature* creature) : BossAI(creature, DATA_WRATH_OF_AZSHARA) { }

    void Reset() override
    {
        _Reset();
        _cried = false;
        _outOfReach = 0;
        _windAngle = 0.0f;
        // She stands in the water at the centre of the island.
        me->SetControlled(true, UNIT_STATE_ROOT);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        Talk(TALK_AGGRO);
        DoCastSelf(SPELL_HEAVING_SANDS, true);

        events.SetPhase(PHASE_ONE);
        events.ScheduleEvent(EVENT_MYSTIC_TORNADO, 8500ms);
        events.ScheduleEvent(EVENT_MASSIVE_DELUGE, 12s);
        events.ScheduleEvent(EVENT_ARCANE_BOMB, 23s);
        if (!IsNormalDifficulty(me))
            events.ScheduleEvent(EVENT_CRUSHING_DEPTHS, 20s);   // calibrate: no DBM timer
        events.ScheduleEvent(EVENT_RAGING_STORMS_CHECK, 1s);
    }

    // The wind's direction, in degrees, for Violent Winds' aura script (spell_eoa_violent_winds).
    uint32 GetData(uint32 /*type*/) const override
    {
        return uint32(_windAngle * 180.0f / float(M_PI));
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(TALK_KILL);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (!_cried && me->HealthBelowPctDamaged(10, damage))
        {
            _cried = true;
            events.ScheduleEvent(EVENT_CRY_OF_WRATH, 0s);
        }
    }

    void OnSpellCast(SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_CRY_OF_WRATH)
            StartPhaseTwo();
    }

    void OnSpellFailed(SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_CRY_OF_WRATH)
            events.ScheduleEvent(EVENT_CRY_OF_WRATH, 1s);
    }

    void CleanUp()
    {
        summons.DespawnAll();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_ARCANE_BOMB);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VIOLENT_WINDS);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_RAGING_STORMS);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_MAGIC_RESONANCE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FROST_RESONANCE);
        ObjectGuid const id = WindForceId(me);
        me->GetMap()->DoOnPlayers([id](Player* player) { player->RemoveMovementForce(id); });
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanUp();
        _EnterEvadeMode();
        _DespawnAtEvade();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(TALK_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanUp();
        _JustDied();
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
                case EVENT_MYSTIC_TORNADO:
                    Talk(TALK_TORNADO);
                    DoCastSelf(SPELL_MYSTIC_TORNADO);
                    events.Repeat(events.IsInPhase(PHASE_TWO) ? 15s : 25s);
                    break;
                case EVENT_MASSIVE_DELUGE:
                    if (Unit* victim = me->GetVictim())
                    {
                        Talk(TALK_DELUGE);
                        me->SetFacingToObject(victim);
                        me->CastSpell(victim->GetPosition(), SPELL_MASSIVE_DELUGE, CastSpellExtraArgs());
                    }
                    events.Repeat(events.IsInPhase(PHASE_TWO) ? 35s : 50s);
                    break;
                case EVENT_ARCANE_BOMB:
                {
                    Unit* target = SelectTarget(SelectTargetMethod::Random, 1, 0.0f, true);
                    if (!target)
                        target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true);
                    if (target)
                    {
                        Talk(TALK_ARCANE_BOMB);
                        Talk(EMOTE_ARCANE_BOMB, target);
                        DoCast(target, SPELL_ARCANE_BOMB);
                    }
                    events.Repeat(23s);
                    break;
                }
                case EVENT_CRUSHING_DEPTHS:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 150.0f, true))
                    {
                        Talk(TALK_CRUSHING_DEPTHS);
                        DoCast(target, SPELL_CRUSHING_DEPTHS);
                    }
                    events.Repeat(40s);    // calibrate
                    break;
                case EVENT_RAGING_STORMS_CHECK:
                    CheckReach();
                    events.Repeat(1s);
                    break;
                case EVENT_CRY_OF_WRATH:
                    // Retried until it goes through (OnSpellFailed reschedules an interrupted one): phase 2 must come.
                    if (DoCastSelf(SPELL_CRY_OF_WRATH) != SPELL_CAST_OK)
                    {
                        events.Repeat(500ms);
                        break;
                    }
                    Talk(TALK_CRY_OF_WRATH);
                    break;
                case EVENT_STORM_WINDS:
                    _windAngle = frand(0.0f, 2.0f * float(M_PI));
                    me->GetMap()->DoOnPlayers([this](Player* player)
                    {
                        if (player->IsAlive() && me->IsWithinDistInMap(player, 150.0f))
                            me->AddAura(SPELL_VIOLENT_WINDS, player);
                    });
                    events.Repeat(10s);
                    break;
                case EVENT_STORM_LIGHTNING:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 150.0f, true))
                        DoCast(target, SPELL_LIGHTNING_STRIKES, true);
                    events.Repeat(5s);
                    break;
                case EVENT_STORM_WAVE:
                    SendTidalWave();
                    events.Repeat(15s);    // calibrate
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    void StartPhaseTwo()
    {
        events.SetPhase(PHASE_TWO);
        // The current bars carry on; their repeats use the phase-2 times.
        events.ScheduleEvent(EVENT_STORM_WINDS, 10s, 0, PHASE_TWO);
        events.ScheduleEvent(EVENT_STORM_LIGHTNING, 5s, 0, PHASE_TWO);
        events.ScheduleEvent(EVENT_STORM_WAVE, 8s, 0, PHASE_TWO);
    }

    void CheckReach()
    {
        bool inReach = false;
        for (auto const& ref : me->GetThreatManager().GetUnsortedThreatList())
            if (Unit* unit = ref->GetVictim())
                if (unit->IsPlayer() && unit->IsAlive() && me->IsWithinMeleeRange(unit))
                    inReach = true;

        if (inReach)
        {
            _outOfReach = 0;
            return;
        }

        if (++_outOfReach >= RagingStormsDelaySeconds)
        {
            _outOfReach = 0;
            DoCastSelf(SPELL_RAGING_STORMS);
        }
    }

    // A wave rolls in from a random side, through her spot and out the other side.
    void SendTidalWave()
    {
        float const angle = frand(0.0f, 2.0f * float(M_PI));
        Position from = me->GetPosition();
        me->MovePosition(from, TidalWaveRange, angle);
        if (Creature* wave = me->SummonCreature(NPC_TIDAL_WAVE, from, TEMPSUMMON_TIMED_DESPAWN, 20s))
        {
            Position to = me->GetPosition();
            me->MovePosition(to, TidalWaveRange, angle + float(M_PI));
            wave->GetMotionMaster()->MovePoint(1, to, false);
        }
    }

    bool _cried = false;
    uint32 _outOfReach = 0;
    float _windAngle = 0.0f;
};

// 97673 - Mystic Tornado: carries area trigger 4930 and wanders where it was dropped.
struct npc_wrath_mystic_tornado : public ScriptedAI
{
    npc_wrath_mystic_tornado(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetUninteractible(true);
        DoCastSelf(SPELL_MYSTIC_TORNADO_AURA, true);
        me->GetMotionMaster()->MoveRandom(8.0f);   // calibrate
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

// 97691 - Arcane Bomb: hangs over the marked player and casts its 15 s blast; if the debuff is dispelled it
// stops following and falls where it is.
struct npc_wrath_arcane_bomb : public ScriptedAI
{
    npc_wrath_arcane_bomb(Creature* creature) : ScriptedAI(creature) { }

    // The Arcane Bomb aura script makes it follow its player and starts the blast.
    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetUninteractible(true);
    }

    void DoAction(int32 /*action*/) override
    {
        // The debuff was dispelled: the bomb drops here.
        me->GetMotionMaster()->Clear();
        me->StopMoving();
    }

    void OnSpellCast(SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_ARCANE_BOMB_BLAST)
            me->DespawnOrUnsummon(1s);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

// 97739 - Tidal Wave (phase 2): strikes the players it passes over.
struct npc_wrath_tidal_wave : public ScriptedAI
{
    npc_wrath_tidal_wave(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetUninteractible(true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_tick > diff)
        {
            _tick -= diff;
            return;
        }
        _tick = 250;

        InstanceScript* instance = me->GetInstanceScript();
        Creature* wrath = instance ? instance->GetCreature(DATA_WRATH_OF_AZSHARA) : nullptr;
        if (!wrath || !wrath->IsAlive())
            return;

        me->GetMap()->DoOnPlayers([&](Player* player)
        {
            if (player->IsAlive() && me->GetExactDist2d(player) <= TidalWaveWidth && _struck.insert(player->GetGUID()).second)
                wrath->CastSpell(player, SPELL_TIDAL_WAVE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        });
    }

private:
    uint32 _tick = 0;
    GuidSet _struck;
};

// 192680 - Mystic Tornado: a tornado 5-10 yd from each player it reaches.
class spell_wrath_mystic_tornado : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MYSTIC_TORNADO_SUMMON });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        Position pos = target->GetPosition();
        target->MovePosition(pos, frand(5.0f, 10.0f), frand(0.0f, 2.0f * float(M_PI)));
        GetCaster()->CastSpell(pos, SPELL_MYSTIC_TORNADO_SUMMON, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_wrath_mystic_tornado::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 192706 - Arcane Bomb: the bomb stalker over the player. The data's vehicle id (4403) is not applied: the bomb
// follows by position instead (the spec's fallback; a vehicle kit on a player mid-fight is unproven here).
class spell_wrath_arcane_bomb : public AuraScript
{
    void PreventVehicle(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        PreventDefaultAction();
    }

    // The Wrath summons the bomb, so her summon list despawns it on a wipe and on her death.
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster)
            return;

        Position pos = target->GetPosition();
        pos.m_positionZ += 3.0f;
        if (TempSummon* bomb = caster->SummonCreature(NPC_ARCANE_BOMB, pos, TEMPSUMMON_TIMED_DESPAWN, 22s))
        {
            _bomb = bomb->GetGUID();
            bomb->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f);
            bomb->CastSpell(bomb, SPELL_ARCANE_BOMB_BLAST, CastSpellExtraArgs());
        }
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_ENEMY_SPELL)
            return;

        if (Creature* bomb = ObjectAccessor::GetCreature(*GetTarget(), _bomb))
            if (bomb->IsAIEnabled())
                bomb->AI()->DoAction(0);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_wrath_arcane_bomb::PreventVehicle, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        AfterEffectApply += AuraEffectApplyFn(spell_wrath_arcane_bomb::HandleApply, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        OnEffectRemove += AuraEffectRemoveFn(spell_wrath_arcane_bomb::PreventVehicle, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_wrath_arcane_bomb::HandleRemove, EFFECT_0, SPELL_AURA_SET_VEHICLE_ID, AURA_EFFECT_HANDLE_REAL);
    }

    ObjectGuid _bomb;
};

// 197365 - Crushing Depths (Heroic, Mythic): 60% of maximum health, split among the players within 12 yd.
class spell_wrath_crushing_depths : public SpellScript
{
    // The hook runs before the default handler, which would add the full percentage on top: so the default is
    // prevented and the split damage is set here.
    void HandleSplit(SpellEffIndex effIndex)
    {
        Unit* target = GetHitUnit();
        if (!target)
            return;

        PreventHitDefaultEffect(effIndex);
        int64 const count = std::max<int64>(1, GetUnitTargetCountForEffect(effIndex));
        SetHitDamage(int32(target->CountPctFromMaxHealth(GetEffectValue()) / count));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_wrath_crushing_depths::HandleSplit, EFFECT_0, SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT);
    }
};

// Elemental Resonance (Heroic, Mythic): her Arcane and Nature hits leave Magic Resonance (196665), her Frost hits
// Frost Resonance (196666). Hooked on each damaging spell of the fight.
class spell_wrath_resonance : public SpellScript
{
    void HandleHit()
    {
        Unit* target = GetHitUnit();
        Unit* caster = GetCaster();
        if (!target || !caster || !target->IsPlayer() || IsNormalDifficulty(caster))
            return;

        uint32 const school = GetSpellInfo()->SchoolMask;
        if (school & (SPELL_SCHOOL_MASK_ARCANE | SPELL_SCHOOL_MASK_NATURE))
            caster->CastSpell(target, SPELL_MAGIC_RESONANCE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        if (school & SPELL_SCHOOL_MASK_FROST)
            caster->CastSpell(target, SPELL_FROST_RESONANCE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_wrath_resonance::HandleHit);
    }
};

// 4930 - Mystic Tornado: a player who touches it is launched (192675, which cannot hit twice in 5 s).
struct at_wrath_mystic_tornado : AreaTriggerAI
{
    at_wrath_mystic_tornado(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        if (Unit* caster = at->GetCaster())
            caster->CastSpell(unit, SPELL_MYSTIC_TORNADO_HIT, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }
};

void AddSC_boss_wrath_of_azshara()
{
    RegisterEyeOfAzsharaCreatureAI(boss_wrath_of_azshara);
    RegisterEyeOfAzsharaCreatureAI(npc_wrath_mystic_tornado);
    RegisterEyeOfAzsharaCreatureAI(npc_wrath_arcane_bomb);
    RegisterEyeOfAzsharaCreatureAI(npc_wrath_tidal_wave);
    RegisterSpellScript(spell_wrath_mystic_tornado);
    RegisterSpellScript(spell_wrath_arcane_bomb);
    RegisterSpellScript(spell_wrath_crushing_depths);
    RegisterSpellScript(spell_wrath_resonance);
    RegisterAreaTriggerAI(at_wrath_mystic_tornado);
}
