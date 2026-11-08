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


#ifndef ClassHallShipments_h__
#define ClassHallShipments_h__

#include "Define.h"
#include "DatabaseEnvFwd.h"
#include "Garrison.h"
#include "ObjectGuid.h"
#include "Optional.h"
#include "SharedDefines.h"
#include <map>
#include <vector>

class ClassHall;
class Player;
struct CharShipmentEntry;

namespace WorldPackets
{
    namespace Garrison
    {
        struct GarrisonShipment;
    }
}

// The orders (shipments) a character has placed in its class hall and not collected yet:
// troops that are being trained.
class TC_GAME_API ClassHallShipments
{
public:
    // how the object of a container looks to the owner: the client takes a display other than the object's own
    // for "ready" and the in-progress flag for "in progress"
    struct ObjectLook
    {
        uint32 DisplayId = 0;   // 0: the object's own
        bool InProgress = false;
    };

    ClassHallShipments(Player* owner, ClassHall* classHall);

    void LoadFromDB(PreparedQueryResult shipments);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void OpenNpc(ObjectGuid npcGuid, uint32 charShipmentId);
    void SendShipmentInfo(ObjectGuid npcGuid) const;

    void RequestShipments(ObjectGuid npcGuid, uint32 count);
    void CreateShipment(CharShipmentEntry const* shipment);
    static SpellCastResult CheckCreateShipmentCast(Player const* caster, uint32 charShipmentId);

    void CompleteShipments(uint32 charShipmentContainerId);

    void SendLandingPageShipmentInfo() const;

    Optional<ObjectLook> GetObjectLook(uint32 charShipmentContainerId) const;
    void Update();

private:
    struct Shipment
    {
        CharShipmentEntry const* Entry = nullptr;
        uint64 Id = 0;
        time_t CreationTime = 0;
        uint32 Duration = 0;

        bool IsReady() const;
        void BuildPacketInfo(WorldPackets::Garrison::GarrisonShipment& shipment) const;
    };

    static bool IsTroopShipment(CharShipmentEntry const* shipment);
    GarrisonError CanOfferShipment(CharShipmentEntry const* shipment) const;
    GarrisonError CanCreateShipment(CharShipmentEntry const* shipment) const;
    uint32 GetCapacity(CharShipmentEntry const* shipment) const;
    uint32 GetAvailableShipments(CharShipmentEntry const* shipment) const;
    std::vector<Shipment const*> GetPendingShipments(uint32 charShipmentContainerId) const;
    void UpdateObjects(uint32 charShipmentContainerId) const;
    void ScheduleObjectsUpdate();

    Player* _owner;
    ClassHall* _classHall;
    std::map<uint64 /*shipmentId*/, Shipment> _shipments;
    uint64 _nextShipmentId;
    time_t _nextObjectsUpdate;  // when the next order turns ready; 0: none is being made

    // the order window the owner has open, and the order it offers
    ObjectGuid _openNpc;
    CharShipmentEntry const* _openShipment;
};

#endif // ClassHallShipments_h__
