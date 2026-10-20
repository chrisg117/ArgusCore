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
#include "emerald_nightmare.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

enum EleretheSpells
{
    SPELL_DARKWING_FORM             = 210308, // cast: Spider to Roc (DBM's trigger, UNIT_SPELLCAST_SUCCEEDED)
    SPELL_BIRD_TRANSFORM            = 226039, // the Roc display aura
    SPELL_ARACHNID_FORM             = 210326, // cast: Roc to Spider
    SPELL_SPIDER_TRANSFORM          = 226055, // the Spider display aura
    SPELL_SWARM                     = 214450, // passive
    SPELL_BERSERK                   = 26662
};

enum EleretheEvents
{
    EVENT_BERSERK = 1,
    EVENT_ENERGY
};

enum EleretheGroups
{
    GROUP_FORM = 1  // every event of the current form, cancelled when the form ends
};

enum ElerethePhases
{
    PHASE_SPIDER = 1,
    PHASE_ROC    = 2
};

enum EleretheTalks
{
    TALK_WEB_OF_PAIN = 0,
    TALK_FEEDING_TIME,
    TALK_GATHERING_CLOUDS,
    TALK_AGGRO,
    TALK_RAZOR_WING,
    TALK_SPIDER_FORM,
    TALK_KILL,
    TALK_WIPE,
    TALK_DEATH,
    TALK_INTRO_1,
    TALK_INTRO_2,
    TALK_INTRO_3,
    TALK_FEEDING_TIME_EMOTE,
    TALK_GATHERING_CLOUDS_EMOTE
};

// Calibrate: every tunable of the fight, in one place.
// Energy fills 0 to 100 in this many seconds (DBM 7.3.31: Roc Form at 90 s on the pull, then 92 s
// from each return to Spider form; the Roc form lasts 132 s).
static constexpr float FirstSpiderFormSeconds = 90.0f;
static constexpr float SpiderFormSeconds = 92.0f;
static constexpr float RocFormSeconds = 132.0f;
static constexpr uint32 MAX_ENERGY = 100;

// Calibrate: how high above the web she flies over a bridge's middle.
static constexpr float FlightHeight = 20.0f;

struct PlatformData
{
    Position landing;   // where she lands, facing the next platform
    Position centre;    // centre of the platform circle (at the floor z)
    float radius;
};

namespace
{
    Position MakePosition(EleretheRoom::Point const& p)
    {
        return Position(p.x, p.y, p.z);
    }

    // The landing spot, facing the next platform.
    Position MakeLanding(EleretheRoom::Point const& p, EleretheRoom::Point const& towards)
    {
        return Position(p.x, p.y, p.z, Position::NormalizeOrientation(std::atan2(towards.y - p.y, towards.x - p.x)));
    }

    PlatformData MakePlatform(size_t index)
    {
        using namespace EleretheRoom;
        return { MakeLanding(Landings[index], Landings[(index + 1) % Landings.size()]), MakePosition(Landings[index]), PlatformRadii[index] };
    }

    Position MakeRaised(EleretheRoom::Point const& p, float raise)
    {
        return Position(p.x, p.y, p.z + raise);
    }
}

// Landings face the next platform; the third faces back to the first.
static std::array<PlatformData, 3> const Platforms = { MakePlatform(0), MakePlatform(1), MakePlatform(2) };

// The flight from P1 to P2 and from P2 to P3: an air point over the bridge's middle, then the landing.
static std::array<std::vector<Position>, 2> const FlightPaths =
{{
    { MakeRaised(EleretheRoom::BridgeMiddles[0], FlightHeight), Platforms[1].landing },
    { MakeRaised(EleretheRoom::BridgeMiddles[1], FlightHeight), Platforms[2].landing }
}};

// The middle of each bridge web, where she is drawn to for Feeding Time.
static std::array<Position, 2> const BridgeMidpoints = { MakePosition(EleretheRoom::BridgeMiddles[0]), MakePosition(EleretheRoom::BridgeMiddles[1]) };

// 106087 - Elerethe Renferal
struct boss_elerethe_renferal : public BossAI
{
    boss_elerethe_renferal(Creature* creature) : BossAI(creature, DATA_ELERETHE_RENFERAL) { ResetState(); }

    void JustAppeared() override
    {
        me->SetMaxPower(POWER_ENERGY, MAX_ENERGY); // power is set to 0 in Creature::UpdateMaxPower
        me->SetPower(POWER_ENERGY, 0);
    }

    void ResetState()
    {
        _platform = 0;
        _formCount = 1;
        _rocCount = 0;
        _busy = false;
        _formChangePending = false;
        _inRocForm = false;
        _energy = 0.0f;
        _energyPerSecond = 0.0f;
    }

    void Reset() override
    {
        BossAI::Reset();
        ResetState();
        me->SetMaxPower(POWER_ENERGY, MAX_ENERGY);
        me->SetPower(POWER_ENERGY, 0);
        me->RemoveAurasDueToSpell(SPELL_BIRD_TRANSFORM);
        me->RemoveAurasDueToSpell(SPELL_SPIDER_TRANSFORM);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        Talk(TALK_AGGRO);
        DoCastSelf(SPELL_SWARM, true);

        events.ScheduleEvent(EVENT_BERSERK, 540s);
        StartSpiderForm();
    }

