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
#include "ConditionMgr.h"
#include "Containers.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameTime.h"
#include "Garrison.h"
#include "GarrisonMgr.h"
#include "Log.h"
#include "Player.h"
#include <algorithm>

namespace
{
// how often the offers are looked at
constexpr time_t MISSION_OFFER_INTERVAL = MINUTE;

// recruitment, zone support and invasion missions are other mechanisms, never on the table
constexpr uint8 NonTableMissionTypes[] = { 11, 51, 52, 57 };
}

ClassHallMissions::ClassHallMissions(Player* owner, ClassHall* classHall) : _owner(owner), _classHall(classHall), _nextMissionDbId(1),
    _nextOfferTime(0)
{
}

// the missions of the class hall's mission table: those for its champions and troops
bool ClassHallMissions::IsClassHallMission(GarrMissionEntry const* mission)
{
    return mission && mission->GarrTypeID == GARRISON_TYPE_CLASS_ORDER && mission->GarrFollowerTypeID == FOLLOWER_TYPE_CLASS_ORDER;
}

bool ClassHallMissions::IsOfferable(GarrMissionEntry const* mission)
{
    if (!IsClassHallMission(mission) || advstd::ranges::contains(NonTableMissionTypes, mission->GarrMissionTypeID))
        return false;

    // a mission without an offer time is on the table only through its condition: the client
    // data has such missions without one only as placeholders and special missions
    return mission->OfferDuration || mission->PlayerConditionID;
}

void ClassHallMissions::InitMission(Mission& mission, GarrMissionEntry const* entry, uint64 dbId, time_t offerTime)
{
    mission.Entry = entry;
    mission.PacketInfo.DbID = dbId;
    mission.PacketInfo.MissionRecID = entry->ID;
    mission.PacketInfo.OfferTime = offerTime;
    mission.PacketInfo.OfferDuration = Seconds(entry->OfferDuration);
    mission.PacketInfo.TravelDuration = Seconds(entry->TravelDuration);
    mission.PacketInfo.MissionDuration = Seconds(entry->MissionDuration);
    mission.PacketInfo.Flags = entry->Flags;
}

// a condition the client data does not have counts as not met: content has to define it first
bool ClassHallMissions::IsConditionMet(GarrMissionEntry const* mission) const
{
    if (!mission->PlayerConditionID)
        return true;

    return sPlayerConditionStore.LookupEntry(mission->PlayerConditionID) && ConditionMgr::IsPlayerMeetingCondition(_owner, mission->PlayerConditionID);
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
            InitMission(mission, entry, dbId, fields[2].GetInt64());
            // a mission that is only offered keeps the start time the client is sent for one never started
            if (time_t startTime = fields[3].GetInt64())
                mission.PacketInfo.StartTime = startTime;
            mission.PacketInfo.SuccessChance = uint32(fields[4].GetInt32());
            mission.PacketInfo.MissionState = fields[5].GetUInt8();
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
        std::vector<WorldPackets::Garrison::GarrisonMissionReward>& rewards = info.MissionRewards.emplace_back();
        if (std::vector<WorldPackets::Garrison::GarrisonMissionReward> const* missionRewards = sGarrisonMgr.GetClassHallMissionRewards(missionId))
            rewards = *missionRewards;

        info.MissionOvermaxRewards.emplace_back();
        info.CanStartMission.push_back(true);
    }
}

void ClassHallMissions::Update()
{
    if (_nextOfferTime > GameTime::GetGameTime())
        return;

    OfferMissions(true);
}

void ClassHallMissions::OfferMissions(bool notify)
{
    time_t now = GameTime::GetGameTime();
    _nextOfferTime = now + MISSION_OFFER_INTERVAL;

    // an offer leaves the table at its end time; the client drops it by itself, so nothing is sent
    for (auto itr = _missions.begin(); itr != _missions.end();)
    {
        Mission const& mission = itr->second;
        if (mission.PacketInfo.MissionState == CLASS_HALL_MISSION_STATE_OFFERED && mission.Entry->OfferDuration
            && time_t(mission.PacketInfo.OfferTime) + time_t(mission.Entry->OfferDuration) <= now)
            itr = _missions.erase(itr);
        else
            ++itr;
    }

    std::vector<GarrMissionEntry const*> rotating;
    for (GarrMissionEntry const* mission : sGarrMissionStore)
    {
        if (!IsOfferable(mission) || _missions.contains(mission->ID) || !IsConditionMet(mission))
            continue;

        // a mission without an offer time is on the table whenever its condition is met
        if (!mission->OfferDuration)
            AddMission(mission, now, notify);
        else
            rotating.push_back(mission);
    }

    uint32 rotatingOnOffer = uint32(std::count_if(_missions.begin(), _missions.end(), [](auto const& mission)
    {
        return mission.second.PacketInfo.MissionState == CLASS_HALL_MISSION_STATE_OFFERED && mission.second.Entry->OfferDuration;
    }));

    if (rotatingOnOffer >= ROTATING_MISSIONS_ON_OFFER)
        return;

    Trinity::Containers::RandomResize(rotating, ROTATING_MISSIONS_ON_OFFER - rotatingOnOffer);
    for (GarrMissionEntry const* mission : rotating)
        AddMission(mission, now, notify);
}

void ClassHallMissions::AddMission(GarrMissionEntry const* entry, time_t offerTime, bool notify)
{
    Mission& mission = _missions[entry->ID];
    InitMission(mission, entry, _nextMissionDbId++, offerTime);

    if (!notify)
        return;

    WorldPackets::Garrison::GarrisonAddMissionResult addMissionResult;
    addMissionResult.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
    addMissionResult.State = CLASS_HALL_MISSION_STATE_OFFERED;
    addMissionResult.Mission = mission.PacketInfo;
    if (std::vector<WorldPackets::Garrison::GarrisonMissionReward> const* rewards = sGarrisonMgr.GetClassHallMissionRewards(entry->ID))
        addMissionResult.Rewards = *rewards;

    addMissionResult.CanStart = true;
    _owner->SendDirectMessage(addMissionResult.Write());
}
