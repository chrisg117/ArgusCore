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

#ifndef ClassHallFollowers_h__
#define ClassHallFollowers_h__

#include "Define.h"
#include "DatabaseEnvFwd.h"
#include "GarrisonPackets.h"
#include "ObjectGuid.h"
#include <list>
#include <map>

class Player;
struct GarrAbilityEntry;
struct GarrFollowerEntry;

// The followers (champions and troops) of a character's class hall.
class TC_GAME_API ClassHallFollowers
{
public:
    explicit ClassHallFollowers(Player* owner);

    void LoadFromDB(PreparedQueryResult followers);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

private:
    struct Follower
    {
        GarrFollowerEntry const* Entry = nullptr;
        WorldPackets::Garrison::GarrisonFollower PacketInfo;
    };

    std::list<GarrAbilityEntry const*> GetAbilities(GarrFollowerEntry const* follower, uint32 quality) const;

    Player* _owner;
    std::map<uint64 /*dbId*/, Follower> _followers;
};

#endif // ClassHallFollowers_h__
