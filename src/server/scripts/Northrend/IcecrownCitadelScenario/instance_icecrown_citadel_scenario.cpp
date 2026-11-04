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

// Map 1480, the death knight's Icecrown Citadel: scenario 901 "The Blades of the Fallen Prince" (Frost artifact,
// quest 38990 The Call of Icecrown). The map shares ICC 631's coordinates; the client's WorldSafeLocs name every
// place the scenario moves the player to.

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "DB2Stores.h"
#include "GameEventSender.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Map.h"
#include "Player.h"
#include "SharedDefines.h"
#include "SpellMgr.h"

#define DataHeader "IcecrownCitadelScenario"

enum IcecrownCitadelScenarioData
{
    MAP_ICECROWN_CITADEL_SCENARIO               = 1480,

    QUEST_THE_CALL_OF_ICECROWN                  = 38990,

    SCENARIO_STEP_THE_CALL_OF_THE_NORTH         = 1809,
    SCENARIO_STEP_THE_GATES_ARE_OPEN            = 1973,

    GAME_EVENT_OPEN_THE_GATE_TO_ICECROWN        = 47614,
    GAME_EVENT_ENTER_ICECROWN_CITADEL           = 45677,

    SPELL_DEATH_GATE_TO_ACHERUS                 = 187753,
    SPELL_ENTER_ICECROWN_CREDIT                 = 185861,

    WORLD_SAFE_LOC_ACHERUS_AREA                 = 5095,
    WORLD_SAFE_LOC_DOOR_TO_DEATHBRINGERS_RISE   = 5076,
    WORLD_SAFE_LOC_DEATHBRINGERS_RISE           = 5075,
    WORLD_SAFE_LOC_THE_SPIRE_AREA               = 5214,

    DATA_ENTERED_THE_CITADEL                    = 1
};

static constexpr Milliseconds SendBackDelay = 2s;

static bool IsOnStep(Map const* map, uint32 stepId)
{
    InstanceMap const* instance = map->ToInstanceMap();
    InstanceScenario const* scenario = instance ? instance->GetInstanceScenario() : nullptr;
    return scenario && scenario->GetStep() && scenario->GetStep()->ID == stepId;
}

// WorldSafeLocs keep their facing in degrees
static void TeleportToSafeLoc(Player* player, uint32 worldSafeLocId)
{
    if (WorldSafeLocsEntry const* loc = sWorldSafeLocsStore.LookupEntry(worldSafeLocId))
        player->NearTeleportTo(loc->Loc.X, loc->Loc.Y, loc->Loc.Z, loc->Facing * float(M_PI) / 180.0f);
}

// Only a death knight on The Call of Icecrown belongs here (a login inside after the quest, or a GM's `go`)
static bool BelongsInTheScenario(Player const* player)
{
    return player->GetClass() == CLASS_DEATH_KNIGHT && player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) == QUEST_STATUS_INCOMPLETE;
}

class instance_icecrown_citadel_scenario : public InstanceMapScript
{
public:
    instance_icecrown_citadel_scenario() : InstanceMapScript("instance_icecrown_citadel_scenario", MAP_ICECROWN_CITADEL_SCENARIO) { }

    struct instance_icecrown_citadel_scenario_InstanceMapScript : public InstanceScript
    {
        instance_icecrown_citadel_scenario_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);

            // Where a player who releases after dying comes back: Acherus beside the citadel, then the Spire
            SetEntranceLocation(WORLD_SAFE_LOC_ACHERUS_AREA);
        }

        void OnPlayerEnter(Player* player) override
        {
            if (player->IsGameMaster() || BelongsInTheScenario(player))
                return;

            // A teleport during the enter would strand the player on the loading screen: send them back once they stand here
            player->m_Events.AddEventAtOffset([player]
            {
                if (player->GetMapId() != MAP_ICECROWN_CITADEL_SCENARIO || player->IsBeingTeleported() || BelongsInTheScenario(player))
                    return;

                if (SpellTargetPosition const* acherus = sSpellMgr->GetSpellTargetPosition(SPELL_DEATH_GATE_TO_ACHERUS, EFFECT_0))
                    player->TeleportTo(*acherus);
            }, SendBackDelay);
        }

        void SetData(uint32 type, uint32 /*data*/) override
        {
            if (type == DATA_ENTERED_THE_CITADEL)
                SetEntranceLocation(WORLD_SAFE_LOC_THE_SPIRE_AREA);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_icecrown_citadel_scenario_InstanceMapScript(map);
    }
};

// 202244 - Scourge Transporter, across the bone bridge from Acherus: opens the gate to Icecrown and carries the
// player to Saurfang's door. Once the gate is open it only carries them
struct go_icc_scenario_scourge_transporter : public GameObjectAI
{
    using GameObjectAI::GameObjectAI;

    bool OnGossipHello(Player* player) override
    {
        // The core passes a game event on to the scenario only for a player who is its source
        if (IsOnStep(me->GetMap(), SCENARIO_STEP_THE_CALL_OF_THE_NORTH))
            GameEvents::Trigger(GAME_EVENT_OPEN_THE_GATE_TO_ICECROWN, player, nullptr);

        TeleportToSafeLoc(player, WORLD_SAFE_LOC_DOOR_TO_DEATHBRINGERS_RISE);
        return true;
    }
};

// Saurfang's door: through it, the player enters the citadel and comes out in the Spire, where the fragments are.
// Before the gate is open (the transporter not used) it does nothing
struct at_icc_scenario_door_to_deathbringers_rise : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || IsOnStep(at->GetMap(), SCENARIO_STEP_THE_CALL_OF_THE_NORTH))
            return;

        if (IsOnStep(at->GetMap(), SCENARIO_STEP_THE_GATES_ARE_OPEN))
        {
            GameEvents::Trigger(GAME_EVENT_ENTER_ICECROWN_CITADEL, player, nullptr);
            player->CastSpell(player, SPELL_ENTER_ICECROWN_CREDIT, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            if (InstanceScript* instance = at->GetInstanceScript())
                instance->SetData(DATA_ENTERED_THE_CITADEL, 1);
        }

        TeleportToSafeLoc(player, WORLD_SAFE_LOC_DEATHBRINGERS_RISE);
    }
};

void AddSC_instance_icecrown_citadel_scenario()
{
    new instance_icecrown_citadel_scenario();
    RegisterGameObjectAI(go_icc_scenario_scourge_transporter);
    RegisterAreaTriggerAI(at_icc_scenario_door_to_deathbringers_rise);
}
