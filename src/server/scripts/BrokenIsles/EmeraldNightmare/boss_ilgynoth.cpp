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
#include "Containers.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "emerald_nightmare.h"

enum IlgynothSpells
{
    SPELL_STUFF_OF_NIGHTMARES   = 209915, // Eye: permanent, -99% damage taken

    // Dominator Tentacle
    SPELL_NIGHTMARISH_FURY      = 215234,
    SPELL_GROUND_SLAM           = 208689,
    SPELL_RUPTURING_ROAR        = 208685, // 300 yd raid damage, 2 s cast

    // Deathglare Tentacle
    SPELL_MIND_FLAY             = 208697, // 3 s cast, interruptible

    // Corruptor Tentacle
    SPELL_SPEW_CORRUPTION       = 208929, // 10 s cast; its periodic dummy drops the pools
    SPELL_NIGHTMARE_CORRUPTION  = 208931, // creates area trigger 6514 at a destination
    SPELL_NIGHTMARE_CORRUPTION_DAMAGE = 212886, // applied to units standing in a pool

    // Nightmare Horror
    SPELL_EYE_OF_FATE           = 210984,
    SPELL_SEEPING_CORRUPTION    = 209387, // periodic, drops pools (212855 -> 208931)
    SPELL_HORROR_EMERGE         = 210289,

    // Every add
    SPELL_SUMMON_ICHOR          = 209966, // not cast: see SummonIchors

    // Eye
    SPELL_NIGHTMARE_GAZE        = 210931,

    // Nightmare Ichor
    SPELL_TOUCH_OF_CORRUPTION   = 209467, // proc aura on the Ichor itself
    SPELL_FIXATE                = 210099,
    SPELL_NIGHTMARE_EXPLOSION   = 209471, // 2.5 s cast

    // Heart
    SPELL_REABSORPTION          = 212942, // an Ichor reached the chamber: raid damage from the heart
    SPELL_DARK_RECONSTITUTION   = 210781, // 50 s cast; kills what is left in the chamber
    SPELL_FINAL_TORPOR          = 223121, // 90 s cast; kills everyone
    SPELL_CURSED_BLOOD          = 215128, // 8 s DoT, 300 yd; capped to a few players by the script
    SPELL_CURSED_BLOOD_BURST    = 215143  // 11 yd, when Cursed Blood runs out
};

enum IlgynothEvents
{
    // Heart, PHASE_HEART
    EVENT_CURSED_BLOOD = 1,
    EVENT_RECONSTITUTION_WARN,
    EVENT_TORPOR_WARN,

    // Heart, PHASE_OUTSIDE
    EVENT_WAVE_DEATHGLARE,
    EVENT_WAVE_CORRUPTOR,
    EVENT_WAVE_HORROR,
    EVENT_RANDOM_TENTACLE,

    // Dominator
    EVENT_FURY,
    EVENT_GROUND_SLAM,

    // Deathglare
    EVENT_MIND_FLAY,

    // Corruptor
    EVENT_SPEW_CORRUPTION,

    // Horror
    EVENT_EYE_OF_FATE,

    // Eye
    EVENT_NIGHTMARE_GAZE
};

enum IlgynothTalks
{
    // Il'gynoth
    TALK_AGGRO = 0,
    TALK_HEART_PHASE,
    TALK_RECONSTITUTED,
    TALK_LOW_HEALTH,
    TALK_DEATH,
    TALK_RECONSTITUTION_WARN,
    TALK_TORPOR_WARN,

    // Eye of Il'gynoth
    TALK_EYE_HORROR = 0,

    // Dominator Tentacle
    TALK_GROUND_SLAM = 0
};

enum IlgynothActions
{
    ACTION_PULL         = 1, // the Eye was engaged: engage the heart
    ACTION_EYE_DIED     = 2, // the Eye died: start the heart phase
    ACTION_ICHOR_INSIDE = 3  // an Ichor came into the chamber: Reabsorption
};

enum IlgynothPhases
{
    PHASE_OUTSIDE = 1,
    PHASE_HEART
};

// The chamber behind the hollow, measured in game on 2026-10-18. A unit is
// inside when it is behind the mouth (the Eye looks out of the hollow along
// MouthFacing, the chamber lies the other way), within ChamberRadius of the
// heart on the floor plane, and within ChamberHeight of the heart's height.
Position const MouthPosition(-12816.817f, 12329.056f, -246.30473f);
Position const HeartPosition(-12793.001f, 12352.525f, -244.93068f);
Position const EyePosition(-12821.085f, 12325.66f, -247.18068f, 3.730643f);
float const MouthFacing     = 3.730643f;
float const ChamberRadius   = 45.0f;
float const ChamberHeight   = 15.0f;

