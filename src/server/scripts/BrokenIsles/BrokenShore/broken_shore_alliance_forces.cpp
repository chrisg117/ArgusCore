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
    POINT_SPIRE                         = 1
};

// 97486 - Gilnean Royal Guard
// 114466 - Darnassus Sentinel
// The Alliance's forces on the beach. While the instance script has them charge they go for a Spire of Woe
// and fight the demons around it, and move on to the next when it has fallen. The Anchoring Crystals that
// hold it are left to players.
// TODO: their weapons and abilities, and what they do in the stages after the beach.
struct npc_broken_shore_alliance_soldier : public ScriptedAI
{
    npc_broken_shore_alliance_soldier(Creature* creature) : ScriptedAI(creature),
        _direction(Position::NormalizeOrientation(float(creature->GetGUID().GetCounter() % 1000) * FighterDirectionStep)),
        _standDistance(frand(SpireStandDistanceMin, SpireStandDistanceMax)), _thinkTimer(0) { }

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

        GameObject* spire = FindNextSpire();
        if (Unit* demon = FindNearestDemon(spire))
            TakeOn(demon);
        else if (spire)
        {
            Position place = GetPlaceAtSpire(spire);
            if (me->GetExactDist2d(place) > SpireStandTolerance && me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                me->GetMotionMaster()->MovePoint(POINT_SPIRE, place, true, place.GetAbsoluteAngle(spire));
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

    // Where the fighter waits at a spire, facing it
    Position GetPlaceAtSpire(GameObject* spire) const
    {
        WorldSafeLocsEntry const* landing = sWorldSafeLocsStore.AssertEntry(WORLD_SAFE_LOC_ALLIANCE_BEACH);

        float angle = spire->GetAbsoluteAngle(landing->Loc.X, landing->Loc.Y) + (GetDirectionFraction() - 0.5f) * SpireStandArc;
        return spire->GetFirstCollisionPosition(_standDistance, spire->ToRelativeAngle(angle));
    }

    // The demon nearest to the fighter among those around the spire, or around the fighter itself once
    // no spire stands
    Unit* FindNearestDemon(GameObject* spire) const
    {
        WorldObject const* center = me;
        if (spire)
            center = spire;

        std::vector<Unit*> units;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(center, me, SpireFightDistance);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(center, units, check);
        Cell::VisitAllObjects(center, searcher, SpireFightDistance);

        Unit* nearest = nullptr;
        float nearestDistance = 0.0f;
        for (Unit* unit : units)
        {
            if (unit->GetEntry() == NPC_ANCHORING_CRYSTAL || !me->IsValidAttackTarget(unit))
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
};

// 90714 - Lady Jaina Proudmoore
// 90717 - Genn Greymane
// The Alliance's leaders on the beach: fighters that yell now and then, and that hold the demons they attack.
// TODO: their abilities and Genn's worgen form.
struct npc_broken_shore_alliance_leader : public npc_broken_shore_alliance_soldier
{
    npc_broken_shore_alliance_leader(Creature* creature) : npc_broken_shore_alliance_soldier(creature), _yellTimer(0) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        if (_yellTimer || !roll_chance_i(LeaderYellChance))
            return;

        Talk(SAY_LEADER_IN_A_FIGHT);
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
}
