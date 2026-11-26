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

#include "CellImpl.h"
#include "GridNotifiersImpl.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellAuras.h"
#include "SpellScript.h"
#include "eye_of_azshara.h"
#include <array>
#include <set>

enum HatecoilSpells
{
    SPELL_STATIC_NOVA                   = 193597,
    SPELL_FOCUSED_LIGHTNING             = 193611,
    SPELL_BECKON_STORM                  = 193682,
    SPELL_BECKON_STORM_SUMMON           = 193683,   // one Saltsea Globule 5-10 yd from the target
    SPELL_WATERY_SPLASH                 = 193636,
    SPELL_CURSE_ONE                     = 193712,   // 1 target
    SPELL_CURSE_THREE                   = 193716,   // 3 targets
    SPELL_CURSE_FIVE                    = 193717,   // 5 targets
    SPELL_CURSE_OF_THE_WITCH            = 193698,   // the 6 s curse
    SPELL_CURSE_RELEASE_KNOCKBACK       = 193700,   // 30 yd cone, players
    SPELL_CURSE_RELEASE_KILL            = 193720,   // 40 yd cone, Saltsea Globules
    SPELL_MONSOON_SELECTOR              = 196634,   // up to 3 players within 80 yd
    SPELL_MONSOON_SUMMON                = 196632,
    SPELL_MONSOON_KNOCKBACK             = 196610,
    SPELL_LIGHTNING_ARENA               = 197325,
    SPELL_CRACKLING_THUNDER             = 197326
};

enum HatecoilTalks
{
    TALK_AGGRO                          = 0,    // 100227
    TALK_STATIC_NOVA                    = 1,    // 100236
    TALK_FOCUSED_LIGHTNING              = 2,    // 100235
    TALK_BECKON_STORM                   = 3,    // 100245, 100237
    TALK_CURSE                          = 4,    // 100244
    TALK_MONSOON                        = 5,    // 100228
    TALK_KILL                           = 6,    // 100230, 100231
    TALK_WIPE                           = 7,    // 100463
    TALK_DEATH                          = 8     // 100458
};

enum HatecoilEvents
{
    EVENT_STATIC_NOVA                   = 1,
    EVENT_FOCUSED_LIGHTNING,
    EVENT_BECKON_STORM,
    EVENT_CURSE,
    EVENT_MONSOON,
    EVENT_ARENA_CHECK
};

namespace
{
    // The arena: the Sand Dune box (x -3479..-3402, y 4552..4620) plus a margin. Calibrate.
    float constexpr ArenaMinX = -3495.0f, ArenaMaxX = -3385.0f;
    float constexpr ArenaMinY = 4535.0f, ArenaMaxY = 4640.0f;

    // A player within this distance of a standing Sand Dune is on it, sheltered from Static Nova. Calibrate.
    float constexpr DuneShelterRadius = 4.0f;

    // DBM's module comment: the curse sequence it saw, with the gap after each cast. Looped.
    struct CurseStep { uint32 spell; Milliseconds next; };
    std::array<CurseStep, 6> constexpr CurseSequence =
    {{
        { SPELL_CURSE_ONE,   18000ms },
        { SPELL_CURSE_THREE, 24500ms },
        { SPELL_CURSE_ONE,   16500ms },
        { SPELL_CURSE_FIVE,  30000ms },
        { SPELL_CURSE_THREE, 17000ms },
        { SPELL_CURSE_FIVE,  19000ms }
    }};

    bool InArena(WorldObject const* obj)
    {
        return obj->GetPositionX() >= ArenaMinX && obj->GetPositionX() <= ArenaMaxX
            && obj->GetPositionY() >= ArenaMinY && obj->GetPositionY() <= ArenaMaxY;
    }

    bool IsNormalDifficulty(Unit const* unit)
    {
        return unit->GetMap()->GetDifficultyID() == DIFFICULTY_NORMAL;
    }

