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
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
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
    SPELL_BERSERK                   = 26662,

    // Spider form
    SPELL_NECROTIC_VENOM            = 215443, // cast: dummy on up to two players
    SPELL_NECROTIC_VENOM_AURA       = 215449, // the 10 s mark, applied to each chosen player
    SPELL_NECROTIC_VENOM_DOT        = 215460, // when the mark runs out: damage and a pool under the player (225519)
    SPELL_NECROTIC_VENOM_POOL       = 225519, // creates area trigger 7078, cast by 215460's tick
    SPELL_FEEDING_TIME              = 212364,
    SPELL_VILE_AMBUSH               = 214339, // missile, then a jump to the point (214346) and a knockback on arrival
    SPELL_WEB_OF_PAIN               = 215288,
    SPELL_WEB_OF_PAIN_SOURCE        = 215300,
    SPELL_WEB_OF_PAIN_TARGET        = 215307,
    SPELL_DRIPPING_FANGS            = 210229, // spiderling passive
    SPELL_VENOMOUS_POOL_CREATE      = 213123, // spiderling death: creates area trigger 7078
    SPELL_VENOMOUS_POOL_DAMAGE      = 213124, // aura on a unit standing in a pool

    // Roc form
    SPELL_TWISTING_SHADOWS          = 210864, // 1.3 s cast, hits everyone in 300 yd; the script keeps 2 (1 solo)
    SPELL_TWISTING_SHADOWS_MARK     = 210863, // the 10 s mark; a tornado forms where the player stands when it ends
    SPELL_TWISTING_SHADOWS_AURA     = 210850, // 4 s stun and damage, put on a player the tornado reaches
    SPELL_TWISTING_SHADOWS_FIXATE   = 221877, // the tornado's target
    SPELL_GATHERING_CLOUDS          = 212707, // 2.5 s cast, 8 s wind that pushes players back
    SPELL_RAKING_TALONS             = 215582, // 2 s cast, a stacking debuff
    SPELL_RAKING_TALONS_REPEAT      = 222037, // the quick second hit
    SPELL_RAZOR_WING                = 210547  // 4.5 s cast, a cone in front of her
};

enum EleretheEvents
{
    // Energy and Berserk are timers in UpdateAI, not events: a cast must not delay them.

    // Spider form
    EVENT_NECROTIC_VENOM = 1,
    EVENT_FEEDING_TIME,
    EVENT_VILE_AMBUSH,
    EVENT_VILE_AMBUSH_FAILSAFE,
    EVENT_WEB_OF_PAIN,

    // Roc form
    EVENT_TWISTING_SHADOWS,
    EVENT_GATHERING_CLOUDS,
    EVENT_DARK_STORM,
    EVENT_RAKING_TALONS,
    EVENT_RAKING_TALONS_REPEAT,
    EVENT_RAZOR_WING
};

