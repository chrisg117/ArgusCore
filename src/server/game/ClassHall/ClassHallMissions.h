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

#ifndef ClassHallMissions_h__
#define ClassHallMissions_h__

#include "Define.h"
#include "DatabaseEnvFwd.h"
#include "GarrisonPackets.h"
#include "ObjectGuid.h"
#include <map>

class ClassHall;
class Player;
struct GarrMissionEntry;

// the states of a mission, as the client knows them
enum ClassHallMissionState : uint32
{
    CLASS_HALL_MISSION_STATE_OFFERED        = 0,
    CLASS_HALL_MISSION_STATE_IN_PROGRESS    = 1,
    CLASS_HALL_MISSION_STATE_COMPLETED      = 2
};

// The missions of a character's class hall: those on offer at its mission table, those its
// followers are on, and those finished whose reward is not collected yet.
class TC_GAME_API ClassHallMissions
{
public:
    ClassHallMissions(Player* owner, ClassHall* classHall);

    void LoadFromDB(PreparedQueryResult missions);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

private:
    struct Mission
    {
        GarrMissionEntry const* Entry = nullptr;
        WorldPackets::Garrison::GarrisonMission PacketInfo;
    };

    static bool IsClassHallMission(GarrMissionEntry const* mission);

    Player* _owner;
    ClassHall* _classHall;
    std::map<uint32 /*garrMissionId*/, Mission> _missions;
    uint64 _nextMissionDbId;
};

#endif // ClassHallMissions_h__