bool IsInsideChamber(Position const& obj)
{
    float const dx = obj.GetPositionX() - MouthPosition.GetPositionX();
    float const dy = obj.GetPositionY() - MouthPosition.GetPositionY();
    if (dx * std::cos(MouthFacing) + dy * std::sin(MouthFacing) >= 0.0f)
        return false;

    if (obj.GetExactDist2d(HeartPosition) > ChamberRadius)
        return false;

    return std::fabs(obj.GetPositionZ() - HeartPosition.GetPositionZ()) <= ChamberHeight;
}

// The outside phase's adds. Times are seconds from the start of the phase, from
// DBM 7.3.31 Ilgynoth.lua (phase1EasyDeathglares {26,62,85,55} from 21.5;
// phase1EasyCorruptors {86,95,35}: 79, then +95, +35; phase2ComboDeathglares
// {21.5,90,130}; phase2Corruptors {45,95,35,85,40}; Horror 60 / 95, then every 280),
// counts from Icy Veins.
struct WaveList
{
    std::vector<Milliseconds> times;
    std::vector<uint8> counts;
};

static WaveList const DeathglareFirst   = { { 21500ms, 83500ms, 168500ms, 223500ms }, { 1, 2, 1, 1 } };
static WaveList const DeathglareAfter   = { { 21500ms, 111500ms, 241500ms }, { 1, 2, 1 } };
static WaveList const CorruptorFirst    = { { 79s, 174s, 209s }, { 2, 2, 1 } };
static WaveList const CorruptorAfter    = { { 45s, 140s, 175s, 260s, 300s }, { 2, 2, 1, 1, 1 } };

Milliseconds const HorrorFirst          = 60s;
Milliseconds const HorrorAfter          = 95s;
Milliseconds const HorrorInterval       = 280s;
Milliseconds const RandomTentacleTime   = 60s;

// The heart phase. The warnings come 5 s before the cast ends (DBM 7.3.31).
Milliseconds const CursedBloodInterval  = 15s;
Milliseconds const ReconstitutionWarn   = 45s;
Milliseconds const TorporWarn           = 85s;

// Nightmare Explosion reaches 10 yd (the spell's radius index 13), not the plan's 13.
float const ExplosionRange          = 10.0f;
uint32 const ExplosionsToKill       = 20; // each takes 5% of the Eye
Milliseconds const GazeMin          = 2s;
Milliseconds const GazeMax          = 3s;

Position const DominatorPositions[2] =
{
    { -12848.881f, 12332.325f, -249.96867f, 3.730643f },
    { -12814.461f, 12288.635f, -248.5229f, 3.730643f }
};
Position const HorrorPosition(-12833.5f, 12317.3f, -248.5f, 3.730643f);
float const TentacleRingRadius = 30.0f;

// Looks for the pools of Nightmare Corruption around a point.
struct NightmarePoolCheck
{
    bool operator()(AreaTrigger* at) const { return at->GetSpellId() == SPELL_NIGHTMARE_CORRUPTION; }
};

// Every add leaves Ichors behind when it dies. They are summoned through the heart, not
// cast with 209966: a TempSummon made by the spell belongs to the add (the summoner's
// AI gets JustSummoned), so the heart's summon list would not hold it, and an Ichor
// could survive a wipe. The heart's list is despawned by CleanUp.
static void SummonIchors(Creature* me, uint8 count)
{
    InstanceScript* instance = me->GetInstanceScript();
    Creature* heart = instance ? instance->GetCreature(DATA_ILGYNOTH) : nullptr;
    if (!heart || !heart->IsAlive() || !heart->IsEngaged())
        return;

    for (uint8 i = 0; i < count; ++i)
    {
        Position pos = me->GetPosition();
        me->MovePosition(pos, frand(0.0f, 3.0f), frand(0.0f, 2.0f * float(M_PI)));
        heart->SummonCreature(NPC_NIGHTMARE_ICHOR, pos, TEMPSUMMON_MANUAL_DESPAWN);
    }
}

// 105393 - Il'gynoth, the Heart of Corruption
struct boss_ilgynoth : public BossAI
{
    boss_ilgynoth(Creature* creature) : BossAI(creature, DATA_ILGYNOTH), _heartPhase(false), _evading(false), _afterHeart(false),
        _heartPhases(0), _lowHealthSaid(false), _deathglareIndex(0), _corruptorIndex(0) { }

    void JustAppeared() override
    {
        Setup();
        SetForcefield(false);
        SummonEye();
    }

    void Reset() override
    {
        BossAI::Reset();
        Setup();
        SummonEye();
    }