enum ElerethePoints
{
    POINT_FEEDING_TIME_RISE = 1
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

// Calibrate: Spider form abilities (DBM 7.3.31 times; see global-constraints.md).
static constexpr Milliseconds NecroticVenomFirst = 12200ms;
static constexpr Milliseconds NecroticVenomRepeat = 21800ms;
static constexpr uint8 NecroticVenomCasts = 4;
static constexpr Milliseconds FeedingTimeFirst = 15500ms;
static constexpr Milliseconds FeedingTimeRepeat = 50s;
static constexpr uint8 FeedingTimeCasts = 2;
static constexpr Milliseconds VileAmbushDelay = 10s;       // time in the web
static constexpr Milliseconds VileAmbushFailsafe = 9s;     // after the cast; the missile takes seconds to arrive
static constexpr Milliseconds WebOfPainRepeat = 30s;
static constexpr float FeedingTimeRise = 15.0f;            // yards straight up
static constexpr uint8 SpiderlingsPerFeedingTime = 5;
static constexpr float SpiderlingSpread = 6.0f;            // radius around the players' centroid
static constexpr uint32 NecroticVenomMaxTargets = 2;       // 1 when a single player is in the fight

// Calibrate: Roc form abilities (DBM 7.3.31 times from the form's transform; see global-constraints.md).
static constexpr Milliseconds TwistingShadowsFirst = 6600ms;
static constexpr std::array<Milliseconds, 3> TwistingShadowsGaps = { 40s, 21500ms, 32500ms };  // 4 casts
static constexpr Milliseconds GatheringCloudsFirst = 15800ms;
static constexpr Milliseconds DarkStormFirst = 26s;          // first and second Roc form only (Task 5)
static constexpr uint8 DarkStormRocForms = 2;
static constexpr Milliseconds RakingTalonsFirst = 52s;
static constexpr Milliseconds RakingTalonsRepeat = 32500ms;
static constexpr uint8 RakingTalonsCasts = 3;
static constexpr Milliseconds RazorWingFirst = 59s;
static constexpr Milliseconds RazorWingRepeat = 32500ms;
static constexpr uint8 RazorWingCasts = 3;
static constexpr uint32 TwistingShadowsMaxTargets = 2;       // 1 when a single player is in the fight

// Calibrate: the Twisting Shadows tornado.
static constexpr Milliseconds TornadoDuration = 4s;          // = the 4 s of 210850
static constexpr float TornadoSpeedRate = 0.5f;              // "slowly"
static constexpr float TornadoReach = 5.0f;                  // yards: a player this close gets 210850
static constexpr float TornadoFixateRange = 100.0f;

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

namespace
{
    // The living players she is fighting: on her threat list, GMs excluded. Not every player in the map.
    std::vector<Player*> GetLivingPlayers(Unit const* source)
    {
        std::vector<Player*> players;
        for (ThreatReference const* ref : source->GetThreatManager().GetUnsortedThreatList())
            if (Player* player = ref->GetVictim()->ToPlayer())
                if (player->IsAlive() && !player->IsGameMaster())
                    players.push_back(player);
        return players;
    }

    bool IsRangedPlayer(Player const* player)
    {
        ChrSpecializationEntry const* spec = player->GetPrimarySpecializationEntry();
        return spec && spec->GetFlags().HasFlag(ChrSpecializationFlag::Ranged);
    }

    // Venomous Pool area triggers: the spiderling's (213123) and Necrotic Venom's (225519).
    struct VenomousPoolCheck
    {
        bool operator()(AreaTrigger* at) const
        {
            return at->GetSpellId() == SPELL_VENOMOUS_POOL_CREATE || at->GetSpellId() == SPELL_NECROTIC_VENOM_POOL;
        }
    };
}

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
        _venomCasts = 0;
        _feedingCount = 0;
        _ambushActive = false;
        _webWarned = false;
        _webTargets.clear();
        _energy = 0.0f;
        _energyPerSecond = 0.0f;
        _berserkTimer = 0;
        _twistingCasts = 0;
        _talonCasts = 0;
        _razorCasts = 0;
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

