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

#ifndef ClassHall_h__
#define ClassHall_h__

#include "Define.h"
#include "ClassHallFollowers.h"
#include "ClassHallMissions.h"
#include "ClassHallShipments.h"
#include "ClassHallTalents.h"
#include "DatabaseEnvFwd.h"
#include "ObjectGuid.h"

class Player;
struct GarrSiteLevelEntry;

namespace WorldPackets
{
    namespace Garrison
    {
        struct GarrisonInfo;
    }
}

// A Legion class hall (garrison type 3). It shares the garrison packets with the Warlords
// garrison but none of its plots, buildings or maps, so it is kept as its own class.
class TC_GAME_API ClassHall
{
public:
    explicit ClassHall(Player* owner);

    bool LoadFromDB(PreparedQueryResult classHall, PreparedQueryResult talents, PreparedQueryResult followers, PreparedQueryResult equipment,
        PreparedQueryResult temporaryAbilities,
        PreparedQueryResult shipments,
        PreparedQueryResult missions);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    bool Create(uint32 garrSiteId);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

    uint32 GetGarrSiteID() const;

    void Update();

    ClassHallTalents& GetTalents() { return _talents; }
    ClassHallTalents const& GetTalents() const { return _talents; }

    ClassHallFollowers& GetFollowers() { return _followers; }
    ClassHallFollowers const& GetFollowers() const { return _followers; }

    ClassHallShipments& GetShipments() { return _shipments; }
    ClassHallShipments const& GetShipments() const { return _shipments; }

    ClassHallMissions& GetMissions() { return _missions; }
    ClassHallMissions const& GetMissions() const { return _missions; }

private:
    Player* _owner;
    GarrSiteLevelEntry const* _siteLevel;
    ClassHallTalents _talents;
    ClassHallFollowers _followers;
    ClassHallShipments _shipments;
    ClassHallMissions _missions;
};

#endif // ClassHall_h__