    // The Eye is a summon of the heart, so evade takes it away with the rest
    // and the heart's reset brings a fresh one. Does nothing if one is alive.
    void SummonEye()
    {
        // A cleared instance reloads with a dead heart: no Eye then.
        if (!me->IsAlive() || instance->GetBossState(DATA_ILGYNOTH) == DONE)
            return;

        Creature* eye = instance->GetCreature(DATA_EYE_OF_ILGYNOTH);
        if (eye && eye->IsAlive())
            return;

        me->SummonCreature(NPC_EYE_OF_ILGYNOTH, EyePosition, TEMPSUMMON_MANUAL_DESPAWN);
    }

    void JustSummoned(Creature* summon) override
    {
        // Not BossAI::JustSummoned: the Eye engages by being hit or approached.
        summons.Summon(summon);
    }

    // The heart is rooted and cannot be selected until the Eye is dead.
    void Setup()
    {
        _heartPhase = false;
        _heartPhases = 0;
        _lowHealthSaid = false;
        me->SetControlled(true, UNIT_STATE_ROOT);
        // Passive: the heart never picks a victim to melee or turn to, yet still evades
        // when its threat list runs empty.
        me->SetReactState(REACT_PASSIVE);
        // Like a raid boss: a stun or an interrupt would cancel the long cast and
        // the encounter could not end.
        for (Mechanics mechanic : { MECHANIC_STUN, MECHANIC_FEAR, MECHANIC_CHARM, MECHANIC_KNOCKOUT, MECHANIC_SAPPED,
            MECHANIC_DISORIENTED, MECHANIC_BANISH, MECHANIC_SILENCE, MECHANIC_INTERRUPT, MECHANIC_POLYMORPH, MECHANIC_HORROR })
        {
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, mechanic, false);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, mechanic, true);
        }
        me->SetUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
    }

    void SetForcefield(bool open)
    {
        instance->HandleGameObject(instance->GetGuidData(DATA_ILGYNOTH_FORCEFIELD), open);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_PULL:
                if (!me->IsEngaged())
                    DoZoneInCombat();
                break;
            case ACTION_EYE_DIED:
                StartHeartPhase();
                break;
            case ACTION_ICHOR_INSIDE:
                // The heart is the caster, so the raid damage comes from it.
                if (_heartPhase && me->IsAlive())
                    me->CastSpell(me, SPELL_REABSORPTION, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
                break;
            default:
                break;
        }
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        if (Creature* eye = instance->GetCreature(DATA_EYE_OF_ILGYNOTH))
            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, eye, 2);
        Talk(TALK_AGGRO);
        StartOutsidePhase(false);
    }

    // The heart has no victim in this phase, so the engage is kept by pulling
    // the zone again at each start.
    void StartOutsidePhase(bool afterHeart)
    {
        _heartPhase = false;
        _afterHeart = afterHeart;
        _deathglareIndex = 0;
        _corruptorIndex = 0;
        events.Reset();
        events.SetPhase(PHASE_OUTSIDE);
        if (!me->IsEngaged())
            DoZoneInCombat();

        _dominators.clear();
        for (uint8 i = 0; i < 2; ++i)
            if (Creature* dominator = me->SummonCreature(NPC_DOMINATOR_TENTACLE, DominatorPositions[i], TEMPSUMMON_MANUAL_DESPAWN))
            {
                _dominators.push_back(dominator->GetGUID());
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, dominator, 3 + i);
            }

        events.ScheduleEvent(EVENT_WAVE_DEATHGLARE, DeathglareList().times[0], 0, PHASE_OUTSIDE);
        events.ScheduleEvent(EVENT_WAVE_CORRUPTOR, CorruptorList().times[0], 0, PHASE_OUTSIDE);
        events.ScheduleEvent(EVENT_WAVE_HORROR, afterHeart ? HorrorAfter : HorrorFirst, 0, PHASE_OUTSIDE);
    }

    WaveList const& DeathglareList() const { return _afterHeart ? DeathglareAfter : DeathglareFirst; }
    WaveList const& CorruptorList() const { return _afterHeart ? CorruptorAfter : CorruptorFirst; }

    // A random point on the ring around the Eye, outside the chamber, on the ground.
    Position RandomRingPoint() const
    {
        Position pos = EyePosition;
        for (uint8 tries = 0; tries < 10; ++tries)
        {
            float const angle = frand(0.0f, 2.0f * float(M_PI));
            pos.Relocate(EyePosition.GetPositionX() + TentacleRingRadius * std::cos(angle),
                EyePosition.GetPositionY() + TentacleRingRadius * std::sin(angle), EyePosition.GetPositionZ());
            pos.SetOrientation(Position::NormalizeOrientation(angle + float(M_PI)));
            float const z = me->GetMap()->GetHeight(me->GetPhaseShift(), pos.GetPositionX(), pos.GetPositionY(), EyePosition.GetPositionZ() + 5.0f, true, 30.0f);
            if (z > INVALID_HEIGHT)
                pos.m_positionZ = z;
            if (!IsInsideChamber(pos))
                break;
        }
        return pos;
    }

    void SummonTentacles(uint32 entry, uint8 count)
    {
        for (uint8 i = 0; i < count; ++i)
            me->SummonCreature(entry, RandomRingPoint(), TEMPSUMMON_MANUAL_DESPAWN);
    }

    Creature* SummonHorror()
    {
        if (Creature* eye = instance->GetCreature(DATA_EYE_OF_ILGYNOTH))
            eye->AI()->Talk(TALK_EYE_HORROR);
        return me->SummonCreature(NPC_NIGHTMARE_HORROR, HorrorPosition, TEMPSUMMON_MANUAL_DESPAWN);
    }

    void ExecuteEvent(uint32 eventId)
    {
        switch (eventId)
        {
            case EVENT_WAVE_DEATHGLARE:
            {
                WaveList const& list = DeathglareList();
                SummonTentacles(NPC_DEATHGLARE_TENTACLE, list.counts[_deathglareIndex]);
                if (++_deathglareIndex < list.times.size())
                    events.ScheduleEvent(EVENT_WAVE_DEATHGLARE, list.times[_deathglareIndex] - list.times[_deathglareIndex - 1], 0, PHASE_OUTSIDE);
                else
                    TryStartRandomTentacles();
                break;
            }
            case EVENT_WAVE_CORRUPTOR:
            {
                WaveList const& list = CorruptorList();
                SummonTentacles(NPC_CORRUPTOR_TENTACLE, list.counts[_corruptorIndex]);
                if (++_corruptorIndex < list.times.size())
                    events.ScheduleEvent(EVENT_WAVE_CORRUPTOR, list.times[_corruptorIndex] - list.times[_corruptorIndex - 1], 0, PHASE_OUTSIDE);
                else
                    TryStartRandomTentacles();
                break;
            }
            case EVENT_WAVE_HORROR:
                SummonHorror();
                events.Repeat(HorrorInterval);
                break;
            case EVENT_RANDOM_TENTACLE:
                SummonTentacles(urand(0, 1) ? NPC_DEATHGLARE_TENTACLE : NPC_CORRUPTOR_TENTACLE, 1);
                events.Repeat(RandomTentacleTime);
                break;
            case EVENT_CURSED_BLOOD:
                // The spell's script picks the targets; with nobody inside it hits nobody.
                me->CastSpell(me, SPELL_CURSED_BLOOD, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
                events.Repeat(CursedBloodInterval);
                break;
            case EVENT_RECONSTITUTION_WARN:
                Talk(TALK_RECONSTITUTION_WARN);
                break;
            case EVENT_TORPOR_WARN:
                Talk(TALK_TORPOR_WARN);
                break;
            default:
                break;
        }
    }

    // Once both tentacle lists are used up, one random tentacle comes every minute.
    void TryStartRandomTentacles()
    {
        if (_deathglareIndex >= DeathglareList().times.size() && _corruptorIndex >= CorruptorList().times.size())
            events.ScheduleEvent(EVENT_RANDOM_TENTACLE, RandomTentacleTime, 0, PHASE_OUTSIDE);
    }

    bool IsHeartPhase() const { return _heartPhase; }

    // The first heart phase casts Dark Reconstitution, the second Final Torpor.
    void StartHeartPhase()
    {
        if (_heartPhase)
            return;

        _heartPhase = true;
        ++_heartPhases;
        events.Reset();
        events.SetPhase(PHASE_HEART);
        SetForcefield(true);
        me->RemoveUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
        Talk(TALK_HEART_PHASE);
        // Nobody has threat on the heart yet, and UpdateVictim would evade.
        DoZoneInCombat();

        if (_heartPhases == 1)
        {
            DoCastSelf(SPELL_DARK_RECONSTITUTION);
            events.ScheduleEvent(EVENT_RECONSTITUTION_WARN, ReconstitutionWarn, 0, PHASE_HEART);
        }
        else
        {
            DoCastSelf(SPELL_FINAL_TORPOR);
            events.ScheduleEvent(EVENT_TORPOR_WARN, TorporWarn, 0, PHASE_HEART);
        }

        events.ScheduleEvent(EVENT_CURSED_BLOOD, CursedBloodInterval, 0, PHASE_HEART);
    }

    // The cast ended. Dark Reconstitution's kill is the spell's own; this closes the
    // chamber and starts the outside phase again. Final Torpor needs nothing: it kills
    // everyone and the wipe evades the heart.
    void OnSpellCast(SpellInfo const* spell) override
    {
        if (spell->Id != SPELL_DARK_RECONSTITUTION || !_heartPhase)
            return;

        Talk(TALK_RECONSTITUTED);
        SetForcefield(false);
        me->SetUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
        // The Eye comes back at full health; the adds that are alive stay.
        SummonEye();
        StartOutsidePhase(true);
    }

    // Fallback if the long cast is lost anyway (an effect the immunities do not cover):
    // cast it again. The remaining time restarts, which is better than a soft lock.
    void OnSpellFailed(SpellInfo const* spell) override
    {
        if (!_heartPhase || !me->IsAlive())
            return;

        if (spell->Id == SPELL_DARK_RECONSTITUTION || spell->Id == SPELL_FINAL_TORPOR)
            DoCastSelf(spell->Id);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (_lowHealthSaid)
            return;

        if (int64(me->GetHealth()) - int64(damage) <= int64(me->CountPctFromMaxHealth(30)))
        {
            _lowHealthSaid = true;
            Talk(TALK_LOW_HEALTH);
        }
    }

    void CleanUp()
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        for (ObjectGuid const& guid : _dominators)
            if (Creature* dominator = ObjectAccessor::GetCreature(*me, guid))
                instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, dominator);
        _dominators.clear();
        if (Creature* eye = instance->GetCreature(DATA_EYE_OF_ILGYNOTH))
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, eye);
        summons.DespawnAll();
        me->RemoveAllAreaTriggers();
        RemovePools();
        // A plain removal does not burst.
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CURSED_BLOOD);
    }

    // Pools belong to the adds that cast them and go with them; this catches stragglers.
    void RemovePools()
    {
        std::list<AreaTrigger*> pools;
        NightmarePoolCheck check;
        Trinity::AreaTriggerListSearcher<NightmarePoolCheck> searcher(me, pools, check);
        Cell::VisitAllObjects(EyePosition.GetPositionX(), EyePosition.GetPositionY(), me->GetMap(), searcher, 100.0f);
        for (AreaTrigger* pool : pools)
            pool->Remove();
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        // The Eye's evade calls this one, and this one evades the Eye.
        if (_evading || !me->IsEngaged())
            return;
        _evading = true;

        _heartPhase = false; // so the interrupted cast is not cast again
        me->InterruptNonMeleeSpells(false);
        CleanUp(); // despawns the Eye with the other summons
        SetForcefield(false);

        _heartPhase = false;
        _EnterEvadeMode();
        _DespawnAtEvade();
        _evading = false;
    }

    void JustDied(Unit* /*killer*/) override
    {
        // A cast in progress must not finish: Dark Reconstitution and Final Torpor kill.
        me->InterruptNonMeleeSpells(false);
        CleanUp();
        SetForcefield(true);
        Talk(TALK_DEATH);
        _JustDied();
    }

    void UpdateAI(uint32 diff) override
    {
        // No victim in the outside phase: the heart must not evade from that.
        if (_heartPhase && !UpdateVictim())
            return;

        scheduler.Update(diff);
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
            ExecuteEvent(eventId);
    }

