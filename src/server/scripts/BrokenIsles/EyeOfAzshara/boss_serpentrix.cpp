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
#include "SpellScript.h"
#include "eye_of_azshara.h"
#include <cmath>

enum SerpentrixSpells
{
    SPELL_TOXIC_WOUND                   = 191855,   // a player drops Toxic Puddles (191856, area trigger 4856)
    SPELL_TOXIC_PUDDLE_DAMAGE           = 191858,
    SPELL_POISON_SPIT                   = 192050,   // up to 3 players; its dummy casts 191839 at each
    SPELL_POISON_SPIT_TARGET            = 191839,   // script effect: one of the three missiles below
    SPELL_POISON_SPIT_MISSILE_FAST      = 191845,   // 0.75 s: the target is upwind
    SPELL_POISON_SPIT_MISSILE           = 191841,   // 2 s: no wind
    SPELL_POISON_SPIT_MISSILE_SLOW      = 191843,   // 3.5 s: the target is downwind
    SPELL_VIOLENT_WINDS                 = 191797,
    SPELL_SUBMERGE                      = 191873,
    SPELL_RAMPAGE                       = 191848,   // ticks 191850 every 0.5 s
    SPELL_RAMPAGE_SPIT                  = 191850,
    SPELL_BLAZING_NOVA                  = 192003,
    SPELL_ARCANE_BLAST                  = 192005
};

enum SerpentrixTalks
{
    EMOTE_SUBMERGE                      = 0     // 99006, with |Hspell:191873|h for DBM
};

enum SerpentrixEvents
{
    EVENT_TOXIC_WOUND                   = 1,
    EVENT_VIOLENT_WINDS,
    EVENT_POISON_SPIT,
    EVENT_RAMPAGE_CHECK,
    EVENT_EMERGE,

    EVENT_BLAZING_NOVA,
    EVENT_ARCANE_BLAST
};

enum SerpentrixMisc
{
    ACTION_SUBMERGE                     = 1
};

namespace
{
    // Calibrate: the wind's push (the core's movement force; Elerethe's clouds use the same packet).
    float constexpr WindMagnitude = 3.0f;
    // Seconds with no player in melee reach before Rampage starts. Calibrate.
    uint32 constexpr RampageDelaySeconds = 3;

    bool IsNormalDifficulty(Unit const* unit)
    {
        return unit->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL;
    }

    ObjectGuid WindForceId(Unit const* caster)
    {
        return ObjectGuid::Create<HighGuid::AreaTrigger>(caster->GetMapId(), SPELL_VIOLENT_WINDS, caster->GetGUID().GetCounter());
    }
}

// 91808 - Serpentrix
// Timers follow DBM 7.3.31 (DBM-Party-Legion/EyeOfAzshara/Serpentrix.lua): Toxic Wound 5 s after the pull and
// every 15 s (on its cast success), Violent Winds 33 s and every 30 s. Submerge at 66% and 33%.
struct boss_serpentrix : public BossAI
{
    boss_serpentrix(Creature* creature) : BossAI(creature, DATA_SERPENTRIX) { }

    void Reset() override
    {
        _Reset();
        // He never walks; Rampage answers a group that stays out of reach.
        me->SetControlled(true, UNIT_STATE_ROOT);
        _submerges = 0;
        _outOfReach = 0;
        _windAngle = 0.0f;
        _windActive = false;
        if (_home.GetPositionX() == 0.0f)
            _home = me->GetHomePosition();
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);

