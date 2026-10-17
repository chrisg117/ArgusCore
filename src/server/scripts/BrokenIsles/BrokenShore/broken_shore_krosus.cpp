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
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Random.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "broken_shore.h"

// How much of his health he has left when he leaves the Alliance for the Horde, comes back, and leaves again
static constexpr int32 KrosusToTheHordeHealth = 75;
static constexpr int32 KrosusToTheAllianceHealth = 50;
static constexpr int32 KrosusToTheHordeAgainHealth = 25;

// Where he fights the Alliance: where Tirion hung, before its ledge. How far he comes is a guess.
Position const KrosusAlliancePosition = { 1495.6f, 1782.4f, 31.9f, 1.48f };

// Where he fights the Horde, before its ledge. It was taken in game, at the height he has in the pool.
Position const KrosusHordePosition = { 1442.5f, 1760.0f, 31.9f, 3.29f };

static constexpr float KrosusArriveDistance = 3.0f;
static constexpr uint32 KrosusMoveCheckInterval = 1 * IN_MILLISECONDS;

// How much of his health the Horde takes from him, and how often
static constexpr int32 KrosusHordeDamage = 1;
static constexpr uint32 KrosusHordeDamageInterval = 1 * IN_MILLISECONDS;

// How far from him the Horde's leaders are looked for, and how often they strike
static constexpr float KrosusHordeLeaderDistance = 60.0f;
static constexpr uint32 KrosusHordeStrikeInterval = 2 * IN_MILLISECONDS;

// How long after he dies fel bursts from the pool where he lies, and he is gone
static constexpr Milliseconds KrosusEruptionDelay = 3s;
static constexpr Milliseconds KrosusCorpseTime = 4s;
static constexpr uint32 KrosusEruptionTime = 3 * IN_MILLISECONDS;

// The Alliance's ledge, where his Fel Storm comes down: how far from its middle, from how high and how fast,
// how long after he starts and how far apart. Where a meteor lands a demon stands.
Position const KrosusFelStormPosition = { 1495.8f, 1813.7f, 37.4f, 0.0f };
static constexpr float KrosusFelStormSpread = 14.0f;
static constexpr float KrosusFelStormHeight = 40.0f;
static constexpr float KrosusFelStormSpeed = 20.0f;
static constexpr float KrosusFelStormLedgeHeight = 4.0f;
static constexpr uint32 KrosusFelStormDelay = 1500;
static constexpr uint32 KrosusFelStormInterval = 700;
static constexpr Milliseconds KrosusFelStormStalkerTime = 20s;
static constexpr Milliseconds KrosusDemonCorpseTime = 10s;

// The demons of one Fel Storm. Which they are is from videos of the retail scenario; how many Burning
// Terrorhounds is a guess.
static constexpr uint32 KrosusFelStormDemons[] =
{
    NPC_BURNING_TERRORHOUND, NPC_LIVING_FELBLAZE, NPC_BURNING_SENTRY, NPC_BURNING_TERRORHOUND, NPC_LIVING_FELBLAZE, NPC_LIVING_FELBLAZE,
    NPC_BURNING_TERRORHOUND, NPC_LIVING_FELBLAZE, NPC_BURNING_SENTRY, NPC_BURNING_TERRORHOUND, NPC_LIVING_FELBLAZE
};

enum KrosusEvents
{
    EVENT_SLAM                          = 1,
    EVENT_FEL_STORM,
    EVENT_ORB_OF_DESTRUCTION
};

enum KrosusPoints
{
    POINT_LEDGE                         = 1
};

enum KrosusPhases
{
    PHASE_WAITING,
    PHASE_ALLIANCE,
    PHASE_HORDE,
    PHASE_ALLIANCE_AGAIN,
    PHASE_HORDE_AGAIN,
    PHASE_DYING
};

// 90544 - Krosus
// "Krosus". The instance script has him rise from the pool and sends him at the Alliance's ledge. From a
// video of the retail scenario: with 75% of his health left he goes to the Horde's ledge across the pool,
// with 50% he comes back to the Alliance's, with 25% he goes to the Horde again, which finishes him off;
// players can hit him from afar while he is with the Horde; he stumbles back as he dies, sinks into the
// pool, where fel bursts up, and leaves no corpse. Who says what as he goes is from that video too, and the
// lines are Blizzard's. So is what he casts: Slam twice before he first leaves; Fel Storm when he is with
// the Horde, both times, and when he is back with the Alliance, where an Orb of Destruction and a Slam
// follow it; and with the Fel Storm meteors come down on the Alliance's ledge and demons with them. When
// he casts them is by eye; that demons come with every Fel Storm, and what Jaina says to the second and
// third, is a guess.
// The Horde's leaders are not fought by anything: what they take from him is taken by this script, and how
// fast is a guess.
// TODO: his Slam hurts nobody; what the demons cast; that he sinks (his corpse is taken away).
struct npc_broken_shore_krosus : public ScriptedAI
{
    npc_broken_shore_krosus(Creature* creature) : ScriptedAI(creature), _phase(PHASE_WAITING), _moving(false), _reachedFloor(false),
        _moveCheckTimer(0), _hordeDamageTimer(0), _hordeStrikeTimer(0), _felStorms(0), _felStormDemon(std::size(KrosusFelStormDemons)),
        _felStormTimer(0) { }