private:
    bool _heartPhase;
    bool _evading;
    bool _afterHeart;
    uint8 _heartPhases;
    bool _lowHealthSaid;
    uint8 _deathglareIndex;
    uint8 _corruptorIndex;
    GuidVector _dominators;
};

// 105906 - Eye of Il'gynoth
struct npc_eye_of_ilgynoth : public ScriptedAI
{
    npc_eye_of_ilgynoth(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()) { }

    void JustAppeared() override
    {
        Setup();
    }

    void Reset() override
    {
        Setup();
    }

    void Setup()
    {
        _dying = false;
        _explosions = 0;
        _events.Reset();
        me->SetControlled(true, UNIT_STATE_ROOT);
        if (!me->HasAura(SPELL_STUFF_OF_NIGHTMARES))
            DoCastSelf(SPELL_STUFF_OF_NIGHTMARES);
    }

    // An Ichor exploded next to the Eye: 5% of its health, set directly because Stuff of
    // Nightmares would cut a damage hit to 1%. The Eye is killed by the Ichor, once.
    void TakeExplosion(Unit* ichor)
    {
        if (_dying || !me->IsAlive())
            return;

        // Counted, not computed from the health: a floored 5% would leave the Eye alive
        // on the 20th hit by the rounding remainder.
        if (++_explosions < ExplosionsToKill)
        {
            uint64 const max = me->GetMaxHealth();
            me->SetHealth(max - max * _explosions / ExplosionsToKill);
            return;
        }

        _dying = true;
        Unit::Kill(ichor, me);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_NIGHTMARE_GAZE, GazeMin, GazeMax);
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 2);
        if (Creature* heart = _instance->GetCreature(DATA_ILGYNOTH))
            heart->AI()->DoAction(ACTION_PULL);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* heart = _instance->GetCreature(DATA_ILGYNOTH))
            heart->AI()->DoAction(ACTION_EYE_DIED);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        _events.Reset();
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        if (Creature* heart = _instance->GetCreature(DATA_ILGYNOTH))
            if (heart->IsEngaged())
            {
                // The heart's evade despawns this Eye; a fresh one comes back.
                heart->AI()->EnterEvadeMode();
                return;
            }

        ScriptedAI::EnterEvadeMode(why);
        me->SetFullHealth();
        Setup();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
            if (eventId == EVENT_NIGHTMARE_GAZE)
            {
                if (Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true))
                    DoCast(target, SPELL_NIGHTMARE_GAZE);
                _events.Repeat(GazeMin, GazeMax);
                break;
            }
    }

