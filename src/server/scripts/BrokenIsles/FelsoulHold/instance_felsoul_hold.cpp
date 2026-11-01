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
#include "CreatureAI.h"
#include "EventMap.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "GameEventSender.h"
#include "InstanceScript.h"
#include "Item.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "DB2Structure.h"
#include "InstanceScenario.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "TemporarySummon.h"
#include "SpellMgr.h"
#include "SpellScript.h"

#define DataHeader "FelsoulHold"

enum FelsoulHoldData
{
    MAP_ACQUISITION_HAVOC                   = 1498,
    MAP_BROKEN_ISLES                        = 1220,
    WORLD_SAFE_LOC_FELSOUL_HOLD_GRAVEYARD   = 5097,
    WORLD_SAFE_LOC_VAREDIS_GRAVEYARD        = 5362,

    SCENARIO_STEP_STALKING_YOUR_PREY        = 1808,
    SCENARIO_STEP_THEY_ARE_NOT_PREPARED     = 1821,
    SCENARIO_STEP_UNLEASHED_HAVOC           = 1825,

    GAME_EVENT_FLY_INTO_FELSOUL_HOLD        = 45563,
    GAME_EVENT_WARD_DESTROYED               = 45698,
    GAME_EVENT_FIRST_WARD                   = 45760,
    GAME_EVENT_SECOND_WARD                  = 45761,
    GAME_EVENT_THIRD_WARD                   = 45762,
    GAME_EVENT_VAREDIS_SLAIN                = 45717,

    // What a kill adds to the bar of "Absorb demon souls", of 1000
    GAME_EVENT_SOUL_225                     = 54742,
    GAME_EVENT_SOUL_85                      = 54743,
    GAME_EVENT_SOUL_50                      = 54744,
    GAME_EVENT_SOUL_35                      = 54745,

    SPELL_TELEPORT_TO_FELSOUL_HOLD          = 188922,
    SPELL_DH_CAMP_EXPLOSION_SCENE           = 186760,
    SPELL_DH_CAMP_EXPLOSION_EVENT           = 186771,
    SPELL_VAREDIS_REVEAL                    = 189475,
    SPELL_DEMONS_BITE                       = 190226,
    SPELL_CHAOS_NOVA                        = 190246,
    SPELL_SHADOW_STRIKE                     = 189526,
    SPELL_FEL_RUSH                          = 202817,
    SPELL_BLUR                              = 202818,

    SPELL_TWINBLADES_OF_THE_DECEIVER        = 189524,
    SPELL_WIELD_THE_TWINBLADES              = 218936,

    GO_TWINBLADES_OF_THE_DECEIVER           = 251878,
    GO_GLAIVE                               = 243267,

    ITEM_TWINBLADES_OF_THE_DECEIVER         = 127829,
    GO_FELSHIELD_WARD_FIRST                 = 243428,
    GO_FELSHIELD_WARD_SECOND                = 243472,
    GO_FELSHIELD_WARD_THIRD                 = 243473,

    NPC_FELSOUL_STALKER                     = 94375,
    NPC_FELSOUL_CHAOSWEAVER                 = 95313,
    NPC_FELSOUL_BERSERKER                   = 95423,
    NPC_FEL_COMMANDER_IGRIUS                = 95285,
    NPC_FELSOUL_CRUSHER                     = 95452,
    NPC_VAREDIS_FELSOUL_IMAGE               = 94310,
    DISPLAY_ILLIDARI_FEL_BAT                = 68247,
    NPC_VAREDIS_FELSOUL                     = 94836,
    NPC_KILJAEDEN_VOICE                     = 25608,

    SAY_VAREDIS_SEEK_MY_HEAD                = 0,

    SAY_VAREDIS_AGGRO                       = 1,
    SAY_VAREDIS_75                          = 2,
    SAY_VAREDIS_50                          = 3,
    SAY_VAREDIS_25                          = 4,
    SAY_VAREDIS_10                          = 5,
    SAY_KILJAEDEN_NOT_YOUR_TIME             = 50,

    DATA_VAREDIS_RECALLED                   = 1,
    DATA_TWINBLADES_WIELDED                 = 2,
    DISPLAY_INVISIBLE_STALKER               = 11686,

    EVENT_ARRIVED                           = 1,
    EVENT_DIVE,
    EVENT_LEAVE_THE_HOLD,
    EVENT_BAT_LEAVES                        = 1,