        _berserkTimer = 540000;
        StartSpiderForm();
    }

    // Spider form: energy runs from 0 and the form's abilities are scheduled.
    void StartSpiderForm()
    {
        _inRocForm = false;
        events.SetPhase(PHASE_SPIDER);
        StartForm(_formCount == 1 ? FirstSpiderFormSeconds : SpiderFormSeconds);

        _venomCasts = 0;
        _feedingCount = 0;
        events.ScheduleEvent(EVENT_NECROTIC_VENOM, NecroticVenomFirst, GROUP_FORM, PHASE_SPIDER);
        events.ScheduleEvent(EVENT_FEEDING_TIME, FeedingTimeFirst, GROUP_FORM, PHASE_SPIDER);
        events.ScheduleEvent(EVENT_WEB_OF_PAIN, WebOfPainRepeat, GROUP_FORM, PHASE_SPIDER);
    }

    // Roc form: energy runs from 0 and the form's abilities are scheduled (Task 5 adds the flight).
    void StartRocForm()
    {
        _inRocForm = true;
        ++_rocCount;
        events.SetPhase(PHASE_ROC);
        StartForm(RocFormSeconds);

        _twistingCasts = 0;
        _talonCasts = 0;
        _razorCasts = 0;
        events.ScheduleEvent(EVENT_TWISTING_SHADOWS, TwistingShadowsFirst, GROUP_FORM, PHASE_ROC);
        events.ScheduleEvent(EVENT_GATHERING_CLOUDS, GatheringCloudsFirst, GROUP_FORM, PHASE_ROC);
        if (_rocCount <= DarkStormRocForms)
            events.ScheduleEvent(EVENT_DARK_STORM, DarkStormFirst, GROUP_FORM, PHASE_ROC);
        events.ScheduleEvent(EVENT_RAKING_TALONS, RakingTalonsFirst, GROUP_FORM, PHASE_ROC);
        events.ScheduleEvent(EVENT_RAZOR_WING, RazorWingFirst, GROUP_FORM, PHASE_ROC);
    }

    void StartForm(float secondsToFull)
    {
        events.CancelEventGroup(GROUP_FORM);
        _formChangePending = false;
        _energy = 0.0f;
        _energyPerSecond = float(MAX_ENERGY) / secondsToFull;
        me->SetPower(POWER_ENERGY, 0);
    }

    // Energy fills from elapsed time, outside the event map: casts (Gathering Clouds, Razor Wing)
    // must not delay the form change. At 100 the change is retried every update until it can happen.
    void UpdateEnergy(uint32 diff)
    {
        if (_energy < float(MAX_ENERGY))
        {
            _energy = std::min(_energy + _energyPerSecond * float(diff) / 1000.0f, float(MAX_ENERGY));
            // Within a thousandth of full counts as full.
            if (_energy > float(MAX_ENERGY) - 0.001f)
                _energy = float(MAX_ENERGY);
            if (int32(_energy) != me->GetPower(POWER_ENERGY))
                me->SetPower(POWER_ENERGY, int32(_energy));
        }

        if (_energy >= float(MAX_ENERGY))
            TryFormChange();
    }

    // Berserk is not held by anything.
    void UpdateBerserk(uint32 diff)
    {
        if (!_berserkTimer)
            return;

        if (diff < _berserkTimer)
        {
            _berserkTimer -= diff;
            return;
        }

        _berserkTimer = 0;
        DoCastSelf(SPELL_BERSERK, true);
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
                me->RemoveAurasDueToSpell(SPELL_SPIDER_TRANSFORM);
                DoCastSelf(SPELL_BIRD_TRANSFORM, true);
                StartRocForm();
                break;
            case SPELL_ARACHNID_FORM:
                me->RemoveAurasDueToSpell(SPELL_BIRD_TRANSFORM);
                DoCastSelf(SPELL_SPIDER_TRANSFORM, true);
                ++_formCount;
                Talk(TALK_SPIDER_FORM);
                StartSpiderForm();
                break;
            case SPELL_RAKING_TALONS:
                // One quick second hit when the cast ends.
                events.ScheduleEvent(EVENT_RAKING_TALONS_REPEAT, 1ms, GROUP_FORM, PHASE_ROC);
                break;
            case SPELL_WEB_OF_PAIN:
                // The pair lands when the cast finishes; the players were picked at its start.
                for (ObjectGuid const& guid : _webTargets)
                    if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
                        if (player->IsAlive())
                            me->CastSpell(player, guid == _webTargets.front() ? SPELL_WEB_OF_PAIN_SOURCE : SPELL_WEB_OF_PAIN_TARGET, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
                _webTargets.clear();
                break;
            default:
                break;
        }
    }

    // Two distinct random players get the Web of Pain pair. Solo it is skipped (the line too).
    void CastWebOfPain()
    {
        std::vector<Player*> players = GetLivingPlayers(me);
        if (players.size() < 2)
        {
            if (!_webWarned)
            {
                _webWarned = true;
                TC_LOG_DEBUG("scripts.ai", "boss_elerethe_renferal: Web of Pain skipped, fewer than two players in the fight");
            }
            return;
        }

        Trinity::Containers::RandomResize(players, 2);
        _webTargets.clear();
        for (Player* player : players)
            _webTargets.push_back(player->GetGUID());

        Talk(TALK_WEB_OF_PAIN);
        DoCastSelf(SPELL_WEB_OF_PAIN);
    }

    // Feeding Time: she rises out of reach and spiderlings drop around the players.
    void StartFeedingTime()
    {
        DoCastSelf(SPELL_FEEDING_TIME, true);
        Talk(TALK_FEEDING_TIME);
        Talk(TALK_FEEDING_TIME_EMOTE);

        SetBusy(true);
        me->SetUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        Position up = me->GetPosition();
        up.m_positionZ += FeedingTimeRise;
        me->GetMotionMaster()->MovePoint(POINT_FEEDING_TIME_RISE, up, false);

        SummonSpiderlings();
        events.ScheduleEvent(EVENT_VILE_AMBUSH, VileAmbushDelay, GROUP_FORM, PHASE_SPIDER);
    }

    void SummonSpiderlings()
    {
        // Around the centroid of the players, at ground z.
        std::vector<Player*> players = GetLivingPlayers(me);
        float x = me->GetPositionX(), y = me->GetPositionY(), z = me->GetPositionZ();
        if (!players.empty())
        {
            x = y = z = 0.0f;
            for (Player* player : players)
            {
                x += player->GetPositionX();
                y += player->GetPositionY();
                z += player->GetPositionZ();
            }
            x /= players.size();
            y /= players.size();
            z /= players.size();
        }

        for (uint8 i = 0; i < SpiderlingsPerFeedingTime; ++i)
        {
            float const angle = frand(0.0f, 2.0f * float(M_PI));
            float const dist = frand(0.0f, SpiderlingSpread);
            float sx = x + std::cos(angle) * dist;
            float sy = y + std::sin(angle) * dist;
            float sz = z;
            me->UpdateGroundPositionZ(sx, sy, sz);
            me->SummonCreature(NPC_VENOMOUS_SPIDERLING, Position(sx, sy, sz, frand(0.0f, 2.0f * float(M_PI))), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10s);
        }
    }

    // Vile Ambush: jump onto a random player's spot. Land() ends the web phase.
    void StartVileAmbush()
    {
        Position dest = me->GetPosition();
        std::vector<Player*> players = GetLivingPlayers(me);
        if (!players.empty())
            dest = Trinity::Containers::SelectRandomContainerElement(players)->GetPosition();
        float z = dest.GetPositionZ();
        me->UpdateGroundPositionZ(dest.m_positionX, dest.m_positionY, z);
        dest.m_positionZ = z;

        _ambushActive = true;
        me->CastSpell(dest, SPELL_VILE_AMBUSH, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
        // Landing comes from MovementInform; this catches a jump that never happens.
        events.ScheduleEvent(EVENT_VILE_AMBUSH_FAILSAFE, VileAmbushFailsafe, GROUP_FORM, PHASE_SPIDER);
    }

    void Land()
    {
        if (!_ambushActive)
            return;

        _ambushActive = false;
        events.CancelEvent(EVENT_VILE_AMBUSH_FAILSAFE);
        me->SetDisableGravity(false);
        me->SetCanFly(false);
        me->RemoveUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
        if (Unit* victim = me->GetVictim())
            me->GetMotionMaster()->MoveChase(victim);
        SetBusy(false);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == EFFECT_MOTION_TYPE && id == EVENT_JUMP)
            Land();
    }

    void RemovePools()
    {
        std::list<AreaTrigger*> pools;
        VenomousPoolCheck check;
        Trinity::AreaTriggerListSearcher<VenomousPoolCheck> searcher(me, pools, check);
        Cell::VisitAllObjects(me, searcher, 250.0f);
        for (AreaTrigger* pool : pools)
            pool->Remove();
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
        events.CancelEventGroup(GROUP_FORM);
        _berserkTimer = 0;
        _ambushActive = false;
        _webTargets.clear();
        RemovePools();
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NECROTIC_VENOM_AURA);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_NECROTIC_VENOM_DOT);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WEB_OF_PAIN_SOURCE);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_WEB_OF_PAIN_TARGET);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_VENOMOUS_POOL_DAMAGE);
        me->RemoveAurasDueToSpell(SPELL_GATHERING_CLOUDS);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWISTING_SHADOWS_MARK);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWISTING_SHADOWS_AURA);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_RAKING_TALONS);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        // An idle evade (not engaged, or already dead) must not despawn her.
        if (!me->IsEngaged() || !me->IsAlive())
            return;

        Talk(TALK_WIPE);
        me->InterruptNonMeleeSpells(false);

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

        // Before the casting check, so a long cast never delays either.
        UpdateEnergy(diff);
        UpdateBerserk(diff);

        events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = events.ExecuteEvent())
        {
            // Holding: up in the web or in flight nothing else is cast. Vile Ambush ends the web
            // phase; the energy timer keeps the form change due.
            bool const ambush = eventId == EVENT_VILE_AMBUSH || eventId == EVENT_VILE_AMBUSH_FAILSAFE;
            if (_busy && !ambush)
            {
                events.Repeat(1s);
                continue;
            }

            // A cast began earlier in this update: this one waits, it is not lost.
            if (!ambush && me->HasUnitState(UNIT_STATE_CASTING))
            {
                events.Repeat(100ms);
                continue;
            }

            switch (eventId)
            {
                case EVENT_NECROTIC_VENOM:
                    DoCastSelf(SPELL_NECROTIC_VENOM);
                    if (++_venomCasts < NecroticVenomCasts)
                        events.Repeat(NecroticVenomRepeat);
                    break;
                case EVENT_FEEDING_TIME:
                    StartFeedingTime();
                    if (++_feedingCount < FeedingTimeCasts)
                        events.Repeat(FeedingTimeRepeat);
                    break;
                case EVENT_VILE_AMBUSH:
                    StartVileAmbush();
                    break;
                case EVENT_VILE_AMBUSH_FAILSAFE:
                    Land();
                    break;
                case EVENT_WEB_OF_PAIN:
                    CastWebOfPain();
                    events.Repeat(WebOfPainRepeat);
                    break;
                case EVENT_TWISTING_SHADOWS:
                    // The spell's script marks the players (2, 1 solo).
                    DoCastSelf(SPELL_TWISTING_SHADOWS);
                    if (_twistingCasts < TwistingShadowsGaps.size())
                        events.Repeat(TwistingShadowsGaps[_twistingCasts]);
                    ++_twistingCasts;
                    break;
                case EVENT_GATHERING_CLOUDS:
                    Talk(TALK_GATHERING_CLOUDS);
                    Talk(TALK_GATHERING_CLOUDS_EMOTE);
                    DoCastSelf(SPELL_GATHERING_CLOUDS);
                    break;
                case EVENT_DARK_STORM:
                    // Task 5: the flight and the storm.
                    break;
                case EVENT_RAKING_TALONS:
                    DoCastVictim(SPELL_RAKING_TALONS);
                    if (++_talonCasts < RakingTalonsCasts)
                        events.Repeat(RakingTalonsRepeat);
                    break;
                case EVENT_RAKING_TALONS_REPEAT:
                    DoCastVictim(SPELL_RAKING_TALONS_REPEAT);
                    break;
                case EVENT_RAZOR_WING:
                    Talk(TALK_RAZOR_WING);
                    DoCastVictim(SPELL_RAZOR_WING);
                    if (++_razorCasts < RazorWingCasts)
                        events.Repeat(RazorWingRepeat);
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
    uint8 _venomCasts;
    uint8 _feedingCount;
    bool _ambushActive;        // Vile Ambush cast, not landed yet
    bool _webWarned;           // the solo debug line is logged once per fight
    std::vector<ObjectGuid> _webTargets;
    float _energy;
    float _energyPerSecond;
    uint32 _berserkTimer;      // ms left to Berserk; 0 = not running
    uint8 _twistingCasts;
    uint8 _talonCasts;
    uint8 _razorCasts;
};

// 106311 - Venomous Spiderling
struct npc_en_venomous_spiderling : public ScriptedAI
{
    npc_en_venomous_spiderling(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_DRIPPING_FANGS, true);
        DoZoneInCombat();
        if (Player* nearest = me->SelectNearestPlayer(200.0f))
        {
            AddThreat(nearest, 1000.0f);
            AttackStart(nearest);
        }
    }

    // The pool is made here, at the corpse (Task 1 (d)); 215489 would only hurt the corpse.
    void JustDied(Unit* /*killer*/) override
    {
        DoCastSelf(SPELL_VENOMOUS_POOL_CREATE, true);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (!me->IsEngaged())
            DoZoneInCombat();
        UpdateVictim();
    }
};