    std::list<Creature*> Dunes(WorldObject const* center, bool alive)
    {
        std::list<Creature*> dunes;
        center->GetCreatureListWithEntryInGrid(dunes, NPC_SAND_DUNE, 150.0f);
        dunes.remove_if([alive](Creature* dune) { return dune->IsAlive() != alive; });
        return dunes;
    }

    bool OnDune(Unit const* unit)
    {
        std::list<Creature*> dunes;
        unit->GetCreatureListWithEntryInGrid(dunes, NPC_SAND_DUNE, DuneShelterRadius + 5.0f);
        for (Creature* dune : dunes)
            if (dune->IsAlive() && unit->GetExactDist2d(dune) <= DuneShelterRadius)
                return true;
        return false;
    }

    // A dune is blown away by killing its stalker: the stalker's linked-summon aura (193060) takes the dune's
    // body (object 244690) with it. Dunes spawn with a 2 h respawn, so they stay gone for the run until restored.
    void BlowAway(Creature* dune)
    {
        if (dune->IsAlive())
            dune->KillSelf();
    }

    // A blown dune whose corpse has decayed is no longer in the grid, so dunes are restored by spawn id: a corpse
    // still there is respawned in place, a removed one through the map's respawn of that spawn.
    void RestoreDunes(Map* map, std::set<ObjectGuid::LowType> const& spawnIds)
    {
        for (ObjectGuid::LowType spawnId : spawnIds)
        {
            auto bounds = map->GetCreatureBySpawnIdStore().equal_range(spawnId);
            bool alive = false;
            Creature* corpse = nullptr;
            for (auto itr = bounds.first; itr != bounds.second; ++itr)
            {
                if (itr->second->IsAlive())
                    alive = true;
                else
                    corpse = itr->second;
            }

            if (alive)
                continue;
            if (corpse)
                corpse->Respawn(true);
            else
                map->Respawn(SPAWN_TYPE_CREATURE, spawnId);
        }
    }
}

// 91789 - Lady Hatecoil
// Timers follow DBM 7.3.31 (DBM-Party-Legion/EyeOfAzshara/Hatecoil.lua): Static Nova 10.5 s after the pull and
// every 34 s, Focused Lightning 15.5 s after each Static Nova's cast start, Beckon Storm 19 s and every 47 s.
// The curse has no timer; the module's comment sequence is used (calibrate the first cast: 15 s).
struct boss_lady_hatecoil : public BossAI
{
    boss_lady_hatecoil(Creature* creature) : BossAI(creature, DATA_LADY_HATECOIL) { }

    void JustAppeared() override
    {
        // A respawn after a wipe reloads the addon's single shield stack: the instance sets it from the count.
        instance->SetData(DATA_ARCANISTS, 0);
    }

    void Reset() override
    {
        _Reset();
        _curseStep = 0;
    }

    void JustEngagedWith(Unit* who) override
    {
        BossAI::JustEngagedWith(who);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1);
        Talk(TALK_AGGRO);

        // Remember every dune standing at the pull, to bring each back by spawn id.
        for (Creature* dune : Dunes(me, true))
            _duneSpawns.insert(dune->GetSpawnId());