    EVENT_DEMONS_BITE                       = 1,
    EVENT_SHADOW_STRIKE,
    EVENT_CHAOS_NOVA,
    EVENT_FEL_RUSH,
    EVENT_BLUR
};

// Where the camp blows up under the player: the scenario's first graveyard stands there (safe location 5097)
static constexpr Position CampDropPoint = { 1264.60f, 5234.68f, 93.53f, 3.97f };
// Over the hold on the fel bat, from the start point (safe location 5313) to above the camp. Ours, high enough to clear the cliffs.
static std::array<G3D::Vector3, 3> const FelBatRide =
{ {
    { 900.0f, 5050.0f, 170.0f },
    { 1100.0f, 5150.0f, 170.0f },
    { 1264.60f, 5234.68f, 110.0f }
} };
static constexpr float FelBatRideSpeed = 35.0f;
// After the second ward Varedis shows himself at the third and sends his crusher
static constexpr Position VaredisAtTheThirdWard = { 1100.0f, 4885.0f, 62.07f, 1.2f };
static constexpr Position CrusherAtTheThirdWard = { 1112.0f, 4900.0f, 62.14f, 4.5f };
// The flat ground by the scenario's stage 4 graveyard (safe location 5362)
static constexpr Position VaredisArena = { 1010.0f, 4925.0f, 36.44f, 3.97f };
static constexpr Milliseconds VaredisRecallDuration = 4s;
// Safe location 5696, Krasus' Landing
static constexpr Position HavocExit = { -849.43f, 4259.27f, 746.28f, 0.0f };
// As long as the aura of the Twinblades scene lasts
static constexpr Milliseconds LeaveTheHoldDelay = 15s;
static constexpr Milliseconds FelBatRideDelay = 2s;
static constexpr Milliseconds ArrivalDelay = 3s;
static constexpr Milliseconds DiveDelay = 16s;
static constexpr Milliseconds WardFadeDelay = 2s;
static constexpr Milliseconds VaredisImageDuration = 15s;

// The scenario. The fel bat has no seat in this client's data, so the ride over the hold is the player mounted on its model
// on a server spline, and the dive a teleport down into the camp.
class instance_felsoul_hold : public InstanceMapScript
{
public:
    instance_felsoul_hold() : InstanceMapScript("instance_felsoul_hold", MAP_ACQUISITION_HAVOC) { }

    struct instance_felsoul_hold_InstanceMapScript : public InstanceScript
    {
        instance_felsoul_hold_InstanceMapScript(InstanceMap* map) : InstanceScript(map), _arrived(false), _varedisShown(false), _wardsDestroyed(0)
        {
            SetHeaders(DataHeader);

            // Where players go when they release after dying; without one the core falls back to the default graveyard in Westfall
            SetEntranceLocation(WORLD_SAFE_LOC_FELSOUL_HOLD_GRAVEYARD);
        }

        void OnPlayerEnter(Player* /*player*/) override
        {
            // A player coming back into the running instance does not fly in twice, nor miss a dive he left before
            if (IsOnStep(SCENARIO_STEP_STALKING_YOUR_PREY))
            {
                if (!_arrived)
                    _events.ScheduleEvent(EVENT_ARRIVED, ArrivalDelay);
            }
            else if (IsOnStep(SCENARIO_STEP_THEY_ARE_NOT_PREPARED))
                _events.RescheduleEvent(EVENT_DIVE, DiveDelay);
        }

        void OnUnitDeath(Unit* unit) override
        {
            switch (unit->GetEntry())
            {
                case NPC_FELSOUL_STALKER:
                    SendScenarioEvent(GAME_EVENT_SOUL_35);
                    break;
                case NPC_FELSOUL_BERSERKER:
                    SendScenarioEvent(GAME_EVENT_SOUL_50);
                    break;
                case NPC_FELSOUL_CHAOSWEAVER:
                    SendScenarioEvent(GAME_EVENT_SOUL_85);
                    break;
                case NPC_FEL_COMMANDER_IGRIUS:
                    SendScenarioEvent(GAME_EVENT_SOUL_225);
                    break;
                case NPC_FELSOUL_CRUSHER:
                    SendScenarioEvent(GAME_EVENT_SOUL_225);
                    break;
                default:
                    break;
            }
        }