        events.ScheduleEvent(EVENT_TOXIC_WOUND, 5s);
        events.ScheduleEvent(EVENT_VIOLENT_WINDS, 33s);
        events.ScheduleEvent(EVENT_POISON_SPIT, 6s);  // calibrate: no DBM timer
        events.ScheduleEvent(EVENT_RAMPAGE_CHECK, 1s);
    }

    // The wind's direction, in degrees, for Violent Winds' aura script.
    uint32 GetData(uint32 /*type*/) const override
    {
        return uint32(_windAngle * 180.0f / float(M_PI));
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if ((_submerges == 0 && me->HealthBelowPctDamaged(66, damage)) || (_submerges == 1 && me->HealthBelowPctDamaged(33, damage)))
        {
            ++_submerges;
            events.ScheduleEvent(EVENT_EMERGE, 0s);
        }
    }

    void CleanUp()
    {
        me->RemoveAllAreaTriggers();
        me->RemoveAurasDueToSpell(SPELL_RAMPAGE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TOXIC_WOUND);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VIOLENT_WINDS);
        ObjectGuid const id = WindForceId(me);
        me->GetMap()->DoOnPlayers([id](Player* player) { player->RemoveMovementForce(id); });
        // Toxic Puddles are the marked players' area triggers.
        me->GetMap()->DoOnPlayers([](Player* player)
        {
            for (AreaTrigger* at : player->GetAreaTriggers(191856))
                at->Remove();
        });
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanUp();
        // Back to the first spot before the respawn.
        me->NearTeleportTo(_home);
        _EnterEvadeMode();
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
        if (spell->Id == SPELL_SUBMERGE)
            Emerge();
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
                case EVENT_TOXIC_WOUND:
                    DoCastSelf(SPELL_TOXIC_WOUND);
                    events.Repeat(15s);
                    break;
                case EVENT_VIOLENT_WINDS:
                    StartWinds();
                    events.Repeat(30s);
                    break;
                case EVENT_POISON_SPIT:
                    if (!me->HasAura(SPELL_RAMPAGE))
                        DoCastSelf(SPELL_POISON_SPIT);
                    events.Repeat(6s);
                    break;
                case EVENT_RAMPAGE_CHECK:
                    CheckReach();
                    events.Repeat(1s);
                    break;
                case EVENT_EMERGE:
                    Talk(EMOTE_SUBMERGE);
                    DoCastSelf(SPELL_SUBMERGE);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    // Each player gets the 30 s wind aura; its aura script pushes them along _windAngle.
    void StartWinds()
    {
        _windAngle = frand(0.0f, 2.0f * float(M_PI));
        _windActive = true;
        me->GetMap()->DoOnPlayers([this](Player* player)
        {
            if (player->IsAlive() && me->IsWithinDistInMap(player, 150.0f))
                me->AddAura(SPELL_VIOLENT_WINDS, player);
        });
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
            me->RemoveAurasDueToSpell(SPELL_RAMPAGE);
            return;
        }

        if (++_outOfReach >= RampageDelaySeconds && !me->HasAura(SPELL_RAMPAGE))
            DoCastSelf(SPELL_RAMPAGE);
    }

    // Submerge's teleport goes to a Sinkhole (spell_target_position); two Hydra Spawns come up beside him.
    // Normal: two Blazing. Heroic and Mythic: one Blazing, one Arcane (journal mask 2). Calibrate the mix.
    void Emerge()
    {
        _outOfReach = 0;
        bool const normal = IsNormalDifficulty(me);
        for (uint8 i = 0; i < 2; ++i)
        {
            uint32 const entry = (normal || i == 0) ? NPC_BLAZING_HYDRA_SPAWN : NPC_ARCANE_HYDRA_SPAWN;
            Position pos = me->GetPosition();
            me->MovePosition(pos, 6.0f, float(M_PI) / 2.0f + float(M_PI) * i);
            me->SummonCreature(entry, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10s);
        }
    }

    Position _home;
    uint8 _submerges = 0;
    uint32 _outOfReach = 0;
    float _windAngle = 0.0f;
    bool _windActive = false;
};

// 97259 - Blazing Hydra Spawn, 97260 - Arcane Hydra Spawn. Rooted heads; no DBM timers (calibrate).
struct npc_serpentrix_hydra_spawn : public ScriptedAI
{
    npc_serpentrix_hydra_spawn(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetControlled(true, UNIT_STATE_ROOT);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        if (me->GetEntry() == NPC_BLAZING_HYDRA_SPAWN)
            _events.ScheduleEvent(EVENT_BLAZING_NOVA, 3s);
        else
            _events.ScheduleEvent(EVENT_ARCANE_BLAST, 2s);
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
                case EVENT_BLAZING_NOVA:
                    DoCastSelf(SPELL_BLAZING_NOVA);
                    _events.Repeat(8s);
                    break;
                case EVENT_ARCANE_BLAST:
                    if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 100.0f, true))
                        DoCast(target, SPELL_ARCANE_BLAST);
                    _events.Repeat(4s);
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

