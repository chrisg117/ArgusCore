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

#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "ScriptedCreature.h"
#include "broken_shore.h"

// How high above the beach Dread Commander Arganoth comes into sight
static constexpr float ArganothCrashHeight = 60.0f;

// How much of his health he has left when he says that the master will mend his flesh
static constexpr int32 ArganothLowHealth = 15;

// How long the burst of fel fire shows where he comes down
static constexpr uint32 ArganothCrashFireTime = 2 * IN_MILLISECONDS;

enum ArganothEvents
{
    EVENT_FEL_CRACK                     = 1,
    EVENT_INFERNO,
    EVENT_SUMMON_FELBLAZE_INFERNAL,
    EVENT_JAINA_WARNS,
    EVENT_YELL
};

enum ArganothPoints
{
    POINT_BEACH                         = 1
};

// 90705 - Dread Commander Arganoth
// "Defeat the Commander". The instance script has him heard before he is seen, crash down on the beach and
// then stand and fight. From a video of the retail scenario: that he arrives that way, that an infernal comes
// down some ten yards from him when he yells "INFERNO!", that he says his long piece with 10 to 15% of his
// health left and his last words as he dies. His abilities are those Wowhead lists for him; when he uses
// them, and when he says which of his other lines, is a guess.
// TODO: what he looks like as he comes down; the burst of fel fire where he lands stands in for it.
struct npc_broken_shore_dread_commander_arganoth : public ScriptedAI
{
    npc_broken_shore_dread_commander_arganoth(Creature* creature) : ScriptedAI(creature), _summons(creature), _saidLowHealthLine(false), _jainaWarned(false) { }

    void JustAppeared() override
    {
        // One that is loaded again, as happens when players leave the scenario and come back, has arrived
        InstanceScript const* instance = me->GetInstanceScript();
        if (instance && instance->GetData(DATA_COMMANDER_ARRIVED))
            return;

        me->SetVisible(false);
        me->SetImmuneToAll(true);
        me->SetReactState(REACT_PASSIVE);
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_ARGANOTH_CRASH_DOWN:
            {
                Position sky = me->GetHomePosition();
                sky.m_positionZ += ArganothCrashHeight;
                me->NearTeleportTo(sky);
                me->SetVisible(true);
                me->GetMotionMaster()->MoveFall(POINT_BEACH);
                break;
            }
            case ACTION_ARGANOTH_FIGHT:
                me->SetVisible(true);
                me->SetImmuneToAll(false);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat();
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == EFFECT_MOTION_TYPE && id == POINT_BEACH)
            me->SendPlaySpellVisualKit(SPELL_VISUAL_KIT_SPIRE_OF_WOE_FIRE, 0, ArganothCrashFireTime);
    }

    void Reset() override
    {
        _events.Reset();
        _summons.DespawnAll();
        _saidLowHealthLine = false;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.ScheduleEvent(EVENT_FEL_CRACK, 8s);
        _events.ScheduleEvent(EVENT_INFERNO, 15s);
        _events.ScheduleEvent(EVENT_YELL, 38s);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
        DoZoneInCombat(summon);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _summons.Despawn(summon);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        if (_saidLowHealthLine || damage >= me->GetHealth() || !me->HealthBelowPctDamaged(ArganothLowHealth, damage))
            return;

        _saidLowHealthLine = true;
        Talk(SAY_ARGANOTH_LOW_HEALTH);

        // It takes him a quarter of a minute to say, and he says nothing else meanwhile
        _events.RescheduleEvent(EVENT_INFERNO, 20s);
        _events.RescheduleEvent(EVENT_YELL, 43s);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _summons.DespawnAll();
        Talk(SAY_ARGANOTH_DEATH);
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
                case EVENT_FEL_CRACK:
                    DoCastVictim(SPELL_FEL_CRACK);
                    _events.Repeat(12s, 16s);
                    break;
                // "INFERNO!" is the last word of what he yells
                case EVENT_INFERNO:
                    Talk(SAY_ARGANOTH_INFERNO);
                    _events.ScheduleEvent(EVENT_SUMMON_FELBLAZE_INFERNAL, 3s);
                    _events.Repeat(45s);
                    break;
                case EVENT_SUMMON_FELBLAZE_INFERNAL:
                    DoCastSelf(SPELL_SUMMON_FELBLAZE_INFERNAL);
                    if (!_jainaWarned)
                    {
                        _jainaWarned = true;
                        _events.ScheduleEvent(EVENT_JAINA_WARNS, 3s);
                    }
                    break;
                case EVENT_JAINA_WARNS:
                    if (InstanceScript* instance = me->GetInstanceScript())
                        if (Creature* jaina = instance->GetCreature(DATA_LADY_JAINA_PROUDMOORE))
                            if (jaina->IsAlive())
                                jaina->AI()->Talk(SAY_JAINA_INFERNALS);
                    break;
                case EVENT_YELL:
                    Talk(SAY_ARGANOTH_IN_THE_FIGHT);
                    _events.Repeat(45s);
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
    SummonList _summons;
    bool _saidLowHealthLine;
    bool _jainaWarned;
};

void AddSC_broken_shore_dread_commander()
{
    RegisterBrokenShoreCreatureAI(npc_broken_shore_dread_commander_arganoth);
}
