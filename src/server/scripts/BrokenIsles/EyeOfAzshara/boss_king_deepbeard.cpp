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
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "eye_of_azshara.h"

enum DeepbeardSpells
{
    SPELL_GASEOUS_BUBBLES               = 193018,   // absorb on up to 2 players, 20 s
    SPELL_GASEOUS_EXPLOSION             = 193047,
    SPELL_QUAKE                         = 193152,   // 100 energy; force-casts 193153 (a Quake stalker) on players
    SPELL_QUAKE_DAMAGE                  = 193159,
    SPELL_AFTERSHOCK                    = 193167,   // 4 s, then 193171
    SPELL_CALL_THE_SEAS                 = 193051,   // the stalkers run 193054 (area trigger 4968)
    SPELL_CALL_THE_SEAS_DAMAGE          = 193055,
    SPELL_GROUND_SLAM                   = 193093,   // dummy naming 193056
    SPELL_GROUND_SLAM_MISSILE           = 193056
};

enum DeepbeardTalks
{
    TALK_AGGRO                          = 0,    // 102315, 102316
    TALK_CALL_THE_SEAS                  = 1,    // 102303
    TALK_GASEOUS_BUBBLES                = 2,    // 102304, 102305, 102306
    TALK_QUAKE                          = 3,    // 102307, 102308
    TALK_KILL                           = 4,    // 102313, 102314
    TALK_WIPE                           = 5,    // 102317
    TALK_DEATH_WINDS                    = 6,    // 102309, 102310: Serpentrix (the winds) still stands
    TALK_DEATH_SHE                      = 7,    // 102311, 102312: the Wrath is all that is left
    EMOTE_CALL_THE_SEAS                 = 8     // 99824
};

enum DeepbeardEvents
{
    EVENT_GASEOUS_BUBBLES               = 1,
    EVENT_QUAKE,
    EVENT_CALL_THE_SEAS,
    EVENT_GROUND_SLAM,
    EVENT_ENERGY
};

namespace
{
    // Calibrate: Call the Seas zone radius (area trigger 4968 has no client shape).
    float constexpr CallTheSeasRadius = 4.0f;

    bool IsNormalDifficulty(Unit const* unit)
    {
        return unit->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL;
    }

    // Quake stalkers are the players' summons (193153 is force-cast on them), so the boss's summon list
    // does not hold them.
    void DespawnQuakes(WorldObject const* center)
    {
        std::list<Creature*> quakes;
        center->GetCreatureListWithEntryInGrid(quakes, NPC_QUAKE, 150.0f);
        for (Creature* quake : quakes)
            quake->DespawnOrUnsummon();
    }

    void StopCallTheSeas(WorldObject const* center)
    {
        std::list<Creature*> stalkers;
        center->GetCreatureListWithEntryInGrid(stalkers, NPC_CALL_THE_SEAS, 150.0f);
        for (Creature* stalker : stalkers)
        {
            stalker->RemoveAllAuras();
            stalker->RemoveAllAreaTriggers();
        }
    }
}

// 91797 - King Deepbeard
// Timers follow DBM 7.3.31 (DBM-Party-Legion/EyeOfAzshara/KingDeepbeard.lua): Gaseous Bubbles 10 s and every 32 s,
// Quake 15 s and every 21.8 s, Call the Seas 20 s and every 30 s (on its cast success), Ground Slam every 18.2 s
// (no first bar: the module's 6 s start is commented out, used here as the first cast).
struct boss_king_deepbeard : public BossAI
{
    boss_king_deepbeard(Creature* creature) : BossAI(creature, DATA_KING_DEEPBEARD) { }

    void JustAppeared() override
    {
        // Cosmetic energy bar filling towards each Quake, which costs 100.
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 0);
    }

    void Reset() override
    {
        _Reset();
        _energyTicks = 0;
        _quakeInterval = 15.0f;
        me->SetPower(POWER_ENERGY, 0);
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        Talk(TALK_AGGRO);

        events.ScheduleEvent(EVENT_GROUND_SLAM, 6s);
        events.ScheduleEvent(EVENT_GASEOUS_BUBBLES, 10s);
        events.ScheduleEvent(EVENT_QUAKE, 15s);
        events.ScheduleEvent(EVENT_CALL_THE_SEAS, 20s);
        events.ScheduleEvent(EVENT_ENERGY, 1s);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(TALK_KILL);
    }

    void CleanUp()
    {
        DespawnQuakes(me);
        StopCallTheSeas(me);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GASEOUS_BUBBLES);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CALL_THE_SEAS_DAMAGE);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        Talk(TALK_WIPE);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanUp();
        _EnterEvadeMode();
        _DespawnAtEvade();
    }

    void JustDied(Unit* /*killer*/) override
    {
        Talk(instance->GetBossState(DATA_SERPENTRIX) == DONE ? TALK_DEATH_SHE : TALK_DEATH_WINDS);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanUp();
        _JustDied();
    }

    void OnSpellCast(SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_QUAKE)
        {
            _energyTicks = 0;
            _quakeInterval = 21.8f;
            me->SetPower(POWER_ENERGY, 0);
        }
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
                case EVENT_GASEOUS_BUBBLES:
                    Talk(TALK_GASEOUS_BUBBLES);
                    DoCastSelf(SPELL_GASEOUS_BUBBLES);
                    events.Repeat(32s);
                    break;
                case EVENT_QUAKE:
                    Talk(TALK_QUAKE);
                    me->SetPower(POWER_ENERGY, 100);
                    DoCastSelf(SPELL_QUAKE);
                    events.Repeat(21800ms);
                    break;
                case EVENT_CALL_THE_SEAS:
                    Talk(TALK_CALL_THE_SEAS);
                    Talk(EMOTE_CALL_THE_SEAS);
                    DoCastSelf(SPELL_CALL_THE_SEAS);
                    events.Repeat(30s);
                    break;
                case EVENT_GROUND_SLAM:
                    // The data's cast time is 3.5 s on Normal, 3 s on Heroic, 2.5 s on Mythic.
                    DoCastVictim(SPELL_GROUND_SLAM);
                    events.Repeat(18200ms);
                    break;
                case EVENT_ENERGY:
                    ++_energyTicks;
                    me->SetPower(POWER_ENERGY, std::min<int32>(100, int32(float(_energyTicks) * 100.0f / _quakeInterval)));
                    events.Repeat(1s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    uint32 _energyTicks = 0;
    float _quakeInterval = 15.0f;
};

