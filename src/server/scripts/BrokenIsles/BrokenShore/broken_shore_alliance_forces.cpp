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
#include "CellImpl.h"
#include "Creature.h"
#include "DB2Stores.h"
#include "GameObject.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "MotionMaster.h"
#include "MovementDefines.h"
#include "Random.h"
#include "ScriptedCreature.h"
#include "ThreatManager.h"
#include "broken_shore.h"
#include <algorithm>
#include <vector>

// How far away a fighter looks for a Spire of Woe that still stands
static constexpr float SpireSearchDistance = 300.0f;

// Each fighter has a direction of its own, which sets where it stands at a spire and from which side it comes
// at a demon. Without one they all take the same line to the same spot and end up standing in each other.
// Fighters that appear one after another get directions this far apart (the golden angle), which keeps any
// number of them spread out.
static constexpr float FighterDirectionStep = 2.3999632f;

// Where a fighter waits at a spire: on an arc this wide around it, on the side that players land on, and at
// a distance of its own between these two, which is outside the circle of its Anchoring Crystals
static constexpr float SpireStandArc = 5.0f * float(M_PI) / 6.0f;
static constexpr float SpireStandDistanceMin = 19.0f;
static constexpr float SpireStandDistanceMax = 27.0f;

// How near to its place at a spire a fighter has to be to stay put
static constexpr float SpireStandTolerance = 2.0f;

// The part of the circle around a demon that fighters in melee spread over: all of it but the quarter in
// front of the demon, where whoever it is fighting stands
static constexpr float MeleeArc = 3.0f * float(M_PI) / 2.0f;

// How far off its own side of a demon a fighter may stand before it moves there
static constexpr float FightAngleTolerance = float(M_PI) / 6.0f;

// How far around a spire a fighter looks for demons to fight
static constexpr float SpireFightDistance = 45.0f;

// How far from that place a fighter stands at least; each has a distance of its own from there on
static constexpr float RallyStandDistanceMin = 2.0f;

// A fighter takes a place on the way to where the forces gather for as long as it is at least this much
// farther from where they gather than that place is. It stands this far to its own side of the way.
static constexpr float RallyWayAhead = 5.0f;
static constexpr float RallyWayDistance = 3.0f;

// How near to where the forces gather before the gate of the Black City a fighter has to be, when they are
// sent into the city, to go through the gateway first, and how near to its place in the gateway it has to
// come
static constexpr float GateForcesDistance = 40.0f;
static constexpr float GatewayTolerance = 4.0f;

// How often a fighter that is not fighting looks for what to do next
static constexpr uint32 FighterThinkInterval = 1 * IN_MILLISECONDS;

// How far from its target a caster stands, and the arc in front of the target that casters spread over. What
// they fight faces whoever it is fighting, so that puts them in a line behind that fighter.
static constexpr float CasterDistance = 25.0f;
static constexpr float CasterArc = 2.0f * float(M_PI) / 3.0f;

// The chance that a leader yells when a fight starts, and the least time between two such yells
static constexpr int32 LeaderYellChance = 35;
static constexpr uint32 LeaderYellCooldown = 30 * IN_MILLISECONDS;

enum BrokenShoreFighterPoints
{
    POINT_SPIRE                         = 1,
    POINT_RALLY                         = 2
};

// 92074 - Alliance Priest
// 92122 - Gnomeregan Tinkerer
// 92123 - Stormwind Guard
// 97486 - Gilnean Royal Guard
// 114466 - Darnassus Sentinel
// The Alliance's forces. While the instance script has them charge they go for a Spire of Woe and fight the
// demons around it, and move on to the next when it has fallen. The Anchoring Crystals that hold it are
// left to players. When the demons' commander has come they go for him. Where no spire stands they fight
// the demons around them, and gather where the instance script has them when there are none.
// TODO: their weapons and abilities.
struct npc_broken_shore_alliance_soldier : public ScriptedAI
{
    npc_broken_shore_alliance_soldier(Creature* creature) : ScriptedAI(creature),
        _direction(Position::NormalizeOrientation(float(creature->GetGUID().GetCounter() % 1000) * FighterDirectionStep)),
        _standDistance(frand(SpireStandDistanceMin, SpireStandDistanceMax)), _thinkTimer(0), _rallyPoint(RALLY_POINT_NONE),
        _throughTheGateway(true) { }