private:
    InstanceScript* _instance;
    EventMap _events;
    bool _dying = false;
    uint32 _explosions = 0;
};

// 105721 - Nightmare Ichor
// Fixates a player in the room, chases, and when it is hurt to 1 HP stops and blows up
// (10 yd). Next to the Eye the blast takes 5% of the Eye's health. In the heart phase an
// Ichor that gets into the chamber is reabsorbed.
struct npc_nightmare_ichor : public ScriptedAI
{
    npc_nightmare_ichor(Creature* creature) : ScriptedAI(creature), _instance(creature->GetInstanceScript()) { }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_TOUCH_OF_CORRUPTION);
        PickTarget();
    }

    bool InRoom(Unit const* unit) const
    {
        CreatureBoundary const* boundary = _instance ? _instance->GetBossBoundary(DATA_ILGYNOTH) : nullptr;
        Position const pos = unit->GetPosition();
        return !boundary || CreatureAI::IsInBounds(*boundary, &pos);
    }

    bool IsGoodTarget(Unit const* unit) const
    {
        return unit && unit->IsAlive() && InRoom(unit);
    }

    void ReleaseTarget()
    {
        if (Unit* old = ObjectAccessor::GetUnit(*me, _fixate))
            old->RemoveAurasDueToSpell(SPELL_FIXATE, me->GetGUID());
        _fixate.Clear();
    }

    // A random living player in the room, or nobody: then the Ichor just stands.
    void PickTarget()
    {
        ReleaseTarget();

        std::vector<Player*> candidates;
        for (MapReference const& ref : me->GetMap()->GetPlayers())
            if (Player* player = ref.GetSource())
                if (!player->IsGameMaster() && IsGoodTarget(player))
                    candidates.push_back(player);

        if (candidates.empty())
        {
            me->AttackStop();
            me->StopMoving();
            return;
        }

        Player* target = Trinity::Containers::SelectRandomContainerElement(candidates);
        _fixate = target->GetGUID();
        DoCast(target, SPELL_FIXATE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        AttackStart(target);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*type*/, SpellInfo const* /*spell*/) override
    {
        if (damage < me->GetHealth() - 1)
            return;

        // Never dies from damage: it is brought to 1 HP, and then blows up.
        damage = me->GetHealth() - 1;
        if (_exploding)
            return;

        _exploding = true;
        ReleaseTarget();
        me->AttackStop();
        me->StopMoving();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveIdle();
    }

    void OnSpellCast(SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_NIGHTMARE_EXPLOSION)
            _exploded = true; // resolved in UpdateAI, not inside the spell's own cast
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (!me->IsAlive())
            return;

        if (_exploded)
        {
            _exploded = false;
            if (Creature* eye = _instance->GetCreature(DATA_EYE_OF_ILGYNOTH))
                if (eye->IsAlive() && me->IsWithinDist(eye, ExplosionRange))
                    if (npc_eye_of_ilgynoth* eyeAI = CAST_AI(npc_eye_of_ilgynoth, eye->AI()))
                        eyeAI->TakeExplosion(me);
            if (me->IsAlive())
                me->KillSelf();
            return;
        }

        if (_exploding)
        {
            // Starts the cast, and again if it ever failed to start.
            if (!me->HasUnitState(UNIT_STATE_CASTING))
                DoCastSelf(SPELL_NIGHTMARE_EXPLOSION);
            return;
        }

        if (Creature* heart = _instance->GetCreature(DATA_ILGYNOTH))
            if (heart->IsAlive() && IsInsideChamber(*me))
                if (boss_ilgynoth* heartAI = CAST_AI(boss_ilgynoth, heart->AI()))
                    if (heartAI->IsHeartPhase())
                    {
                        heart->AI()->DoAction(ACTION_ICHOR_INSIDE);
                        ReleaseTarget();
                        me->DespawnOrUnsummon();
                        return;
                    }

        if (!IsGoodTarget(ObjectAccessor::GetUnit(*me, _fixate)))
            PickTarget();
    }

