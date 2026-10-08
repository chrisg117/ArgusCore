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

/* ScriptData
Name: scenario_commandscript
%Complete: 100
Comment: Commands for stepping through a scenario while scripting it
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "ChatCommand.h"
#include "CriteriaHandler.h"
#include "DB2Stores.h"
#include "GameEventSender.h"
#include "Player.h"
#include "RBAC.h"
#include "Scenario.h"
#include <algorithm>

using namespace Trinity::ChatCommands;

class scenario_commandscript : public CommandScript
{
public:
    scenario_commandscript() : CommandScript("scenario_commandscript") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable scenarioCommandTable =
        {
            { "event",          HandleScenarioEventCommand,         rbac::RBAC_PERM_COMMAND_DEBUG,  Console::No },
            { "completestep",   HandleScenarioCompleteStepCommand,  rbac::RBAC_PERM_COMMAND_DEBUG,  Console::No }
        };
        static ChatCommandTable commandTable =
        {
            { "scenario",       scenarioCommandTable }
        };
        return commandTable;
    }

    static bool HandleScenarioEventCommand(ChatHandler* handler, uint32 gameEventId, Optional<uint32> count)
    {
        Player* player = handler->GetPlayer();
        if (!player->GetScenario())
        {
            handler->SendSysMessage("You are not in a scenario.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        uint32 times = std::min<uint32>(count.value_or(1), 100);
        for (uint32 i = 0; i < times; ++i)
            GameEvents::Trigger(gameEventId, player, nullptr);

        handler->PSendSysMessage("Triggered game event %u %u time(s).", gameEventId, times);
        return true;
    }

    // Sends every game event the current step's criteria are waiting for
    static bool HandleScenarioCompleteStepCommand(ChatHandler* handler)
    {
        Player* player = handler->GetPlayer();
        Scenario const* scenario = player->GetScenario();
        if (!scenario)
        {
            handler->SendSysMessage("You are not in a scenario.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        ScenarioStepEntry const* step = scenario->GetStep();
        if (!step)
        {
            handler->SendSysMessage("The scenario has no step in progress.");
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (CriteriaTree const* tree = sCriteriaMgr->GetCriteriaTree(step->Criteriatreeid))
        {
            // One pass completes a step that counts its events. A step shown as a progress bar weighs
            // them instead (Amount is then the weight), so it can take several passes to fill.
            for (uint32 pass = 0; pass < 100 && scenario->GetStep() == step; ++pass)
            {
                CriteriaMgr::WalkCriteriaTree(tree, [player](CriteriaTree const* node)
                {
                    if (!node->Criteria || CriteriaType(node->Criteria->Entry->Type) != CriteriaType::AnyoneTriggerGameEventScenario)
                        return;

                    for (int32 i = 0; i < node->Entry->Amount; ++i)
                        GameEvents::Trigger(node->Criteria->Entry->Asset.EventID, player, nullptr);
                });
            }
        }

        if (scenario->GetStep() == step)
        {
            handler->PSendSysMessage("Step %u (%s) is not completed by game events alone.", step->ID, step->Title[handler->GetSessionDbcLocale()]);
            handler->SetSentErrorMessage(true);
            return false;
        }

        handler->PSendSysMessage("Completed step %u (%s).", step->ID, step->Title[handler->GetSessionDbcLocale()]);
        return true;
    }
};

void AddSC_scenario_commandscript()
{
    new scenario_commandscript();
}