        events.ScheduleEvent(EVENT_STATIC_NOVA, 10500ms);
        events.ScheduleEvent(EVENT_BECKON_STORM, 19s);
        events.ScheduleEvent(EVENT_CURSE, 15s);
        if (!IsNormalDifficulty(me))
            events.ScheduleEvent(EVENT_MONSOON, 25s);   // calibrate: no DBM timer
        events.ScheduleEvent(EVENT_ARENA_CHECK, 1s);
    }

    void KilledUnit(Unit* victim) override
    {
        if (victim->IsPlayer())
            Talk(TALK_KILL);
    }

    void CleanUp()
    {
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_LIGHTNING_ARENA);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CRACKLING_THUNDER);
        instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_CURSE_OF_THE_WITCH);
        RestoreDunes(me->GetMap(), _duneSpawns);
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
        Talk(TALK_DEATH);
        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
        CleanUp();
        _JustDied();
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
                // Static Nova and Focused Lightning share category 1152 (10 s), the curses 1159: a cast refused by a
                // category cooldown is tried again 0.5 s later instead of being lost.
                case EVENT_STATIC_NOVA:
                    if (DoCastSelf(SPELL_STATIC_NOVA) != SPELL_CAST_OK)
                    {
                        events.Repeat(500ms);
                        break;
                    }
                    Talk(TALK_STATIC_NOVA);
                    // DBM starts the Focused Lightning bar on Static Nova's cast start.
                    events.ScheduleEvent(EVENT_FOCUSED_LIGHTNING, 15500ms);
                    events.Repeat(34s);
                    break;
                case EVENT_FOCUSED_LIGHTNING:
                    if (DoCastSelf(SPELL_FOCUSED_LIGHTNING) != SPELL_CAST_OK)
                    {
                        events.Repeat(500ms);
                        break;
                    }
                    Talk(TALK_FOCUSED_LIGHTNING);
                    break;
                case EVENT_BECKON_STORM:
                    Talk(TALK_BECKON_STORM);
                    DoCastSelf(SPELL_BECKON_STORM);
                    events.Repeat(47s);
                    break;
                case EVENT_CURSE:
                {
                    CurseStep const& step = CurseSequence[_curseStep];
                    if (DoCastSelf(step.spell) != SPELL_CAST_OK)
                    {
                        events.Repeat(500ms);
                        break;
                    }
                    _curseStep = (_curseStep + 1) % CurseSequence.size();
                    Talk(TALK_CURSE);
                    events.Repeat(step.next);
                    break;
                }
                case EVENT_MONSOON:
                    Talk(TALK_MONSOON);
                    DoCastSelf(SPELL_MONSOON_SELECTOR);
                    events.Repeat(30s);    // calibrate
                    break;
                case EVENT_ARENA_CHECK:
                    UpdateArena();
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
    // Players inside the arena carry Lightning Arena; one in the fight who steps out is struck by Crackling
    // Thunder (stacking +25%) every 2 s until they return.
    void UpdateArena()
    {
        ++_arenaTicks;
        for (auto const& ref : me->GetThreatManager().GetUnsortedThreatList())
        {
            Player* player = ref->GetVictim()->ToPlayer();
            if (!player || !player->IsAlive())
                continue;

            if (InArena(player))
            {
                if (!player->HasAura(SPELL_LIGHTNING_ARENA))
                    me->AddAura(SPELL_LIGHTNING_ARENA, player);
            }
            else
            {
                player->RemoveAurasDueToSpell(SPELL_LIGHTNING_ARENA);
                if (_arenaTicks % 2 == 0)
                    DoCast(player, SPELL_CRACKLING_THUNDER, true);
            }
        }
    }

    std::set<ObjectGuid::LowType> _duneSpawns;
    std::size_t _curseStep = 0;
    uint32 _arenaTicks = 0;
};

// 98293 - Saltsea Globule
struct npc_hatecoil_saltsea_globule : public ScriptedAI
{
    npc_hatecoil_saltsea_globule(Creature* creature) : ScriptedAI(creature) { }

    void JustDied(Unit* /*killer*/) override
    {
        DoCastSelf(SPELL_WATERY_SPLASH, true);
    }
};

// 99852 - Monsoon (Heroic, Mythic)
// Chases the nearest player; a player it reaches is launched (196610), and a dune it passes over is blown away.
struct npc_hatecoil_monsoon : public ScriptedAI
{
    npc_hatecoil_monsoon(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
        me->SetUninteractible(true);
        _checkTimer = 500;
    }

