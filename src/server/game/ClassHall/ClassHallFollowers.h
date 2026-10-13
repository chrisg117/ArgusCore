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
#include "Garrison.h"
#include "GarrisonPackets.h"
#include "ObjectGuid.h"
#include <list>
#include <map>

class ClassHall;
class Player;
struct GarrAbilityEntry;
struct GarrFollowerEntry;

// The followers (champions and troops) of a character's class hall.
class TC_GAME_API ClassHallFollowers
{
public:
    // followers a character may activate per day
    static constexpr uint32 DAILY_FOLLOWER_ACTIVATIONS = 1;

    ClassHallFollowers(Player* owner, ClassHall const* classHall);

    void LoadFromDB(PreparedQueryResult followers);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

    void AddFollower(uint32 garrFollowerId);
    void SetFollowerInactive(uint64 dbId, bool inactive);

    uint32 GetActivationsRemaining() const { return _activationsRemaining; }
    void SetActivationsRemaining(uint32 activations) { _activationsRemaining = activations; }
    void ResetActivations();

private:
    struct Follower
    {
        GarrFollowerEntry const* Entry = nullptr;
        WorldPackets::Garrison::GarrisonFollower PacketInfo;

        bool IsActive() const { return (PacketInfo.FollowerStatus & FOLLOWER_STATUS_INACTIVE) == 0; }
    };

    GarrisonError CanAddFollower(GarrFollowerEntry const* follower) const;
    GarrisonError CanSetFollowerInactive(Follower const* follower, bool inactive) const;
    void SendActivationsRemaining() const;
    std::list<GarrAbilityEntry const*> GetAbilities(GarrFollowerEntry const* follower, uint32 quality) const;
    uint32 GetClassSpec(GarrFollowerEntry const* follower) const;
    uint32 CountActiveChampions() const;
    uint32 CountTroops(uint32 garrClassSpecId) const;

    Player* _owner;
    ClassHall const* _classHall;
    std::map<uint64 /*dbId*/, Follower> _followers;
    uint32 _activationsRemaining;
};

#endif // ClassHallFollowers_h__