        void ProcessEvent(WorldObject* obj, uint32 eventId, WorldObject* /*invoker*/) override
        {
            if (eventId != GAME_EVENT_WARD_DESTROYED)
                return;

            if (GameObject* ward = Object::ToGameObject(obj))
            {
                ward->DespawnOrUnsummon(WardFadeDelay);
                switch (ward->GetEntry())
                {
                    case GO_FELSHIELD_WARD_FIRST:
                        SendScenarioEvent(GAME_EVENT_FIRST_WARD);
                        break;
                    case GO_FELSHIELD_WARD_SECOND:
                        SendScenarioEvent(GAME_EVENT_SECOND_WARD);
                        break;
                    case GO_FELSHIELD_WARD_THIRD:
                        SendScenarioEvent(GAME_EVENT_THIRD_WARD);
                        break;
                    default:
                        break;
                }
            }

            if (++_wardsDestroyed == 2)
                VaredisShowsHimself();
        }

        void SetData(uint32 type, uint32 /*data*/) override
        {
            if (type == DATA_TWINBLADES_WIELDED)
            {
                if (GameObject* glaive = instance->GetGameObject(_glaiveGuid))
                    glaive->DespawnOrUnsummon();
                _events.ScheduleEvent(EVENT_LEAVE_THE_HOLD, LeaveTheHoldDelay);
                return;
            }

            if (type != DATA_VAREDIS_RECALLED)
                return;

            if (Player* player = FindScenarioPlayer())
            {
                QuaternionData const rotation = QuaternionData::fromEulerAnglesZYX(VaredisArena.GetOrientation(), 0.0f, 0.0f);
                player->SummonGameObject(GO_TWINBLADES_OF_THE_DECEIVER, VaredisArena, rotation, 0s);
                // The object above is a box that cannot be seen, as for the other artifacts: this is what the player looks for
                if (GameObject* glaive = player->SummonGameObject(GO_GLAIVE, VaredisArena, rotation, 0s))
                    _glaiveGuid = glaive->GetGUID();
            }
            if (TempSummon* voice = instance->SummonCreature(NPC_KILJAEDEN_VOICE, VaredisArena))
            {
                voice->SetDisplayId(DISPLAY_INVISIBLE_STALKER);
                voice->AI()->Talk(SAY_KILJAEDEN_NOT_YOUR_TIME);
                voice->DespawnOrUnsummon(VaredisRecallDuration);
            }
            SendScenarioEvent(GAME_EVENT_VAREDIS_SLAIN);
        }