// 215443 - Necrotic Venom: one target when a single player is in the fight, else two; ranged first.
class spell_en_necrotic_venom : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_NECROTIC_VENOM_AURA });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target) { return !target->IsPlayer() || !target->ToPlayer()->IsAlive(); });

        size_t const wanted = GetLivingPlayers(GetCaster()).size() <= 1 ? 1 : NecroticVenomMaxTargets;
        if (targets.size() <= wanted)
            return;

        std::vector<WorldObject*> ranged, other;
        for (WorldObject* target : targets)
            (IsRangedPlayer(target->ToPlayer()) ? ranged : other).push_back(target);

        // Ranged players first (chosen among themselves), then anyone for the rest.
        Trinity::Containers::RandomShuffle(ranged);
        Trinity::Containers::RandomShuffle(other);
        std::vector<WorldObject*> picked;
        for (WorldObject* target : ranged)
            if (picked.size() < wanted)
                picked.push_back(target);
        for (WorldObject* target : other)
            if (picked.size() < wanted)
                picked.push_back(target);

        targets.assign(picked.begin(), picked.end());
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_NECROTIC_VENOM_AURA, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_en_necrotic_venom::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_en_necrotic_venom::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 215449 - Necrotic Venom (the 10 s mark): when it runs out, the damage and pool-dropping debuff starts.
class spell_en_necrotic_venom_mark : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_NECROTIC_VENOM_DOT });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Unit* target = GetTarget();
        if (!target->IsAlive())
            return;

        if (Unit* caster = GetCaster())
            if (caster->IsAlive() && caster->IsEngaged())
                caster->CastSpell(target, SPELL_NECROTIC_VENOM_DOT, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_en_necrotic_venom_mark::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 7078 - Venomous Pool (created by 213123 on a spiderling's death and by 225519 under a Necrotic Venom player)
struct at_en_venomous_pool : AreaTriggerAI
{
    at_en_venomous_pool(AreaTrigger* at) : AreaTriggerAI(at) { }

    // 213124 targets its caster (effect target 1), so the player must be the caster, or the aura
    // lands on whoever cast it (the boss stacked it on herself). Elerethe is the original caster
    // for the damage credit. One application per player: the pool presence holds it.
    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer() || !unit->IsAlive() || unit->HasAura(SPELL_VENOMOUS_POOL_DAMAGE))
            return;

        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        if (InstanceScript* instance = at->GetInstanceScript())
            if (Creature* boss = instance->GetCreature(DATA_ELERETHE_RENFERAL))
                args.SetOriginalCaster(boss->GetGUID());
        unit->CastSpell(unit, SPELL_VENOMOUS_POOL_DAMAGE, args);
    }

    void OnUnitExit(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        // Pools overlap: keep the damage while the player still stands in another one.
        std::list<AreaTrigger*> pools;
        VenomousPoolCheck check;
        Trinity::AreaTriggerListSearcher<VenomousPoolCheck> searcher(at, pools, check);
        Cell::VisitAllObjects(at, searcher, 15.0f);
        for (AreaTrigger* other : pools)
            if (other != at && other->GetInsideUnits().contains(unit->GetGUID()))
                return;

        unit->RemoveAurasDueToSpell(SPELL_VENOMOUS_POOL_DAMAGE);
    }
};