private:
    InstanceScript* _instance;
    ObjectGuid _fixate;
    bool _exploding = false;
    bool _exploded = false;
};

// The adds are rooted (the Horror aside), engage everyone in the zone as they appear
// and pick random players for their casts. A null target skips the cast for a second.
struct ilgynoth_add_ai : public ScriptedAI
{
    ilgynoth_add_ai(Creature* creature, bool rooted) : ScriptedAI(creature), _instance(creature->GetInstanceScript()), _rooted(rooted) { }

    void JustAppeared() override
    {
        if (_rooted)
            me->SetControlled(true, UNIT_STATE_ROOT);
        DoZoneInCombat();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // Nobody left to fight: the whole encounter resets (it also despawns this add).
        // Anything else (no path, boundary) is no reason to reset: the add just stays.
        if (why == EvadeReason::NoHostiles)
            if (Creature* heart = _instance->GetCreature(DATA_ILGYNOTH))
                if (heart->IsEngaged())
                    heart->AI()->EnterEvadeMode();
    }

    Unit* RandomPlayer() { return SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true); }

protected:
    InstanceScript* _instance;
    EventMap _events;

private:
    bool _rooted;
};

// 105304 - Dominator Tentacle
struct npc_dominator_tentacle : public ilgynoth_add_ai
{
    static constexpr uint8 ICHORS = 2;

