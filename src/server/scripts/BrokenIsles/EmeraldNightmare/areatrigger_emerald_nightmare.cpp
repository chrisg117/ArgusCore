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
#include "Chat.h"
#include "DB2Stores.h"
#include "DB2Structure.h"
#include "InstanceScript.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "emerald_nightmare.h"

namespace
{
// areatrigger_create_properties ids of the four hub portals, and the
// WorldSafeLocs arrival point in the wing each one leads to.
struct HubPortal
{
    uint32 CreatePropertiesId;
    uint32 WorldSafeLocsId;
};

HubPortal const hubPortals[] =
{
    { 152003, 5461 }, // Un'goro Crater
    { 152004, 5578 }, // Thunder Bluff
    { 152005, 5574 }, // Grizzly Hills
    { 152006, 5575 }  // The Emerald Dreamway
};
}

// Hub portals above Nythendra's room. Every wing opens once she is dead; a
// player in GM mode may use them at any time.
struct at_en_hub_portal : public AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player)
            return;

        uint32 const id = at->GetCreateProperties()->Id.Id;
        HubPortal const* portal = nullptr;
        for (HubPortal const& candidate : hubPortals)
            if (candidate.CreatePropertiesId == id)
                portal = &candidate;

        if (!portal)
            return;

        InstanceScript* instance = at->GetInstanceScript();
        if (!player->IsGameMaster() && (!instance || instance->GetBossState(DATA_NYTHENDRA) != DONE))
        {
            ChatHandler(player->GetSession()).SendSysMessage("The portal does not respond.");
            return;
        }

        WorldSafeLocsEntry const* dest = sWorldSafeLocsStore.LookupEntry(portal->WorldSafeLocsId);
        if (!dest)
            return;

        // WorldSafeLocs facing is in degrees
        player->TeleportTo(dest->MapID, dest->Loc.X, dest->Loc.Y, dest->Loc.Z, Position::NormalizeOrientation((dest->Facing * float(M_PI)) / 180.0f));
    }
};

void AddSC_areatrigger_emerald_nightmare()
{
    RegisterAreaTriggerAI(at_en_hub_portal);
}