    // Spider form: energy runs from 0 and the form's abilities are scheduled (Tasks 3 and 4).
    void StartSpiderForm()
    {
        _inRocForm = false;
        events.SetPhase(PHASE_SPIDER);
        StartForm(_formCount == 1 ? FirstSpiderFormSeconds : SpiderFormSeconds);
    }

    // Roc form: energy runs from 0 and the form's abilities are scheduled (Task 5).
    void StartRocForm()
    {
        _inRocForm = true;
        ++_rocCount;
        events.SetPhase(PHASE_ROC);
        StartForm(RocFormSeconds);
    }

    void StartForm(float secondsToFull)
    {
        events.CancelEventGroup(GROUP_FORM);
        _formChangePending = false;
        _energy = 0.0f;
        _energyPerSecond = float(MAX_ENERGY) / secondsToFull;
        me->SetPower(POWER_ENERGY, 0);
        events.RescheduleEvent(EVENT_ENERGY, 1s);
    }

    // Energy is full: change form as soon as she is on the ground and not in a cast. If she is up
    // in the web or in flight, the change is held (energy stays at 100) until SetBusy(false).
    void TryFormChange()
    {
        if (me->GetPower(POWER_ENERGY) < int32(MAX_ENERGY))
            return;

        if (_busy || me->HasUnitState(UNIT_STATE_CASTING))
        {
            _formChangePending = true;
            return;
        }

        _formChangePending = false;
        DoCastSelf(_inRocForm ? SPELL_ARACHNID_FORM : SPELL_DARKWING_FORM);
    }

    // True while she is up in the web or in flight. Clearing it lets a held form change happen
    // (Task 3 adds the held ability events).
    void SetBusy(bool busy)
    {
        _busy = busy;
        if (!busy)
            TryFormChange();
    }

    void OnSpellCast(SpellInfo const* spell) override
    {
        switch (spell->Id)
        {
            case SPELL_DARKWING_FORM:
                DoCastSelf(SPELL_BIRD_TRANSFORM, true);
                StartRocForm();
                break;
            case SPELL_ARACHNID_FORM:
                DoCastSelf(SPELL_SPIDER_TRANSFORM, true);
                ++_formCount;
                Talk(TALK_SPIDER_FORM);
                StartSpiderForm();
                break;
            default:
                break;
        }
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(TALK_KILL);
    }

    void CleanUp()
    {
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        summons.DespawnAll();
        me->RemoveAllAreaTriggers();
        me->RemoveAurasDueToSpell(SPELL_BIRD_TRANSFORM);
        me->RemoveAurasDueToSpell(SPELL_SPIDER_TRANSFORM);
        me->RemoveAurasDueToSpell(SPELL_SWARM);
        me->SetDisableGravity(false);
        me->SetCanFly(false);
        me->RemoveUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        if (me->IsEngaged())
            Talk(TALK_WIPE);

        CleanUp();
        // Task 6: tell the instance to respawn the egg sacs.
        _EnterEvadeMode();
        ResetState();
        me->SetPower(POWER_ENERGY, 0);
        _DespawnAtEvade();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(TALK_DEATH);
        CleanUp();
        _JustDied();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BERSERK:
                    DoCastSelf(SPELL_BERSERK, true);
                    break;
                case EVENT_ENERGY:
                    // A float accumulator: the rate is not a whole number a second.
                    if (_energy < float(MAX_ENERGY))
                    {
                        _energy = std::min(_energy + _energyPerSecond, float(MAX_ENERGY));
                        // Within a thousandth of full counts as full (100/92 added 92 times).
                        if (_energy > float(MAX_ENERGY) - 0.001f)
                            _energy = float(MAX_ENERGY);
                        me->SetPower(POWER_ENERGY, int32(_energy));
                    }
                    // At 100 the change is retried every second until it can happen.
                    if (_energy >= float(MAX_ENERGY))
                        TryFormChange();
                    events.Repeat(1s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    uint8 _platform;           // 0-2, the platform she stands on
    uint8 _formCount;          // Spider forms started; 1 at the pull
    uint8 _rocCount;
    bool _busy;                // true while up in the web or in flight
    bool _formChangePending;
    bool _inRocForm;
    float _energy;
    float _energyPerSecond;
};

// 106311 - Venomous Spiderling (Task 3)
struct npc_en_venomous_spiderling : public ScriptedAI
{
    npc_en_venomous_spiderling(Creature* creature) : ScriptedAI(creature) { }
};

// 108542 - Skittering Spiderling (Task 6)
struct npc_en_skittering_spiderling : public ScriptedAI
{
    npc_en_skittering_spiderling(Creature* creature) : ScriptedAI(creature) { }
};

// 106350 - Twisting Shadows (Task 5)
struct npc_en_twisting_shadows : public ScriptedAI
{
    npc_en_twisting_shadows(Creature* creature) : ScriptedAI(creature) { }
};

// 108540 - Egg Sac (Task 6)
struct npc_en_elerethe_egg_sac : public ScriptedAI
{
    npc_en_elerethe_egg_sac(Creature* creature) : ScriptedAI(creature) { }
};

void AddSC_boss_elerethe_renferal()
{
    RegisterEmeraldNightmareCreatureAI(boss_elerethe_renferal);
    RegisterCreatureAI(npc_en_venomous_spiderling);
    RegisterCreatureAI(npc_en_skittering_spiderling);
    RegisterCreatureAI(npc_en_twisting_shadows);
    RegisterCreatureAI(npc_en_elerethe_egg_sac);
}