    npc_dominator_tentacle(Creature* creature) : ilgynoth_add_ai(creature, true) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_FURY, 6s);
        _events.ScheduleEvent(EVENT_GROUND_SLAM, 12s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        SummonIchors(me, ICHORS);
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
                case EVENT_FURY:
                    DoCastVictim(SPELL_NIGHTMARISH_FURY);
                    _events.Repeat(10900ms);
                    break;
                case EVENT_GROUND_SLAM:
                    if (Unit* target = RandomPlayer())
                    {
                        Talk(TALK_GROUND_SLAM, target);
                        DoCast(target, SPELL_GROUND_SLAM);
                        _events.Repeat(20500ms);
                    }
                    else
                        _events.Repeat(1s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }

        // Nothing else was due: with nobody in reach the roar goes out back to back. It is
        // checked here, not scheduled, so that a due Fury or Slam always goes first.
        bool const anyInReach = SelectTarget(SelectTargetMethod::MaxThreat, 0, [this](Unit* unit)
        {
            return me->IsWithinMeleeRange(unit);
        }) != nullptr;
        if (!anyInReach)
            DoCastAOE(SPELL_RUPTURING_ROAR);
    }
};

// 105322 - Deathglare Tentacle
struct npc_deathglare_tentacle : public ilgynoth_add_ai
{
    static constexpr uint8 ICHORS = 2;

    npc_deathglare_tentacle(Creature* creature) : ilgynoth_add_ai(creature, true) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_MIND_FLAY, 1s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        SummonIchors(me, ICHORS);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        // Recast as soon as the last cast ends or is interrupted.
        while (uint32 eventId = _events.ExecuteEvent())
            if (eventId == EVENT_MIND_FLAY)
            {
                if (Unit* target = RandomPlayer())
                    DoCast(target, SPELL_MIND_FLAY);
                _events.Repeat(500ms);
                break;
            }
    }
};

// 105383 - Corruptor Tentacle
struct npc_ilgynoth_corruptor_tentacle : public ilgynoth_add_ai
{
    static constexpr uint8 ICHORS = 1;

    npc_ilgynoth_corruptor_tentacle(Creature* creature) : ilgynoth_add_ai(creature, true) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_SPEW_CORRUPTION, 1s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        SummonIchors(me, ICHORS);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        // Back to back, on one player: someone who does not carry the Spew yet if there is one.
        while (uint32 eventId = _events.ExecuteEvent())
            if (eventId == EVENT_SPEW_CORRUPTION)
            {
                Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true, true, -int32(SPELL_SPEW_CORRUPTION));
                if (!target)
                    target = RandomPlayer();
                if (target && DoCast(target, SPELL_SPEW_CORRUPTION) == SPELL_CAST_OK)
                    _events.Repeat(500ms);
                else
                    _events.Repeat(1s);
                break;
            }
    }
};

// 105591 - Nightmare Horror
struct npc_nightmare_horror : public ilgynoth_add_ai
{
    static constexpr uint8 ICHORS = 4;

    npc_nightmare_horror(Creature* creature) : ilgynoth_add_ai(creature, false) { }