// 97916 - Quake (a stalker at a player's feet for 8 s): the quake, then the aftershock at the same spot.
struct npc_deepbeard_quake : public ScriptedAI
{
    npc_deepbeard_quake(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetUninteractible(true);
        DoCastSelf(SPELL_QUAKE_DAMAGE, true);
        DoCastSelf(SPELL_AFTERSHOCK, true);
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

// 193018 - Gaseous Bubbles: a shield that was not used up explodes on its bearer's group when it ends.
// Normal: the explosion's own damage. Heroic and Mythic: the absorb left (journal section 12686).
class spell_deepbeard_gaseous_bubbles : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_GASEOUS_EXPLOSION });
    }

    void HandleRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        int32 const left = aurEff->GetAmount();
        if (left <= 0)
            return;

        Unit* bearer = GetTarget();
        if (!bearer->IsAlive())
            return;

        CastSpellExtraArgs args(TRIGGERED_FULL_MASK);
        args.SetOriginalCaster(GetCasterGUID());
        if (!IsNormalDifficulty(bearer))
            args.AddSpellMod(SPELLVALUE_BASE_POINT0, left);
        bearer->CastSpell(bearer, SPELL_GASEOUS_EXPLOSION, args);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_deepbeard_gaseous_bubbles::HandleRemove, EFFECT_0, SPELL_AURA_SCHOOL_ABSORB, AURA_EFFECT_HANDLE_REAL);
    }
};

// 193093 - Ground Slam: its dummy's value names 193056, which throws 193088 at a point 14 yd in front.
class spell_deepbeard_ground_slam : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_GROUND_SLAM_MISSILE });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        GetCaster()->CastSpell(GetCaster(), SPELL_GROUND_SLAM_MISSILE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_deepbeard_ground_slam::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

struct CallTheSeasCheck
{
    bool operator()(AreaTrigger* at) const { return at->GetSpellId() == 193054; }
};

// 4968 - Call the Seas (on each stalker for 20 s): the zone's damage while a player stands in it.
struct at_deepbeard_call_the_seas : AreaTriggerAI
{
    at_deepbeard_call_the_seas(AreaTrigger* at) : AreaTriggerAI(at) { }

    void OnUnitEnter(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        // The 35 stalkers stay friendly (faction 35: a hostile trigger would join the fight); the damage comes
        // from Deepbeard himself, whose 193055 reaches anywhere.
        if (InstanceScript* instance = at->GetInstanceScript())
            if (Creature* deepbeard = instance->GetCreature(DATA_KING_DEEPBEARD))
                if (deepbeard->IsAlive() && deepbeard->IsEngaged())
                    deepbeard->CastSpell(unit, SPELL_CALL_THE_SEAS_DAMAGE, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void OnUnitExit(Unit* unit) override
    {
        if (!unit->IsPlayer())
            return;

        std::list<AreaTrigger*> zones;
        CallTheSeasCheck check;
        Trinity::AreaTriggerListSearcher<CallTheSeasCheck> searcher(at, zones, check);
        Cell::VisitAllObjects(at, searcher, CallTheSeasRadius * 4.0f);
        for (AreaTrigger* other : zones)
            if (other != at && other->GetInsideUnits().contains(unit->GetGUID()))
                return;

        unit->RemoveAurasDueToSpell(SPELL_CALL_THE_SEAS_DAMAGE);
    }
};

void AddSC_boss_king_deepbeard()
{
    RegisterEyeOfAzsharaCreatureAI(boss_king_deepbeard);
    RegisterEyeOfAzsharaCreatureAI(npc_deepbeard_quake);
    RegisterSpellScript(spell_deepbeard_gaseous_bubbles);
    RegisterSpellScript(spell_deepbeard_ground_slam);
    RegisterAreaTriggerAI(at_deepbeard_call_the_seas);
}