        void Update(uint32 diff) override
        {
            // The step can end by any of its objectives, so there is no one event that says stage 4 began
            if (!_varedisShown && IsOnStep(SCENARIO_STEP_UNLEASHED_HAVOC))
                VaredisRises();

            _events.Update(diff);
            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_ARRIVED:
                        FlownIn();
                        break;
                    case EVENT_DIVE:
                        Dive();
                        break;
                    case EVENT_LEAVE_THE_HOLD:
                        LeaveTheHold();
                        break;
                    default:
                        break;
                }
            }
        }

    private:
        bool IsOnStep(uint32 stepId) const
        {
            InstanceScenario const* scenario = instance->GetInstanceScenario();
            return scenario && scenario->GetStep() && scenario->GetStep()->ID == stepId;
        }

        // The core only passes a game event on to the scenario when a player is its source, and never for a player in GM mode
        Player* FindScenarioPlayer() const
        {
            for (MapReference const& ref : instance->GetPlayers())
                if (Player* player = ref.GetSource())
                    if (!player->IsGameMaster())
                        return player;
            return nullptr;
        }

        void SendScenarioEvent(uint32 gameEventId) const
        {
            if (Player* player = FindScenarioPlayer())
                GameEvents::Trigger(gameEventId, player, nullptr);
        }

        void FlownIn()
        {
            if (Player* player = FindScenarioPlayer())
            {
                _arrived = true;
                GameEvents::Trigger(GAME_EVENT_FLY_INTO_FELSOUL_HOLD, player, nullptr);
                player->Mount(DISPLAY_ILLIDARI_FEL_BAT);
                player->GetMotionMaster()->LaunchMoveSpline([](Movement::MoveSplineInit& init)
                {
                    Movement::PointsArray path(FelBatRide.begin(), FelBatRide.end());
                    init.MovebyPath(path);
                    init.SetFly();
                    init.SetVelocity(FelBatRideSpeed);
                }, 0, MOTION_PRIORITY_NORMAL, POINT_MOTION_TYPE);
                _events.ScheduleEvent(EVENT_DIVE, DiveDelay);
            }
        }

        void Dive()
        {
            // A player on the way out comes back through OnPlayerEnter; a teleport now would strand him on the loading screen
            Player* player = FindScenarioPlayer();
            if (!player || player->IsBeingTeleported() || !IsOnStep(SCENARIO_STEP_THEY_ARE_NOT_PREPARED))
                return;

            player->Dismount();
            player->NearTeleportTo(CampDropPoint);
            player->CastSpell(player, SPELL_DH_CAMP_EXPLOSION_SCENE, true);
            player->CastSpell(player, SPELL_DH_CAMP_EXPLOSION_EVENT, true);
        }

        void VaredisShowsHimself()
        {
            Player* player = FindScenarioPlayer();
            if (TempSummon* varedis = instance->SummonCreature(NPC_VAREDIS_FELSOUL_IMAGE, VaredisAtTheThirdWard))
            {
                varedis->AI()->Talk(SAY_VAREDIS_SEEK_MY_HEAD, player);
                varedis->DespawnOrUnsummon(VaredisImageDuration);
            }
            instance->SummonCreature(NPC_FELSOUL_CRUSHER, CrusherAtTheThirdWard);
        }

        void LeaveTheHold()
        {
            for (MapReference const& ref : instance->GetPlayers())
                if (Player* player = ref.GetSource())
                    player->TeleportTo(MAP_BROKEN_ISLES, HavocExit.GetPositionX(), HavocExit.GetPositionY(), HavocExit.GetPositionZ(), HavocExit.GetOrientation());
        }

        void VaredisRises()
        {
            Player* player = FindScenarioPlayer();
            if (!player)
                return;

            _varedisShown = true;
            SetEntranceLocation(WORLD_SAFE_LOC_VAREDIS_GRAVEYARD);
            player->AddAura(SPELL_VAREDIS_REVEAL, player);
            instance->SummonCreature(NPC_VAREDIS_FELSOUL, VaredisArena);
        }

        EventMap _events;
        ObjectGuid _glaiveGuid;
        bool _arrived;
        bool _varedisShown;
        uint8 _wardsDestroyed;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_felsoul_hold_InstanceMapScript(map);
    }
};

// 94836 - Varedis Felsoul
// The wiki lists his abilities with no timings and no recording of the fight was found, so the timers are ours. He does not die:
// Kil'jaeden calls him back to Argus.
struct boss_varedis_felsoul : public ScriptedAI
{
    boss_varedis_felsoul(Creature* creature) : ScriptedAI(creature), _nextLine(0), _aggroSaid(false), _recalled(false) { }