// 192050 - Poison Spit: 191839 at each of its targets.
class spell_serpentrix_poison_spit : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_POISON_SPIT_TARGET });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, SPELL_POISON_SPIT_TARGET, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_serpentrix_poison_spit::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 191839 - Poison Spit (script effect), 191850 - Rampage (script effect): one of three missiles, all landing
// 191847. The data gives them travel times of 0.75, 2 and 3.5 s; with Violent Winds up, a target downwind of
// Serpentrix gets the slow one (the journal: "stay downwind to avoid it"), one upwind the fast one.
class spell_serpentrix_spit_missile : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_POISON_SPIT_MISSILE, SPELL_POISON_SPIT_MISSILE_FAST, SPELL_POISON_SPIT_MISSILE_SLOW });
    }

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!target)
            return;

        uint32 missile = SPELL_POISON_SPIT_MISSILE;
        if (target->HasAura(SPELL_VIOLENT_WINDS) && caster->IsAIEnabled())
        {
            float const wind = float(caster->GetAI()->GetData(0)) * float(M_PI) / 180.0f;
            float const toTarget = caster->GetAbsoluteAngle(target);
            float const off = std::fabs(Position::NormalizeOrientation(toTarget - wind + float(M_PI)) - float(M_PI));
            missile = off < float(M_PI) / 2.0f ? SPELL_POISON_SPIT_MISSILE_SLOW : SPELL_POISON_SPIT_MISSILE_FAST;
        }

        caster->CastSpell(target, missile, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_serpentrix_spit_missile::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 191797 - Violent Winds (30 s on each player): the push, along the caster's chosen wind direction.
// Serpentrix casts it in his fight, and the Wrath of Azshara in her last phase; both answer GetData(0) with the
// direction in degrees.
class spell_eoa_violent_winds : public AuraScript
{
    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetTarget();
        if (!caster || !caster->IsAIEnabled() || !target->IsPlayer())
            return;

        float const angle = float(caster->GetAI()->GetData(0)) * float(M_PI) / 180.0f;
        target->ApplyMovementForce(WindForceId(caster), target->GetPosition(), WindMagnitude, MovementForceType::SingleDirectional,
            Position(std::cos(angle), std::sin(angle), 0.0f));
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Unit* caster = GetCaster())
            GetTarget()->RemoveMovementForce(WindForceId(caster));
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_eoa_violent_winds::HandleApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_eoa_violent_winds::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 4856 - Toxic Puddle (dropped by a player with Toxic Wound, 120 s): 191858 every second on players inside.
struct at_serpentrix_toxic_puddle : AreaTriggerAI
{
    at_serpentrix_toxic_puddle(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUpdate(uint32 diff) override
    {
        if (_tick > diff)
        {
            _tick -= diff;
            return;
        }
        _tick = 1000;

        ObjectGuid serpentrix;
        if (InstanceScript* instance = at->GetInstanceScript())
            if (Creature* boss = instance->GetCreature(DATA_SERPENTRIX))
                serpentrix = boss->GetGUID();

        for (ObjectGuid const& guid : at->GetInsideUnits())
            if (Player* player = ObjectAccessor::GetPlayer(*at, guid))
                if (player->IsAlive())
                    player->CastSpell(player, SPELL_TOXIC_PUDDLE_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK).SetOriginalCaster(serpentrix));
    }

private:
    uint32 _tick = 0;
};

void AddSC_boss_serpentrix()
{
    RegisterEyeOfAzsharaCreatureAI(boss_serpentrix);
    RegisterEyeOfAzsharaCreatureAI(npc_serpentrix_hydra_spawn);
    RegisterSpellScript(spell_serpentrix_poison_spit);
    RegisterSpellScript(spell_serpentrix_spit_missile);
    RegisterSpellScript(spell_eoa_violent_winds);
    RegisterAreaTriggerAI(at_serpentrix_toxic_puddle);
}
