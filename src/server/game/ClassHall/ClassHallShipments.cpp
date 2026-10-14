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


#include "ClassHallShipments.h"
#include "ClassHall.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameTime.h"
#include "GarrisonPackets.h"
#include "Log.h"
#include "Player.h"
#include <algorithm>
#include <tuple>

ClassHallShipments::ClassHallShipments(Player* owner, ClassHall* classHall) : _owner(owner), _classHall(classHall), _nextShipmentId(1),
    _openShipment(nullptr)
{
}

bool ClassHallShipments::Shipment::IsReady() const
{
    return CreationTime + time_t(Duration) <= GameTime::GetGameTime();
}

void ClassHallShipments::Shipment::BuildPacketInfo(WorldPackets::Garrison::GarrisonShipment& shipment) const
{
    shipment.ShipmentRecID = Entry->ID;
    shipment.ShipmentID = Id;
    shipment.CreationTime = CreationTime;
    shipment.ShipmentDuration = Duration;
}

// the orders this class handles: those of a class hall container that give a follower
bool ClassHallShipments::IsTroopShipment(CharShipmentEntry const* shipment)
{
    if (!shipment || !shipment->GarrFollowerID)
        return false;

    CharShipmentContainerEntry const* container = sCharShipmentContainerStore.LookupEntry(shipment->ContainerID);
    return container && container->GarrTypeID == GARRISON_TYPE_CLASS_ORDER;
}

GarrisonError ClassHallShipments::CanOfferShipment(CharShipmentEntry const* shipment) const
{
    if (!IsTroopShipment(shipment))
        return GARRISON_ERROR_OPERATION_NOT_SUPPORTED;

    // the limit of the troop kind is not checked here: orders count against it on their own
    GarrisonError error = _classHall->GetFollowers().CanAddFollower(sGarrFollowerStore.LookupEntry(shipment->GarrFollowerID));
    if (error != GARRISON_SUCCESS && error != GARRISON_ERROR_AT_CLASS_SPEC_CAP)
        return error;

    return GARRISON_SUCCESS;
}

GarrisonError ClassHallShipments::CanCreateShipment(CharShipmentEntry const* shipment) const
{
    GarrisonError error = CanOfferShipment(shipment);
    if (error != GARRISON_SUCCESS)
        return error;

    if (!GetAvailableShipments(shipment))
        return GARRISON_ERROR_AT_CLASS_SPEC_CAP;

    return GARRISON_SUCCESS;
}

// as the client counts it: the order's own maximum, but never above the limit of the troop kind
uint32 ClassHallShipments::GetCapacity(CharShipmentEntry const* shipment) const
{
    GarrFollowerEntry const* follower = sGarrFollowerStore.LookupEntry(shipment->GarrFollowerID);
    uint32 capacity = shipment->MaxShipments;
    if (GarrClassSpecEntry const* classSpec = sGarrClassSpecStore.LookupEntry(_classHall->GetFollowers().GetClassSpec(follower)))
        capacity = std::min<uint32>(capacity, classSpec->FollowerClassLimit);

    return capacity;
}

// troops of the kind already owned and orders already placed both use up the capacity
uint32 ClassHallShipments::GetAvailableShipments(CharShipmentEntry const* shipment) const
{
    ClassHallFollowers const& followers = _classHall->GetFollowers();
    GarrFollowerEntry const* follower = sGarrFollowerStore.LookupEntry(shipment->GarrFollowerID);
    uint32 used = followers.CountTroops(followers.GetClassSpec(follower)) + uint32(GetPendingShipments(shipment->ContainerID).size());
    uint32 capacity = GetCapacity(shipment);
    return capacity > used ? capacity - used : 0;
}

// the orders of a container, oldest first
std::vector<ClassHallShipments::Shipment const*> ClassHallShipments::GetPendingShipments(uint32 charShipmentContainerId) const
{
    std::vector<Shipment const*> shipments;
    for (auto const& [shipmentId, shipment] : _shipments)
        if (shipment.Entry->ContainerID == charShipmentContainerId)
            shipments.push_back(&shipment);

    std::sort(shipments.begin(), shipments.end(), [](Shipment const* left, Shipment const* right)
    {
        return std::tie(left->CreationTime, left->Id) < std::tie(right->CreationTime, right->Id);
    });
    return shipments;
}

void ClassHallShipments::LoadFromDB(PreparedQueryResult shipments)
{
    if (!shipments)
        return;

    do
    {
        //                0               1             2         3
        // SELECT shipmentId, charShipmentId, creationTime, duration FROM character_class_hall_shipment WHERE guid = ?
        Field* fields = shipments->Fetch();
        uint64 shipmentId = fields[0].GetUInt64();
        uint32 charShipmentId = fields[1].GetUInt32();
        _nextShipmentId = std::max(_nextShipmentId, shipmentId + 1);

        CharShipmentEntry const* entry = sCharShipmentStore.LookupEntry(charShipmentId);
        if (!IsTroopShipment(entry))
        {
            TC_LOG_ERROR("entities.player", "ClassHallShipments::LoadFromDB: player {} has a class hall shipment of CharShipment {} which is not a class hall troop order, skipped.",
                _owner->GetGUID().ToString(), charShipmentId);
            continue;
        }

        Shipment& shipment = _shipments[shipmentId];
        shipment.Entry = entry;
        shipment.Id = shipmentId;
        shipment.CreationTime = fields[2].GetInt64();
        shipment.Duration = fields[3].GetUInt32();
    } while (shipments->NextRow());
}

