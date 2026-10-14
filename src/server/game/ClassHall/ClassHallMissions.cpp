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

#include "ClassHallMissions.h"
#include "ClassHall.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "Garrison.h"
#include "Log.h"
#include "Player.h"
#include <algorithm>

ClassHallMissions::ClassHallMissions(Player* owner, ClassHall* classHall) : _owner(owner), _classHall(classHall), _nextMissionDbId(1)
{
}

// the missions of the class hall's mission table: those for its champions and troops
bool ClassHallMissions::IsClassHallMission(GarrMissionEntry const* mission)
{
    return mission && mission->GarrTypeID == GARRISON_TYPE_CLASS_ORDER && mission->GarrFollowerTypeID == FOLLOWER_TYPE_CLASS_ORDER;
}

void ClassHallMissions::LoadFromDB(PreparedQueryResult missions)
{
    if (missions)
    {
        do
        {
            //               0     1          2          3              4      5
            // SELECT missionId, dbId, offerTime, startTime, successChance, state FROM character_class_hall_mission WHERE guid = ?
            Field* fields = missions->Fetch();
            uint32 missionId = fields[0].GetUInt32();
            uint64 dbId = fields[1].GetUInt64();
            _nextMissionDbId = std::max(_nextMissionDbId, dbId + 1);

            GarrMissionEntry const* entry = sGarrMissionStore.LookupEntry(missionId);
            if (!IsClassHallMission(entry))
            {
                TC_LOG_ERROR("entities.player", "ClassHallMissions::LoadFromDB: player {} has GarrMission {} which is not a class hall mission, skipped.",
                    _owner->GetGUID().ToString(), missionId);
                continue;
            }

            Mission& mission = _missions[missionId];
            mission.Entry = entry;
            mission.PacketInfo.DbID = dbId;
            mission.PacketInfo.MissionRecID = missionId;
            mission.PacketInfo.OfferTime = fields[2].GetInt64();
            mission.PacketInfo.OfferDuration = Seconds(entry->OfferDuration);
            // a mission that is only offered keeps the start time the client is sent for one never started
            if (time_t startTime = fields[3].GetInt64())
                mission.PacketInfo.StartTime = startTime;
            mission.PacketInfo.TravelDuration = Seconds(entry->TravelDuration);
            mission.PacketInfo.MissionDuration = Seconds(entry->MissionDuration);
            mission.PacketInfo.MissionState = fields[5].GetUInt8();
            mission.PacketInfo.SuccessChance = uint32(fields[4].GetInt32());
            mission.PacketInfo.Flags = entry->Flags;
        } while (missions->NextRow());
    }

    // a follower can only be on a mission the character holds in progress
    ClassHallFollowers& followers = _classHall->GetFollowers();
    for (auto const& [dbId, follower] : followers.GetAllFollowers())
    {
        uint32 missionId = follower.PacketInfo.CurrentMissionID;
        if (!missionId)
            continue;

        auto itr = _missions.find(missionId);
        if (itr != _missions.end() && itr->second.PacketInfo.MissionState == CLASS_HALL_MISSION_STATE_IN_PROGRESS)
            continue;

        TC_LOG_ERROR("entities.player", "ClassHallMissions::LoadFromDB: player {} has a class hall follower on a mission it does not hold in progress (follower {}, GarrMission {}), set free.",
            _owner->GetGUID().ToString(), dbId, missionId);
        followers.SetFollowerMission(dbId, 0);
    }
}

void ClassHallMissions::SaveToDB(CharacterDatabaseTransaction trans) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_MISSIONS);
    stmt->setUInt64(0, _owner->GetGUID().GetCounter());
    trans->Append(stmt);

    for (auto const& [missionId, mission] : _missions)
    {
        uint8 index = 0;
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_CLASS_HALL_MISSION);
        stmt->setUInt64(index++, _owner->GetGUID().GetCounter());
        stmt->setUInt32(index++, missionId);
        stmt->setUInt64(index++, mission.PacketInfo.DbID);
        stmt->setInt64(index++, time_t(mission.PacketInfo.OfferTime));
        stmt->setInt64(index++, mission.PacketInfo.MissionState == CLASS_HALL_MISSION_STATE_OFFERED ? time_t(0) : time_t(mission.PacketInfo.StartTime));
        stmt->setInt32(index++, int32(mission.PacketInfo.SuccessChance));
        stmt->setUInt8(index++, uint8(mission.PacketInfo.MissionState));
        trans->Append(stmt);
    }
}

void ClassHallMissions::DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_MISSIONS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);
}

void ClassHallMissions::BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const
{
    for (auto const& [missionId, mission] : _missions)
    {
        info.Missions.push_back(&mission.PacketInfo);
        info.MissionRewards.emplace_back();
        info.MissionOvermaxRewards.emplace_back();
        info.CanStartMission.push_back(true);
    }
}