    // A creature that fights swims where there is water, and the pool is that: he would swim from ledge to
    // ledge instead of running as he does when he first comes
    void JustAppeared() override
    {
        me->SetUnitFlag(UNIT_FLAG_CANT_SWIM);
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_KROSUS_ADVANCE || _phase != PHASE_WAITING)
            return;

        _phase = PHASE_ALLIANCE;
        LeaderTalk(DATA_KING_VARIAN_WRYNN, SAY_VARIAN_BRACE_YOURSELVES);
        MoveTo(KrosusAlliancePosition);
    }

    // He does not give up, wherever those he fights are
    void EnterEvadeMode(EvadeReason why) override
    {
        if (_phase == PHASE_WAITING)
            ScriptedAI::EnterEvadeMode(why);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        if (_phase == PHASE_WAITING || _phase == PHASE_DYING || _reachedFloor)
        {
            damage = 0;
            return;
        }

        uint64 floor = std::max<uint64>(me->CountPctFromMaxHealth(FloorHealth()), 1);
        if (me->GetHealth() > floor + damage)
            return;

        // He goes no lower than where he leaves, and does not die before he has sunk into the pool
        damage = me->GetHealth() > floor ? uint32(me->GetHealth() - floor) : 0;
        _reachedFloor = true;
        if (attacker)
            _killer = attacker->GetGUID();
    }

    void UpdateAI(uint32 diff) override
    {
        if (_reachedFloor)
        {
            _reachedFloor = false;
            LeaveOrDie();
            if (!me->IsAlive())
                return;
        }

        UpdateFelStorm(diff);

        if (_moving)
        {
            UpdateMove(diff);
            return;
        }

        switch (_phase)
        {
            case PHASE_ALLIANCE:
            case PHASE_ALLIANCE_AGAIN:
                break;
            case PHASE_HORDE:
            case PHASE_HORDE_AGAIN:
                UpdateHorde(diff);
                if (me->IsAlive())
                    UpdateEvents(diff);
                return;
            default:
                return;
        }

        if (UpdateVictim())
            UpdateEvents(diff);
    }