// 108542 - Skittering Spiderling (Task 6)
struct npc_en_skittering_spiderling : public ScriptedAI
{
    npc_en_skittering_spiderling(Creature* creature) : ScriptedAI(creature) { }
};

// 106350 - Twisting Shadows: the tornado a 210863 mark leaves behind. It follows a player slowly and
// puts 210850 (stun and damage for 4 s) on whoever it reaches, then goes (the summon's duration).
// 210850's own effects target its caster, so the player casts it on themselves; the tornado is the
// original caster. The boss summons the tornado, so JustSummoned puts it in her summons.
struct npc_en_twisting_shadows : public ScriptedAI
{
    npc_en_twisting_shadows(Creature* creature) : ScriptedAI(creature), _checkTimer(0) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetSpeedRate(MOVE_WALK, TornadoSpeedRate);
        me->SetSpeedRate(MOVE_RUN, TornadoSpeedRate);

        // The player it chases: the nearest one (the marked player stood where it formed).
        if (Player* target = me->SelectNearestPlayer(TornadoFixateRange))
        {
            if (!target->IsGameMaster())
            {
                DoCast(target, SPELL_TWISTING_SHADOWS_FIXATE, true);
                me->GetMotionMaster()->MoveFollow(target, 0.5f);
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = 250;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, TornadoReach);
        for (Player* player : players)
        {
            if (!player->IsAlive() || player->IsGameMaster() || player->HasAura(SPELL_TWISTING_SHADOWS_AURA))
                continue;

            CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
            args.SetOriginalCaster(me->GetGUID());
            player->CastSpell(player, SPELL_TWISTING_SHADOWS_AURA, args);
        }
    }

private:
    uint32 _checkTimer;
};