    void Reset() override
    {
        _events.Reset();
        _nextLine = 0;
        _aggroSaid = false;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        // Combat can drop and start again without a reset
        if (!_aggroSaid)
        {
            Talk(SAY_VAREDIS_AGGRO);
            _aggroSaid = true;
        }

        _events.Reset();
        _events.ScheduleEvent(EVENT_DEMONS_BITE, 3s);
        _events.ScheduleEvent(EVENT_SHADOW_STRIKE, 8s);
        _events.ScheduleEvent(EVENT_FEL_RUSH, 12s);
        _events.ScheduleEvent(EVENT_CHAOS_NOVA, 20s);
        _events.ScheduleEvent(EVENT_BLUR, 30s);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (_recalled)
        {
            damage = 0;
            return;
        }

        static constexpr std::array<std::pair<uint32, uint32>, 4> Lines =
        { { { 75, SAY_VAREDIS_75 }, { 50, SAY_VAREDIS_50 }, { 25, SAY_VAREDIS_25 }, { 10, SAY_VAREDIS_10 } } };
        while (_nextLine < Lines.size() && me->HealthBelowPctDamaged(Lines[_nextLine].first, damage))
            Talk(Lines[_nextLine++].second);

        if (me->HealthBelowPctDamaged(RecallHealthPct, damage))
        {
            damage = 0;
            Recall();
        }
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
                case EVENT_DEMONS_BITE:
                    DoCastVictim(SPELL_DEMONS_BITE);
                    _events.Repeat(4s);
                    break;
                case EVENT_SHADOW_STRIKE:
                    DoCastVictim(SPELL_SHADOW_STRIKE);
                    _events.Repeat(14s);
                    break;
                case EVENT_FEL_RUSH:
                    DoCastVictim(SPELL_FEL_RUSH);
                    _events.Repeat(18s);
                    break;
                case EVENT_CHAOS_NOVA:
                    DoCastSelf(SPELL_CHAOS_NOVA);
                    _events.Repeat(30s);
                    break;
                case EVENT_BLUR:
                    DoCastSelf(SPELL_BLUR);
                    _events.Repeat(40s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    static constexpr uint32 RecallHealthPct = 5;

    void Recall()
    {
        _recalled = true;
        _events.Reset();
        me->CastStop();
        me->AttackStop();
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToAll(true);
        if (InstanceScript* instance = me->GetInstanceScript())
            instance->SetData(DATA_VAREDIS_RECALLED, 1);
        me->DespawnOrUnsummon(VaredisRecallDuration);
    }

    EventMap _events;
    size_t _nextLine;
    bool _aggroSaid;
    bool _recalled;
};

// 251878 - Twinblades of the Deceiver
struct go_twinblades_of_the_deceiver : public GameObjectAI
{
    using GameObjectAI::GameObjectAI;

    bool OnGossipHello(Player* player) override
    {
        // Gives and equips the blades and plays the scene; the event that ends the stage is its own spell, which nothing casts in the data
        player->CastSpell(player, SPELL_TWINBLADES_OF_THE_DECEIVER, true);
        player->CastSpell(player, SPELL_WIELD_THE_TWINBLADES, true);
        if (InstanceScript* instance = me->GetInstanceScript())
            instance->SetData(DATA_TWINBLADES_WIELDED, 1);
        me->DespawnOrUnsummon();
        return true;
    }
};

// 189523 - The Twinblades of the Deceiver
// SPELL_EFFECT_FORCE_EQUIP_ITEM does nothing in the core
class spell_felsoul_hold_equip_twinblades : public SpellScript
{
    void HandleEquip(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        Item* item = player->GetItemByEntry(ITEM_TWINBLADES_OF_THE_DECEIVER);
        if (item && !item->IsEquipped())
            player->SwapItem(item->GetPos(), (INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_MAINHAND);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_felsoul_hold_equip_twinblades::HandleEquip, EFFECT_0, SPELL_EFFECT_FORCE_EQUIP_ITEM);
    }
};

// 94324 - Illidari Fel Bat, summoned by the one at Krasus' Landing. It has no seat in this client's data, so it only comes to fetch the player.
struct npc_illidari_fel_bat_felsoul_hold : public ScriptedAI
{
    npc_illidari_fel_bat_felsoul_hold(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(WorldObject* /*summoner*/) override
    {
        _events.ScheduleEvent(EVENT_BAT_LEAVES, FelBatRideDelay);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);
        if (_events.ExecuteEvent() == EVENT_BAT_LEAVES)
            DoCastSelf(SPELL_TELEPORT_TO_FELSOUL_HOLD, true);
    }

private:
    EventMap _events;
};

// 186709 - Fly to Felsoul Hold
// Its effect takes the player into a dungeon of the dungeon finder, which the core does not do. The place is the one the spell has in `spell_target_position`.
class spell_felsoul_hold_fly_in : public SpellScript
{
    void HandleTeleport(SpellEffIndex effIndex)
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        if (SpellTargetPosition const* destination = sSpellMgr->GetSpellTargetPosition(GetSpellInfo()->Id, effIndex))
            player->TeleportTo(*destination);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_felsoul_hold_fly_in::HandleTeleport, EFFECT_0, SPELL_EFFECT_TELEPORT_TO_LFG_DUNGEON);
    }
};

void AddSC_instance_felsoul_hold()
{
    new instance_felsoul_hold();
    RegisterCreatureAI(boss_varedis_felsoul);
    RegisterGameObjectAI(go_twinblades_of_the_deceiver);
    RegisterSpellScript(spell_felsoul_hold_equip_twinblades);
    RegisterCreatureAI(npc_illidari_fel_bat_felsoul_hold);
    RegisterSpellScript(spell_felsoul_hold_fly_in);
}