private:
    void UpdateEvents(uint32 diff)
    {
        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SLAM:
                    DoCastVictim(SPELL_KROSUS_SLAM);
                    _events.Repeat(_phase == PHASE_ALLIANCE ? 20s : 25s);
                    break;
                case EVENT_FEL_STORM:
                    DoCastSelf(SPELL_KROSUS_FEL_STORM);
                    ++_felStorms;
                    _felStormDemon = 0;
                    _felStormTimer = KrosusFelStormDelay;
                    if (TempSummon* stalker = me->SummonCreature(NPC_FEL_METEOR_STALKER, KrosusFelStormPosition.GetPositionX(), KrosusFelStormPosition.GetPositionY(),
                        KrosusFelStormPosition.GetPositionZ() + KrosusFelStormHeight, 0.0f, TEMPSUMMON_TIMED_DESPAWN, KrosusFelStormStalkerTime))
                        _felStormStalker = stalker->GetGUID();
                    break;
                case EVENT_ORB_OF_DESTRUCTION:
                {
                    Unit* target = SelectTarget(SelectTargetMethod::Random, 0, 150.0f, true);
                    if (!target)
                        target = me->GetVictim();
                    if (target)
                        me->CastSpell(target->GetPosition(), SPELL_ORB_OF_DESTRUCTION, false);
                    _events.Repeat(25s);
                    break;
                }
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

    // A meteor of his Fel Storm leaves for the Alliance's ledge, and a demon stands where it lands
    void UpdateFelStorm(uint32 diff)
    {
        if (_felStormDemon >= std::size(KrosusFelStormDemons))
            return;

        if (_felStormTimer > diff)
        {
            _felStormTimer -= diff;
            return;
        }

        _felStormTimer = KrosusFelStormInterval;
        uint32 entry = KrosusFelStormDemons[_felStormDemon++];

        Position place = KrosusFelStormPosition;
        for (uint8 i = 0; i < 10; ++i)
        {
            float angle = frand(0.0f, 2.0f * float(M_PI));
            float distance = KrosusFelStormSpread * std::sqrt(frand(0.0f, 1.0f));
            float x = KrosusFelStormPosition.GetPositionX() + distance * std::cos(angle);
            float y = KrosusFelStormPosition.GetPositionY() + distance * std::sin(angle);
            float z = me->GetMap()->GetHeight(me->GetPhaseShift(), x, y, KrosusFelStormPosition.GetPositionZ() + KrosusFelStormLedgeHeight);
            if (std::abs(z - KrosusFelStormPosition.GetPositionZ()) > KrosusFelStormLedgeHeight)
                continue;

            place.Relocate(x, y, z, angle);
            break;
        }

        Milliseconds flight = 0ms;
        if (Creature* stalker = ObjectAccessor::GetCreature(*me, _felStormStalker))
        {
            stalker->SendPlaySpellVisual(place, SPELL_VISUAL_FEL_METEOR, 0, 0, KrosusFelStormSpeed);
            flight = Milliseconds(int64(stalker->GetExactDist(place) / KrosusFelStormSpeed * float(IN_MILLISECONDS)));
        }

        bool first = _felStormDemon == 1;
        me->m_Events.AddEventAtOffset([this, entry, place, first]()
        {
            if (!me->IsAlive())
                return;

            if (first)
                LeaderTalk(DATA_LADY_JAINA_PROUDMOORE, _felStorms == 1 ? uint8(SAY_JAINA_BEASTS_FROM_THE_FELFIRE) : _felStorms == 2 ? uint8(SAY_JAINA_PUT_THEM_ON_ICE) : uint8(SAY_JAINA_ANOTHER_WAVE));

            TempSummon* demon = me->SummonCreature(entry, place, TEMPSUMMON_CORPSE_TIMED_DESPAWN, KrosusDemonCorpseTime);
            if (!demon)
                return;

            demon->SendPlaySpellVisualKit(SPELL_VISUAL_KIT_SPIRE_OF_WOE_FIRE, 0, KrosusEruptionTime);
            DoZoneInCombat(demon);
        }, flight);
    }

    int32 FloorHealth() const
    {
        switch (_phase)
        {
            case PHASE_ALLIANCE:
                return KrosusToTheHordeHealth;
            case PHASE_HORDE:
                return KrosusToTheAllianceHealth;
            case PHASE_ALLIANCE_AGAIN:
                return KrosusToTheHordeAgainHealth;
            default:
                return 0;
        }
    }

    void LeaderTalk(uint32 creatureDataType, uint8 textGroup)
    {
        if (InstanceScript* instance = me->GetInstanceScript())
            if (Creature* leader = instance->GetCreature(creatureDataType))
                if (leader->IsAlive())
                    leader->AI()->Talk(textGroup);
    }

    // He is rooted where he fights (creature_static_flags_override), which is lifted while he goes
    void MoveTo(Position const& destination)
    {
        _destination = destination;
        _moving = true;
        _moveCheckTimer = KrosusMoveCheckInterval;
        _events.Reset();

        me->InterruptNonMeleeSpells(false);
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetSessile(false);
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(POINT_LEDGE, _destination, false, _destination.GetOrientation());
    }

    void UpdateMove(uint32 diff)
    {
        if (_moveCheckTimer > diff)
        {
            _moveCheckTimer -= diff;
            return;
        }

        _moveCheckTimer = KrosusMoveCheckInterval;
        if (me->GetExactDist2d(_destination) <= KrosusArriveDistance)
        {
            _moving = false;
            me->SetSessile(true);
            me->SetHomePosition(_destination);
            me->SetFacingTo(_destination.GetOrientation());
            Arrive();
        }
        else if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
            me->GetMotionMaster()->MovePoint(POINT_LEDGE, _destination, false, _destination.GetOrientation());
    }

    void Arrive()
    {
        switch (_phase)
        {
            case PHASE_ALLIANCE:
            case PHASE_ALLIANCE_AGAIN:
                LeaderTalk(DATA_KING_VARIAN_WRYNN, _phase == PHASE_ALLIANCE ? uint8(SAY_VARIAN_FOCUS_YOUR_FIRE) : uint8(SAY_VARIAN_WEVE_GOT_HIM));
                me->SetImmuneToAll(false);
                me->SetReactState(REACT_AGGRESSIVE);
                DoZoneInCombat();
                if (_phase == PHASE_ALLIANCE)
                    _events.ScheduleEvent(EVENT_SLAM, 10s);
                else
                {
                    _events.ScheduleEvent(EVENT_FEL_STORM, 1s);
                    _events.ScheduleEvent(EVENT_ORB_OF_DESTRUCTION, 12s);
                    _events.ScheduleEvent(EVENT_SLAM, 18s);
                }
                break;
            case PHASE_HORDE:
            case PHASE_HORDE_AGAIN:
                LeaderTalk(DATA_LADY_SYLVANAS_WINDRUNNER, _phase == PHASE_HORDE ? uint8(SAY_SYLVANAS_LETS_GO_HORDE) : uint8(SAY_SYLVANAS_JOB_DONE));
                _hordeDamageTimer = KrosusHordeDamageInterval;
                _hordeStrikeTimer = 0;
                _events.ScheduleEvent(EVENT_FEL_STORM, 2s);
                break;
            default:
                break;
        }
    }

    // With the Horde he loses health without anything hitting him, while its leaders make as if they did
    void UpdateHorde(uint32 diff)
    {
        if (_hordeStrikeTimer > diff)
            _hordeStrikeTimer -= diff;
        else
        {
            _hordeStrikeTimer = KrosusHordeStrikeInterval;
            for (uint32 entry : { NPC_LADY_SYLVANAS_WINDRUNNER, NPC_THRALL, NPC_BAINE_BLOODHOOF, NPC_VOLJIN })
            {
                Creature* leader = me->FindNearestCreature(entry, KrosusHordeLeaderDistance);
                if (!leader)
                    continue;

                leader->SetFacingToObject(me);
                leader->HandleEmoteCommand(entry == NPC_LADY_SYLVANAS_WINDRUNNER ? EMOTE_ONESHOT_ATTACK_BOW : EMOTE_ONESHOT_ATTACK1H);
            }
        }

        if (_hordeDamageTimer > diff)
        {
            _hordeDamageTimer -= diff;
            return;
        }

        _hordeDamageTimer = KrosusHordeDamageInterval;

        uint64 floor = std::max<uint64>(me->CountPctFromMaxHealth(FloorHealth()), 1);
        uint64 damage = me->CountPctFromMaxHealth(KrosusHordeDamage);
        if (me->GetHealth() > floor + damage)
        {
            me->SetHealth(me->GetHealth() - damage);
            return;
        }

        me->SetHealth(floor);
        LeaveOrDie();
    }

    void LeaveOrDie()
    {
        switch (_phase)
        {
            case PHASE_ALLIANCE:
            case PHASE_ALLIANCE_AGAIN:
                LeaderTalk(DATA_KING_VARIAN_WRYNN, _phase == PHASE_ALLIANCE ? uint8(SAY_VARIAN_HEADING_YOUR_WAY) : uint8(SAY_VARIAN_ANOTHER_SHOT));
                _phase = _phase == PHASE_ALLIANCE ? PHASE_HORDE : PHASE_HORDE_AGAIN;

                // The Alliance's forces do not follow him into the pool; players can still hit him
                me->Unit::SetImmuneToNPC(true, false);
                MoveTo(KrosusHordePosition);
                break;
            case PHASE_HORDE:
                LeaderTalk(DATA_LADY_SYLVANAS_WINDRUNNER, SAY_SYLVANAS_ALL_YOURS);
                _phase = PHASE_ALLIANCE_AGAIN;
                MoveTo(KrosusAlliancePosition);
                break;
            case PHASE_HORDE_AGAIN:
            {
                _phase = PHASE_DYING;
                _events.Reset();
                _felStormDemon = std::size(KrosusFelStormDemons);
                me->InterruptNonMeleeSpells(false);

                Unit* killer = ObjectAccessor::GetUnit(*me, _killer);
                Unit::Kill(killer ? killer : me, me);
                me->m_Events.AddEventAtOffset([me = me]()
                {
                    me->SendPlaySpellVisualKit(SPELL_VISUAL_KIT_SPIRE_OF_WOE_FIRE, 0, KrosusEruptionTime);
                }, KrosusEruptionDelay);
                me->DespawnOrUnsummon(KrosusCorpseTime);
                break;
            }
            default:
                break;
        }
    }

    EventMap _events;
    KrosusPhases _phase;
    Position _destination;
    ObjectGuid _killer;
    bool _moving;
    bool _reachedFloor;
    uint32 _moveCheckTimer;
    uint32 _hordeDamageTimer;
    uint32 _hordeStrikeTimer;
    uint32 _felStorms;
    std::size_t _felStormDemon;
    uint32 _felStormTimer;
    ObjectGuid _felStormStalker;
};

// 221080 - Fel Storm
// Krosus's. Every quarter of a second it would cast two spells that send a missile each, which the client
// has no spells for; his script sends the meteors.
class spell_broken_shore_krosus_fel_storm : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_broken_shore_krosus_fel_storm::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_broken_shore_krosus_fel_storm::HandlePeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

void AddSC_broken_shore_krosus()
{
    RegisterBrokenShoreCreatureAI(npc_broken_shore_krosus);
    RegisterSpellScript(spell_broken_shore_krosus_fel_storm);
}