// 210864 - Twisting Shadows (cast): marks up to two players (one when solo) with 210863.
class spell_en_twisting_shadows : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_TWISTING_SHADOWS_MARK });
    }

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target)
        {
            Player* player = target->ToPlayer();
            return !player || !player->IsAlive() || player->IsGameMaster();
        });

        size_t const wanted = GetLivingPlayers(GetCaster()).size() <= 1 ? 1 : TwistingShadowsMaxTargets;
        if (targets.size() > wanted)
            Trinity::Containers::RandomResize(targets, wanted);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetHitUnit(), SPELL_TWISTING_SHADOWS_MARK, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_en_twisting_shadows::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnEffectHitTarget += SpellEffectFn(spell_en_twisting_shadows::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 210863 - Twisting Shadows (the 10 s mark): when it runs out, a tornado forms where the player stands.
// The aura's own periodic trigger (210877, a summon at the caster) is stopped; the boss summons the
// tornado at the marked player instead.
class spell_en_twisting_shadows_mark : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Unit* target = GetTarget();
        Unit* caster = GetCaster();
        if (!target->IsAlive() || !caster || !caster->IsAlive() || !caster->IsEngaged())
            return;

        caster->SummonCreature(NPC_TWISTING_SHADOWS, target->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, TornadoDuration);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_en_twisting_shadows_mark::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_en_twisting_shadows_mark::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
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
    RegisterSpellScript(spell_en_necrotic_venom);
    RegisterSpellScript(spell_en_necrotic_venom_mark);
    RegisterSpellScript(spell_en_twisting_shadows);
    RegisterSpellScript(spell_en_twisting_shadows_mark);
    RegisterAreaTriggerAI(at_en_venomous_pool);
}