    // A fighter picks its own fights
    void MoveInLineOfSight(Unit* /*who*/) override { }

    // When a fight ends a fighter stays where it is and carries on from there
    void EnterEvadeMode(EvadeReason why) override
    {
        _EnterEvadeMode(why);
    }

    // A fighter comes at a demon from its own side, so that several of them surround it
    void AttackStart(Unit* who) override
    {
        if (who && me->Attack(who, true))
            me->GetMotionMaster()->MoveChase(who, {}, ChaseAngle(float(M_PI) + (GetDirectionFraction() - 0.5f) * MeleeArc, FightAngleTolerance));
    }

    // On a way of several places a fighter goes on at once
    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_RALLY)
            _thinkTimer = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        if (UpdateVictim())
        {
            UpdateFight();
            return;
        }

        if (_thinkTimer > diff)
        {
            _thinkTimer -= diff;
            return;
        }

        _thinkTimer = FighterThinkInterval;

        if (!IsCharging())
            return;

        // Once the demons' commander is on the beach they all go for him
        if (Unit* commander = FindCommander())
        {
            TakeOn(commander);
            return;
        }

        if (GoThroughTheGateway())
            return;

        GameObject* spire = FindNextSpire();
        if (Unit* demon = FindNearestDemon(spire))
            TakeOn(demon);
        else if (spire)
        {
            Position place = GetPlaceAtSpire(spire);
            if (me->GetExactDist2d(place) > SpireStandTolerance && me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                me->GetMotionMaster()->MovePoint(POINT_SPIRE, place, true, place.GetAbsoluteAngle(spire));
        }
        else if (BrokenShoreRallyPoint const* rally = GetRallyPoint())
        {
            Position place = GetNextPlaceTo(*rally);
            if (me->GetExactDist2d(place) > SpireStandTolerance && me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
            {
                me->SetWalk(false);
                me->GetMotionMaster()->MovePoint(POINT_RALLY, place);
            }
        }
    }

protected:
    // In a fight the core does the hitting; this is for what else a fighter does
    virtual void UpdateFight() { }

    // Starts the fight with the demon the fighter has picked
    virtual void TakeOn(Unit* demon)
    {
        AttackStart(demon);
    }

    // The fighter's own direction as a part of a whole turn, from 0 to 1
    float GetDirectionFraction() const
    {
        return _direction / float(2 * M_PI);
    }

private:
    bool IsCharging() const
    {
        InstanceScript const* instance = me->GetInstanceScript();
        return instance && instance->GetData(DATA_ALLIANCE_FORCES_CHARGING);
    }

    // Dread Commander Arganoth, once he can be fought
    Unit* FindCommander() const
    {
        InstanceScript* instance = me->GetInstanceScript();
        Creature* commander = instance ? instance->GetCreature(DATA_DREAD_COMMANDER_ARGANOTH) : nullptr;
        return commander && commander->IsAlive() && me->IsValidAttackTarget(commander) ? commander : nullptr;
    }

    // The Spire of Woe to go for: of those that still stand, the one nearest to where players land, so
    // that all fighters go for the same spire and take them in the same order
    GameObject* FindNextSpire() const
    {
        std::vector<GameObject*> spires;
        me->GetGameObjectListWithEntryInGrid(spires, GO_SPIRE_OF_WOE, SpireSearchDistance);

        WorldSafeLocsEntry const* landing = sWorldSafeLocsStore.AssertEntry(WORLD_SAFE_LOC_ALLIANCE_BEACH);

        GameObject* next = nullptr;
        float nextDistance = 0.0f;
        for (GameObject* spire : spires)
        {
            if (spire->GetGoState() != GO_STATE_READY)
                continue;

            float distance = spire->GetExactDist2d(landing->Loc.X, landing->Loc.Y);
            if (!next || distance < nextDistance)
            {
                next = spire;
                nextDistance = distance;
            }
        }

        return next;
    }

    // A fighter that is before the gate of the Black City when the forces are sent into the city makes for
    // the gateway before anything else. Returns whether it is still on its way there.
    bool GoThroughTheGateway()
    {
        InstanceScript const* instance = me->GetInstanceScript();
        uint32 point = instance ? instance->GetData(DATA_ALLIANCE_RALLY_POINT) : uint32(RALLY_POINT_NONE);
        if (point != _rallyPoint)
        {
            BrokenShoreRallyPoint const& gate = AllianceRallyPoints[RALLY_POINT_BLACK_CITY_GATE];
            _rallyPoint = point;
            _throughTheGateway = point != RALLY_POINT_BLACK_CITY || me->GetExactDist2d(gate.X, gate.Y) > GateForcesDistance;
        }

        if (_throughTheGateway)
            return false;

        float x = BlackCityGateway.X + RallyWayDistance * std::cos(_direction);
        float y = BlackCityGateway.Y + RallyWayDistance * std::sin(_direction);
        if (me->GetExactDist2d(x, y) <= GatewayTolerance)
        {
            _throughTheGateway = true;
            return false;
        }

        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
        {
            me->SetWalk(false);
            me->GetMotionMaster()->MovePoint(POINT_RALLY, Position(x, y, me->GetMapHeight(x, y, BlackCityGateway.Z + 5.0f)));
        }

        return true;
    }

    // The place the instance script has the forces gather at, if there is one and the fighter is not too
    // far from it
    BrokenShoreRallyPoint const* GetRallyPoint() const
    {
        InstanceScript const* instance = me->GetInstanceScript();
        uint32 point = instance ? instance->GetData(DATA_ALLIANCE_RALLY_POINT) : uint32(RALLY_POINT_NONE);
        if (point == RALLY_POINT_NONE || point >= RALLY_POINT_MAX)
            return nullptr;

        BrokenShoreRallyPoint const& rally = AllianceRallyPoints[point];
        return me->GetExactDist2d(rally.X, rally.Y) <= rally.JoinDistance ? &rally : nullptr;
    }

    // Where the fighter goes next to get there: the first place on the way that is still ahead of it, if
    // there is a way, or its place there
    Position GetNextPlaceTo(BrokenShoreRallyPoint const& rally) const
    {
        if (&rally == &AllianceRallyPoints[RALLY_POINT_BLACK_CITY_GATE])
        {
            float left = me->GetExactDist2d(rally.X, rally.Y);
            for (BrokenShoreRallyPoint const& via : BlackCityGateWay)
            {
                float x = via.X + RallyWayDistance * std::cos(_direction);
                float y = via.Y + RallyWayDistance * std::sin(_direction);
                if (left > Position(x, y).GetExactDist2d(rally.X, rally.Y) + RallyWayAhead)
                    return Position(x, y, me->GetMapHeight(x, y, via.Z + 5.0f));
            }
        }

        return GetRallyPlace(rally);
    }

    // Where the fighter waits there: a few yards from it, on its own side
    Position GetRallyPlace(BrokenShoreRallyPoint const& rally) const
    {
        float distance = RallyStandDistanceMin + _standDistance - SpireStandDistanceMin;
        float x = rally.X + distance * std::cos(_direction);
        float y = rally.Y + distance * std::sin(_direction);
        return Position(x, y, me->GetMapHeight(x, y, rally.Z + 5.0f));
    }

    // Where the fighter waits at a spire, facing it
    Position GetPlaceAtSpire(GameObject* spire) const
    {
        WorldSafeLocsEntry const* landing = sWorldSafeLocsStore.AssertEntry(WORLD_SAFE_LOC_ALLIANCE_BEACH);

        float angle = spire->GetAbsoluteAngle(landing->Loc.X, landing->Loc.Y) + (GetDirectionFraction() - 0.5f) * SpireStandArc;
        return spire->GetFirstCollisionPosition(_standDistance, spire->ToRelativeAngle(angle));
    }

    // The demon nearest to the fighter among those around the spire, or around the fighter itself once
    // no spire stands. How far it looks then is up to the place the forces gather at.
    Unit* FindNearestDemon(GameObject* spire) const
    {
        WorldObject const* center = me;
        float fightDistance = SpireFightDistance;
        if (spire)
            center = spire;
        else if (BrokenShoreRallyPoint const* rally = GetRallyPoint())
            fightDistance = rally->FightDistance;

        std::vector<Unit*> units;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(center, me, fightDistance);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(center, units, check);
        Cell::VisitAllObjects(center, searcher, fightDistance);

        Unit* nearest = nullptr;
        float nearestDistance = 0.0f;
        for (Unit* unit : units)
        {
            if (unit->GetEntry() == NPC_ANCHORING_CRYSTAL || unit->GetEntry() == NPC_SHIELDED_ANCHOR || !me->IsValidAttackTarget(unit))
                continue;

            float distance = me->GetExactDist2d(unit);
            if (!nearest || distance < nearestDistance)
            {
                nearest = unit;
                nearestDistance = distance;
            }
        }

        return nearest;
    }

    float _direction;
    float _standDistance;
    uint32 _thinkTimer;
    uint32 _rallyPoint;
    bool _throughTheGateway;
};

