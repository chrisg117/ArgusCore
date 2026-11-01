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
#include "EventMap.h"
#include "GameEventSender.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SpellMgr.h"
#include "SpellScript.h"

#define DataHeader "FelsoulHold"

enum FelsoulHoldData
{
    MAP_ACQUISITION_HAVOC                   = 1498,
    WORLD_SAFE_LOC_FELSOUL_HOLD_GRAVEYARD   = 5097,

    GAME_EVENT_FLY_INTO_FELSOUL_HOLD        = 45563,

    SPELL_TELEPORT_TO_FELSOUL_HOLD          = 188922,
    SPELL_DH_CAMP_EXPLOSION_SCENE           = 186760,
    SPELL_DH_CAMP_EXPLOSION_EVENT           = 186771,

    EVENT_ARRIVED                           = 1,
    EVENT_BAT_LEAVES                        = 1
};

// The camp lies far below the cliff where the fel bat drops the player (safe location 5313, height 138)
static constexpr float DiveHeight = 110.0f;
static constexpr Milliseconds FelBatRideDelay = 2s;
static constexpr Milliseconds ArrivalDelay = 3s;

// The scenario's first two steps. The rest of the hold comes with the steps that use it.
class instance_felsoul_hold : public InstanceMapScript
{
public:
    instance_felsoul_hold() : InstanceMapScript("instance_felsoul_hold", MAP_ACQUISITION_HAVOC) { }

    struct instance_felsoul_hold_InstanceMapScript : public InstanceScript
    {
        instance_felsoul_hold_InstanceMapScript(InstanceMap* map) : InstanceScript(map), _arrived(false), _dived(false)
        {
            SetHeaders(DataHeader);

            // Where players go when they release after dying; without one the core falls back to the default graveyard in Westfall
            SetEntranceLocation(WORLD_SAFE_LOC_FELSOUL_HOLD_GRAVEYARD);
        }

        void OnPlayerEnter(Player* /*player*/) override
        {
            if (!_arrived)
                _events.ScheduleEvent(EVENT_ARRIVED, ArrivalDelay);
        }

        void Update(uint32 diff) override
        {
            _events.Update(diff);
            while (uint32 eventId = _events.ExecuteEvent())
                if (eventId == EVENT_ARRIVED)
                    FlownIn();

            if (_arrived && !_dived)
                CheckDive();
        }

    private:
        // The core only passes a game event on to the scenario when a player is its source, and never for a player in GM mode
        Player* FindScenarioPlayer() const
        {
            for (MapReference const& ref : instance->GetPlayers())
                if (Player* player = ref.GetSource())
                    if (!player->IsGameMaster())
                        return player;
            return nullptr;
        }

        void FlownIn()
        {
            if (Player* player = FindScenarioPlayer())
            {
                _arrived = true;
                GameEvents::Trigger(GAME_EVENT_FLY_INTO_FELSOUL_HOLD, player, nullptr);
            }
        }

        void CheckDive()
        {
            Player* player = FindScenarioPlayer();
            if (!player || player->GetPositionZ() >= DiveHeight)
                return;

            _dived = true;
            player->CastSpell(player, SPELL_DH_CAMP_EXPLOSION_SCENE, true);
            player->CastSpell(player, SPELL_DH_CAMP_EXPLOSION_EVENT, true);
        }

        EventMap _events;
        bool _arrived;
        bool _dived;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_felsoul_hold_InstanceMapScript(map);
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
    RegisterCreatureAI(npc_illidari_fel_bat_felsoul_hold);
    RegisterSpellScript(spell_felsoul_hold_fly_in);
}