void ClassHallShipments::SaveToDB(CharacterDatabaseTransaction trans) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_SHIPMENTS);
    stmt->setUInt64(0, _owner->GetGUID().GetCounter());
    trans->Append(stmt);

    for (auto const& [shipmentId, shipment] : _shipments)
    {
        uint8 index = 0;
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_CLASS_HALL_SHIPMENT);
        stmt->setUInt64(index++, _owner->GetGUID().GetCounter());
        stmt->setUInt64(index++, shipmentId);
        stmt->setUInt32(index++, shipment.Entry->ID);
        stmt->setInt64(index++, shipment.CreationTime);
        stmt->setUInt32(index++, shipment.Duration);
        trans->Append(stmt);
    }
}

void ClassHallShipments::DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_SHIPMENTS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);
}

void ClassHallShipments::OpenNpc(ObjectGuid npcGuid, uint32 charShipmentId)
{
    CharShipmentEntry const* shipment = sCharShipmentStore.LookupEntry(charShipmentId);
    if (CanOfferShipment(shipment) != GARRISON_SUCCESS)
        return;

    _openNpc = npcGuid;
    _openShipment = shipment;

    WorldPackets::Garrison::OpenShipmentNpcFromGossip openShipmentNpc;
    openShipmentNpc.NpcGUID = npcGuid;
    openShipmentNpc.CharShipmentContainerID = shipment->ContainerID;
    _owner->SendDirectMessage(openShipmentNpc.Write());
}

void ClassHallShipments::SendShipmentInfo(ObjectGuid npcGuid) const
{
    if (npcGuid != _openNpc || !_openShipment)
        return;

    // the client works out what is left from the limit of the troop kind itself
    WorldPackets::Garrison::GetShipmentInfoResponse shipmentInfo;
    shipmentInfo.Success = true;
    shipmentInfo.ShipmentRecID = _openShipment->ID;
    shipmentInfo.MaxShipments = GetCapacity(_openShipment);
    for (Shipment const* shipment : GetPendingShipments(_openShipment->ContainerID))
        shipment->BuildPacketInfo(shipmentInfo.Shipments.emplace_back());

    _owner->SendDirectMessage(shipmentInfo.Write());
}

void ClassHallShipments::RequestShipments(ObjectGuid npcGuid, uint32 count)
{
    if (npcGuid != _openNpc || !_openShipment)
        return;

    GarrisonError error = CanCreateShipment(_openShipment);
    if (error != GARRISON_SUCCESS)
    {
        WorldPackets::Garrison::CreateShipmentResponse createShipmentResponse;
        createShipmentResponse.ShipmentRecID = _openShipment->ID;
        createShipmentResponse.Result = error;
        _owner->SendDirectMessage(createShipmentResponse.Write());
        return;
    }

    // the order spell takes the cost, and its effect places the order
    count = std::min(count, GetAvailableShipments(_openShipment));
    for (uint32 i = 0; i < count; ++i)
        if (_owner->CastSpell(_owner, _openShipment->SpellID) != SPELL_CAST_OK)
            break;
}

void ClassHallShipments::CreateShipment(CharShipmentEntry const* shipment)
{
    if (CanCreateShipment(shipment) != GARRISON_SUCCESS)
        return;

    // the orders of a container are made one after another
    time_t creationTime = GameTime::GetGameTime();
    for (Shipment const* pending : GetPendingShipments(shipment->ContainerID))
        creationTime = std::max(creationTime, pending->CreationTime + time_t(pending->Duration));

    uint64 shipmentId = _nextShipmentId++;
    Shipment& created = _shipments[shipmentId];
    created.Entry = shipment;
    created.Id = shipmentId;
    created.CreationTime = creationTime;
    created.Duration = shipment->Duration;

    WorldPackets::Garrison::CreateShipmentResponse createShipmentResponse;
    createShipmentResponse.ShipmentID = shipmentId;
    createShipmentResponse.ShipmentRecID = shipment->ID;
    createShipmentResponse.Result = GARRISON_SUCCESS;
    _owner->SendDirectMessage(createShipmentResponse.Write());
}

// the check of an order spell, before its cost is taken
SpellCastResult ClassHallShipments::CheckCreateShipmentCast(Player const* caster, uint32 charShipmentId)
{
    CharShipmentEntry const* shipment = sCharShipmentStore.LookupEntry(charShipmentId);
    CharShipmentContainerEntry const* container = shipment ? sCharShipmentContainerStore.LookupEntry(shipment->ContainerID) : nullptr;
    if (!container || container->GarrTypeID != GARRISON_TYPE_CLASS_ORDER)
        return SPELL_CAST_OK;

    ClassHall const* classHall = caster->GetClassHall();
    if (!classHall)
        return SPELL_FAILED_NO_BUILDING_FOR_SHIPMENT;

    switch (classHall->GetShipments().CanCreateShipment(shipment))
    {
        case GARRISON_SUCCESS:
            return SPELL_CAST_OK;
        case GARRISON_ERROR_AT_CLASS_SPEC_CAP:
            return SPELL_FAILED_SHIPMENTS_FULL;
        default:
            return SPELL_FAILED_NO_SHIPMENTS_FOR_CONTAINER;
    }
}