// 90713 - King Varian Wrynn
// 90714 - Lady Jaina Proudmoore
// 90716 - Gelbin Mekkatorque
// 90717 - Genn Greymane
// The Alliance's leaders: fighters that yell now and then, and that hold the demons they attack.
// TODO: their abilities and Genn's worgen form.
struct npc_broken_shore_alliance_leader : public npc_broken_shore_alliance_soldier
{
    npc_broken_shore_alliance_leader(Creature* creature) : npc_broken_shore_alliance_soldier(creature), _yellTimer(0) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        if (_yellTimer || !roll_chance_i(LeaderYellChance))
            return;

        // Varian's first group is something else
        Talk(me->GetEntry() == NPC_KING_VARIAN_WRYNN ? uint8(SAY_VARIAN_IN_A_FIGHT) : uint8(SAY_LEADER_IN_A_FIGHT));
        _yellTimer = LeaderYellCooldown;
    }

    void UpdateAI(uint32 diff) override
    {
        _yellTimer -= std::min(_yellTimer, diff);

        npc_broken_shore_alliance_soldier::UpdateAI(diff);
    }

protected:
    // A leader holds what it attacks: the demon is given as much threat as it has health, which is more
    // than a player's damage alone adds up to. On retail players are told to let the leaders tank; how
    // they hold a demon there is not known.
    void TakeOn(Unit* demon) override
    {
        demon->GetThreatManager().AddThreat(me, float(demon->GetMaxHealth()), nullptr, true, true);
        npc_broken_shore_alliance_soldier::TakeOn(demon);
    }