    void UpdateAI(uint32 diff) override
    {
        if (_checkTimer > diff)
        {
            _checkTimer -= diff;
            return;
        }
        _checkTimer = 500;

        Player* nearest = me->SelectNearestPlayer(80.0f);
        if (!nearest)
            return;

        if (me->GetExactDist2d(nearest) <= 2.0f)
        {
            if (!_launched.contains(nearest->GetGUID()))
            {
                _launched.insert(nearest->GetGUID());
                DoCast(nearest, SPELL_MONSOON_KNOCKBACK, true);
            }
        }
        else
            me->GetMotionMaster()->MovePoint(1, nearest->GetPosition(), true);

        std::list<Creature*> dunes;
        me->GetCreatureListWithEntryInGrid(dunes, NPC_SAND_DUNE, 3.0f);
        for (Creature* dune : dunes)
            BlowAway(dune);
    }

private:
    uint32 _checkTimer = 500;
    GuidSet _launched;
};

// 193597 - Static Nova: players standing on a Sand Dune are sheltered.
class spell_hatecoil_static_nova : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([](WorldObject* target)
        {
            Unit* unit = target->ToUnit();
            return unit && OnDune(unit);
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hatecoil_static_nova::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hatecoil_static_nova::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

// 193625 - Excess Lightning (6 yd dummy around each player struck by Focused Lightning): blows away dunes.
class spell_hatecoil_excess_lightning : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Creature* dune = Object::ToCreature(GetHitUnit()))
            if (dune->GetEntry() == NPC_SAND_DUNE)
                BlowAway(dune);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hatecoil_excess_lightning::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 193682 - Beckon Storm: a Saltsea Globule beside each player it reaches.
class spell_hatecoil_beckon_storm : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_BECKON_STORM_SUMMON });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, SPELL_BECKON_STORM_SUMMON, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hatecoil_beckon_storm::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 193712, 193716, 193717 - Curse of the Witch (1, 3 or 5 targets): curse each one.
class spell_hatecoil_curse_selector : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CURSE_OF_THE_WITCH });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target, SPELL_CURSE_OF_THE_WITCH, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hatecoil_curse_selector::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 193698 - Curse of the Witch: when it runs out, its bearer releases a frontal cone that knocks players back
// (193700) and destroys Saltsea Globules (193720).
class spell_hatecoil_curse_of_the_witch : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CURSE_RELEASE_KNOCKBACK, SPELL_CURSE_RELEASE_KILL });
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        Unit* target = GetTarget();
        if (!target->IsAlive())
            return;

        target->CastSpell(target, SPELL_CURSE_RELEASE_KNOCKBACK, CastSpellExtraArgs(TRIGGERED_FULL_MASK).SetOriginalCaster(GetCasterGUID()));
        target->CastSpell(target, SPELL_CURSE_RELEASE_KILL, CastSpellExtraArgs(TRIGGERED_FULL_MASK).SetOriginalCaster(GetCasterGUID()));
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hatecoil_curse_of_the_witch::HandleRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 196634 - Monsoon (Heroic, Mythic): a Monsoon where each chosen player stands.
class spell_hatecoil_monsoon_selector : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_MONSOON_SUMMON });
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            GetCaster()->CastSpell(target->GetPosition(), SPELL_MONSOON_SUMMON, CastSpellExtraArgs(TRIGGERED_FULL_MASK));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hatecoil_monsoon_selector::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_boss_lady_hatecoil()
{
    RegisterEyeOfAzsharaCreatureAI(boss_lady_hatecoil);
    RegisterEyeOfAzsharaCreatureAI(npc_hatecoil_saltsea_globule);
    RegisterEyeOfAzsharaCreatureAI(npc_hatecoil_monsoon);
    RegisterSpellScript(spell_hatecoil_static_nova);
    RegisterSpellScript(spell_hatecoil_excess_lightning);
    RegisterSpellScript(spell_hatecoil_beckon_storm);
    RegisterSpellScript(spell_hatecoil_curse_selector);
    RegisterSpellScript(spell_hatecoil_curse_of_the_witch);
    RegisterSpellScript(spell_hatecoil_monsoon_selector);
}
