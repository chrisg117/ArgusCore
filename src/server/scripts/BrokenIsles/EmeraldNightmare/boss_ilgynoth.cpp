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
    SPELL_SUMMON_ICHOR          = 209966
};

enum IlgynothEvents
{
    // Heart, PHASE_OUTSIDE
    EVENT_WAVE_DEATHGLARE = 1,
    EVENT_WAVE_CORRUPTOR,
    EVENT_WAVE_HORROR,
    EVENT_RANDOM_TENTACLE,

    // Dominator
    EVENT_FURY,
    EVENT_GROUND_SLAM,
    EVENT_RUPTURING_ROAR,

    // Deathglare
    EVENT_MIND_FLAY,

    // Corruptor
    EVENT_SPEW_CORRUPTION,

    // Horror
    EVENT_EYE_OF_FATE
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

// Every add leaves Ichors behind when it dies (Task 4 owns what they do).
static void SummonIchors(Creature* me, uint8 count)
{
    for (uint8 i = 0; i < count; ++i)
    {
        Position pos = me->GetPosition();
        me->MovePosition(pos, frand(0.0f, 3.0f), frand(0.0f, 2.0f * float(M_PI)));
        me->CastSpell(pos, SPELL_SUMMON_ICHOR, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }
}

// 105393 - Il'gynoth, the Heart of Corruption
struct boss_ilgynoth : public BossAI
{
    boss_ilgynoth(Creature* creature) : BossAI(creature, DATA_ILGYNOTH), _heartPhase(false), _evading(false), _afterHeart(false),
        _deathglareIndex(0), _corruptorIndex(0) { }

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
        me->SetControlled(true, UNIT_STATE_ROOT);
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
            case ACTION_ICHOR_INSIDE: // Reabsorption: a later task
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

    // Stub: Task 4 fills in Cursed Blood and the timed cast.
    void StartHeartPhase()
    {
        _heartPhase = true;
        events.Reset();
        events.SetPhase(PHASE_HEART);
        SetForcefield(true);
        me->RemoveUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
        Talk(TALK_HEART_PHASE);
        // Nobody has threat on the heart yet, and UpdateVictim would evade.
        DoZoneInCombat();
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

        CleanUp(); // despawns the Eye with the other summons
        SetForcefield(false);

        _heartPhase = false;
        _EnterEvadeMode();
        _DespawnAtEvade();
        _evading = false;
    }

    void JustDied(Unit* /*killer*/) override
    {
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
        me->SetControlled(true, UNIT_STATE_ROOT);
        if (!me->HasAura(SPELL_STUFF_OF_NIGHTMARES))
            DoCastSelf(SPELL_STUFF_OF_NIGHTMARES);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
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

    void UpdateAI(uint32 /*diff*/) override
    {
        UpdateVictim();
    }

private:
    InstanceScript* _instance;
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
        _events.ScheduleEvent(EVENT_RUPTURING_ROAR, 1s);
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
                case EVENT_RUPTURING_ROAR:
                {
                    // Nobody in reach: the roar goes out back to back.
                    bool const anyInReach = SelectTarget(SelectTargetMethod::MaxThreat, 0, [this](Unit* unit)
                    {
                        return me->IsWithinMeleeRange(unit);
                    }) != nullptr;
                    if (!anyInReach)
                        DoCastAOE(SPELL_RUPTURING_ROAR);
                    _events.Repeat(500ms);
                    break;
                }
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
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
    RegisterEmeraldNightmareCreatureAI(npc_dominator_tentacle);
    RegisterEmeraldNightmareCreatureAI(npc_deathglare_tentacle);
    RegisterEmeraldNightmareCreatureAI(npc_ilgynoth_corruptor_tentacle);
    RegisterEmeraldNightmareCreatureAI(npc_nightmare_horror);
    RegisterSpellScript(spell_ilgynoth_spew_corruption);
    RegisterAreaTriggerAI(at_ilgynoth_nightmare_corruption);
}