private:
    uint32 _yellTimer;
};

// 110615 - Argent Dawnbringer
// Caged in and before the Black City, or dead there. One that a player frees joins the Alliance's forces
// and fights the city's demons with them. That it runs into the city to fight is from a video of the
// retail scenario.
struct npc_broken_shore_argent_dawnbringer : public npc_broken_shore_alliance_soldier
{
    npc_broken_shore_argent_dawnbringer(Creature* creature) : npc_broken_shore_alliance_soldier(creature), _freed(false) { }

    void DoAction(int32 action) override
    {
        if (action != ACTION_DAWNBRINGER_FREED || _freed)
            return;

        _freed = true;
        me->SetFaction(FACTION_BROKEN_SHORE_ALLIANCE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_freed)
            npc_broken_shore_alliance_soldier::UpdateAI(diff);
    }

private:
    bool _freed;
};

// 91353, 97496 - Kirin Tor Battle-Mage
// 110627 - Gilnean Druid
// Fighters that keep their distance and cast one spell at what they fight: Frostfire Bolt for the mages,
// Wrath for the druids.
struct npc_broken_shore_alliance_caster : public npc_broken_shore_alliance_soldier
{
    npc_broken_shore_alliance_caster(Creature* creature) : npc_broken_shore_alliance_soldier(creature),
        _spellId(creature->GetEntry() == NPC_GILNEAN_DRUID ? SPELL_WRATH : SPELL_FROSTFIRE_BOLT) { }

    void AttackStart(Unit* who) override
    {
        if (who && me->Attack(who, false))
            me->GetMotionMaster()->MoveChase(who, ChaseRange(CasterDistance), ChaseAngle((GetDirectionFraction() - 0.5f) * CasterArc, FightAngleTolerance));
    }

protected:
    void UpdateFight() override
    {
        DoSpellAttackIfReady(_spellId);
    }

private:
    uint32 _spellId;
};

void AddSC_broken_shore_alliance_forces()
{
    RegisterBrokenShoreCreatureAI(npc_broken_shore_alliance_soldier);
    RegisterBrokenShoreCreatureAI(npc_broken_shore_alliance_leader);
    RegisterBrokenShoreCreatureAI(npc_broken_shore_alliance_caster);
    RegisterBrokenShoreCreatureAI(npc_broken_shore_argent_dawnbringer);
}
