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
#include "ObjectGuid.h"
#include <map>

class Player;
struct CharShipmentEntry;

// The orders (shipments) a character has placed in its class hall and not collected yet:
// troops that are being trained.
class TC_GAME_API ClassHallShipments
{
public:
    explicit ClassHallShipments(Player* owner);

    void LoadFromDB(PreparedQueryResult shipments);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

private:
    struct Shipment
    {
        CharShipmentEntry const* Entry = nullptr;
        uint64 Id = 0;
        time_t CreationTime = 0;
        uint32 Duration = 0;

        bool IsReady() const;
    };

    static bool IsTroopShipment(CharShipmentEntry const* shipment);

    Player* _owner;
    std::map<uint64 /*shipmentId*/, Shipment> _shipments;
    uint64 _nextShipmentId;
};

#endif // ClassHallShipments_h__
