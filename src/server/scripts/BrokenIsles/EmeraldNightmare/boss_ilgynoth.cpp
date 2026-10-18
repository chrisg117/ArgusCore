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
#include "Map.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "emerald_nightmare.h"

enum IlgynothSpells
{
    SPELL_STUFF_OF_NIGHTMARES   = 209915 // Eye: permanent, -99% damage taken
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

bool IsInsideChamber(WorldObject const* obj)
{
    float const dx = obj->GetPositionX() - MouthPosition.GetPositionX();
    float const dy = obj->GetPositionY() - MouthPosition.GetPositionY();
    if (dx * std::cos(MouthFacing) + dy * std::sin(MouthFacing) >= 0.0f)
        return false;

    if (obj->GetExactDist2d(HeartPosition) > ChamberRadius)
        return false;

    return std::fabs(obj->GetPositionZ() - HeartPosition.GetPositionZ()) <= ChamberHeight;
}

// 105393 - Il'gynoth, the Heart of Corruption
struct boss_ilgynoth : public BossAI
{
    boss_ilgynoth(Creature* creature) : BossAI(creature, DATA_ILGYNOTH), _heartPhase(false), _evading(false) { }

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

    // Stub: Task 3 fills in the Dominators, the waves and the Eye's schedule.
    // The heart has no victim in this phase, so the engage is kept by pulling
    // the zone again at each start.
    void StartOutsidePhase(bool /*afterHeart*/)
    {
        _heartPhase = false;
        events.Reset();
        events.SetPhase(PHASE_OUTSIDE);
        if (!me->IsEngaged())
            DoZoneInCombat();
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
        if (Creature* eye = instance->GetCreature(DATA_EYE_OF_ILGYNOTH))
            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, eye);
        summons.DespawnAll();
        me->RemoveAllAreaTriggers();
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
    }

private:
    bool _heartPhase;
    bool _evading;
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

void AddSC_boss_ilgynoth()
{
    RegisterEmeraldNightmareCreatureAI(boss_ilgynoth);
    RegisterEmeraldNightmareCreatureAI(npc_eye_of_ilgynoth);
}
