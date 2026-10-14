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

#include "WorldSession.h"
#include "ClassHall.h"
#include "Garrison.h"
#include "GarrisonPackets.h"
#include "Player.h"

namespace
{
// the follower requests do not say which garrison the follower belongs to
bool IsGarrisonFollower(Player const* player, uint64 followerDbId)
{
    Garrison const* garrison = player->GetGarrison();
    return garrison && garrison->GetFollower(followerDbId);
}
}

void WorldSession::HandleGetGarrisonInfo(WorldPackets::Garrison::GetGarrisonInfo& /*getGarrisonInfo*/)
{
    // the client asks once and expects every garrison the character has in one reply
    WorldPackets::Garrison::GetGarrisonInfoResult garrisonInfo;
    garrisonInfo.FactionIndex = _player->GetTeam() == HORDE ? GARRISON_FACTION_INDEX_HORDE : GARRISON_FACTION_INDEX_ALLIANCE;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->BuildInfo(garrisonInfo.Garrisons.emplace_back());

    if (ClassHall const* classHall = _player->GetClassHall())
        classHall->BuildInfo(garrisonInfo.Garrisons.emplace_back());

    if (garrisonInfo.Garrisons.empty())
        return;

    SendPacket(garrisonInfo.Write());
}

void WorldSession::HandleGarrisonPurchaseBuilding(WorldPackets::Garrison::GarrisonPurchaseBuilding& garrisonPurchaseBuilding)
{
    if (!_player->GetNPCIfCanInteractWith(garrisonPurchaseBuilding.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG_2_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->PlaceBuilding(garrisonPurchaseBuilding.PlotInstanceID, garrisonPurchaseBuilding.BuildingID);
}

void WorldSession::HandleGarrisonCancelConstruction(WorldPackets::Garrison::GarrisonCancelConstruction& garrisonCancelConstruction)
{
    if (!_player->GetNPCIfCanInteractWith(garrisonCancelConstruction.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG_2_GARRISON_ARCHITECT))
        return;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->CancelBuildingConstruction(garrisonCancelConstruction.PlotInstanceID);
}

void WorldSession::HandleGarrisonRequestBlueprintAndSpecializationData(WorldPackets::Garrison::GarrisonRequestBlueprintAndSpecializationData& /*garrisonRequestBlueprintAndSpecializationData*/)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendBlueprintAndSpecializationData();
}

void WorldSession::HandleGarrisonGetMapData(WorldPackets::Garrison::GarrisonGetMapData& /*garrisonGetMapData*/)
{
    if (Garrison* garrison = _player->GetGarrison())
        garrison->SendMapData(_player);
}

void WorldSession::HandleGarrisonRequestScoutingMap(WorldPackets::Garrison::GarrisonRequestScoutingMap& garrisonRequestScoutingMap)
{
    WorldPackets::Garrison::GarrisonScoutingMapResult result;
    result.ID = garrisonRequestScoutingMap.ID;
    result.Active = true;
    SendPacket(result.Write());
}

void WorldSession::HandleGarrisonResearchTalent(WorldPackets::Garrison::GarrisonResearchTalent& garrisonResearchTalent)
{
    if (ClassHall* classHall = _player->GetClassHall())
    {
        classHall->GetTalents().Research(garrisonResearchTalent.GarrTalentID);
        return;
    }

    WorldPackets::Garrison::GarrisonResearchTalentResult researchTalentResult;
    researchTalentResult.Result = GARRISON_ERROR_NO_GARRISON;
    researchTalentResult.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
    researchTalentResult.Talent.GarrTalentID = garrisonResearchTalent.GarrTalentID;
    SendPacket(researchTalentResult.Write());
}

void WorldSession::HandleGarrisonSetFollowerInactive(WorldPackets::Garrison::GarrisonSetFollowerInactive& garrisonSetFollowerInactive)
{
    if (IsGarrisonFollower(_player, garrisonSetFollowerInactive.FollowerDBID))
        return;

    if (ClassHall* classHall = _player->GetClassHall())
        classHall->GetFollowers().SetFollowerInactive(garrisonSetFollowerInactive.FollowerDBID, garrisonSetFollowerInactive.Inactive);
}

void WorldSession::HandleGarrisonRemoveFollower(WorldPackets::Garrison::GarrisonRemoveFollower& garrisonRemoveFollower)
{
    if (!_player->GetNPCIfCanInteractWith(garrisonRemoveFollower.NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG_2_GARRISON_MISSION_NPC))
        return;

    if (IsGarrisonFollower(_player, garrisonRemoveFollower.FollowerDBID))
        return;

    if (ClassHall* classHall = _player->GetClassHall())
        classHall->GetFollowers().RemoveFollower(garrisonRemoveFollower.FollowerDBID);
}

void WorldSession::HandleGarrisonRequestClassSpecCategoryInfo(WorldPackets::Garrison::GarrisonRequestClassSpecCategoryInfo& garrisonRequestClassSpecCategoryInfo)
{
    if (ClassHall const* classHall = _player->GetClassHall())
        classHall->GetFollowers().SendClassSpecCategoryInfo(garrisonRequestClassSpecCategoryInfo.GarrFollowerTypeID);
}

void WorldSession::HandleGarrisonRequestShipmentInfo(WorldPackets::Garrison::GarrisonRequestShipmentInfo& garrisonRequestShipmentInfo)
{
    // the recruiter offers its orders through gossip: the client opens no gossip for a shipment crafter
    if (!_player->GetNPCIfCanInteractWith(garrisonRequestShipmentInfo.NpcGUID, UNIT_NPC_FLAG_GOSSIP, UNIT_NPC_FLAG_2_NONE))
        return;

    if (ClassHall const* classHall = _player->GetClassHall())
        classHall->GetShipments().SendShipmentInfo(garrisonRequestShipmentInfo.NpcGUID);
}

void WorldSession::HandleCreateShipment(WorldPackets::Garrison::CreateShipment& createShipment)
{
    if (!_player->GetNPCIfCanInteractWith(createShipment.NpcGUID, UNIT_NPC_FLAG_GOSSIP, UNIT_NPC_FLAG_2_NONE))
        return;

    if (ClassHall* classHall = _player->GetClassHall())
        classHall->GetShipments().RequestShipments(createShipment.NpcGUID, createShipment.Count);
}