    void JustAppeared() override
    {
        ilgynoth_add_ai::JustAppeared();
        DoCastSelf(SPELL_HORROR_EMERGE);
        DoCastSelf(SPELL_SEEPING_CORRUPTION);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_EYE_OF_FATE, 14s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        SummonIchors(me, ICHORS);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
            if (eventId == EVENT_EYE_OF_FATE)
            {
                DoCastVictim(SPELL_EYE_OF_FATE);
                _events.Repeat(10s);
                break;
            }
    }
};

// 208929 - Spew Corruption: every tick the Corruptor spits a pool where the victim stands.
class spell_ilgynoth_spew_corruption : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_NIGHTMARE_CORRUPTION });
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();

        // The pool belongs to the Corruptor, so it goes away with the add.
        if (Unit* caster = GetCaster())
            caster->CastSpell(GetTarget()->GetPosition(), SPELL_NIGHTMARE_CORRUPTION, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_ilgynoth_spew_corruption::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
    }
};

// 210781 - Dark Reconstitution: only what stands in the chamber is killed.
class spell_ilgynoth_dark_reconstitution : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target) { return !IsInsideChamber(*target); });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ilgynoth_dark_reconstitution::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
    }
};

// 215128 - Cursed Blood: players inside the chamber only; one if a single player is
// there, else players / 5 clamped to 2..6.
class spell_ilgynoth_cursed_blood : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        // The area is shared by the aura's effects; they must all get the same players.
        if (_chosen)
        {
            targets.remove_if([this](WorldObject* target) { return !_picked.contains(target->GetGUID()); });
            return;
        }

        _chosen = true;
        targets.remove_if([](WorldObject* target) { return !target->IsPlayer() || !IsInsideChamber(*target); });

        // The cap follows the raid's size (GMs count), not who stands inside.
        size_t raid = 0;
        for (MapReference const& ref : GetCaster()->GetMap()->GetPlayers())
            if (ref.GetSource())
                ++raid;
        size_t const cap = raid <= 1 ? 1 : std::clamp<size_t>(raid / 5, 2, 6);
        if (targets.size() > cap)
            Trinity::Containers::RandomResize(targets, cap);

        for (WorldObject* target : targets)
            _picked.insert(target->GetGUID());
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ilgynoth_cursed_blood::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_ilgynoth_cursed_blood::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
    }

    bool _chosen = false;
    GuidUnorderedSet _picked;
};

// 215128 - Cursed Blood: when it runs out, not when it is removed, the target bursts.
class spell_ilgynoth_cursed_blood_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CURSED_BLOOD_BURST });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        GetTarget()->CastSpell(GetTarget(), SPELL_CURSED_BLOOD_BURST, CastSpellExtraArgs(TRIGGERED_FULL_MASK).SetOriginalCaster(GetCasterGUID()));
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_ilgynoth_cursed_blood_aura::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
    }
};

// 6514 - Nightmare Corruption (created by 208931)
struct at_ilgynoth_nightmare_corruption : AreaTriggerAI
{
    at_ilgynoth_nightmare_corruption(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        if (Unit* caster = at->GetCaster())
            caster->CastSpell(unit, SPELL_NIGHTMARE_CORRUPTION_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void OnUnitExit(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        // Pools overlap, and belong to different adds: keep the effect while the unit
        // still stands in another one.
        std::list<AreaTrigger*> pools;
        NightmarePoolCheck check;
        Trinity::AreaTriggerListSearcher<NightmarePoolCheck> searcher(at, pools, check);
        Cell::VisitAllObjects(at, searcher, 15.0f);
        for (AreaTrigger* other : pools)
            if (other != at && other->GetInsideUnits().contains(unit->GetGUID()))
                return;

        unit->RemoveAurasDueToSpell(SPELL_NIGHTMARE_CORRUPTION_DAMAGE);
    }
};

void AddSC_boss_ilgynoth()
{
    RegisterEmeraldNightmareCreatureAI(boss_ilgynoth);
    RegisterEmeraldNightmareCreatureAI(npc_eye_of_ilgynoth);
    RegisterEmeraldNightmareCreatureAI(npc_nightmare_ichor);
    RegisterEmeraldNightmareCreatureAI(npc_dominator_tentacle);
    RegisterEmeraldNightmareCreatureAI(npc_deathglare_tentacle);
    RegisterEmeraldNightmareCreatureAI(npc_ilgynoth_corruptor_tentacle);
    RegisterEmeraldNightmareCreatureAI(npc_nightmare_horror);
    RegisterSpellScript(spell_ilgynoth_spew_corruption);
    RegisterSpellScript(spell_ilgynoth_dark_reconstitution);
    RegisterSpellScript(spell_ilgynoth_cursed_blood);
    RegisterSpellScript(spell_ilgynoth_cursed_blood_aura);
    RegisterAreaTriggerAI(at_ilgynoth_nightmare_corruption);
}
