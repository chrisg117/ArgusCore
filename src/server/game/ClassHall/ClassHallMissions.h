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
#include "ClassHallFollowers.h"
#include "DatabaseEnvFwd.h"
#include "GarrisonPackets.h"
#include "ObjectGuid.h"
#include <map>
#include <vector>

class ClassHall;
class Player;
struct GarrMechanicEntry;
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
    // how many missions with an offer time the table holds at once; the retail number is not
    // known, so this is a chosen value
    static constexpr uint32 ROTATING_MISSIONS_ON_OFFER = 10;

    ClassHallMissions(Player* owner, ClassHall* classHall);

    void LoadFromDB(PreparedQueryResult missions);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

    void Update();
    void OfferMissions(bool notify);
    static bool IsOfferable(GarrMissionEntry const* mission);

    void OpenNpc(ObjectGuid npcGuid);

    void StartMission(ObjectGuid npcGuid, uint32 garrMissionId, std::vector<uint64> const& followerDbIds);
    void CompleteMission(ObjectGuid npcGuid, uint32 garrMissionId);
    void BonusRoll(ObjectGuid npcGuid, uint32 garrMissionId);

private:
    struct Mission
    {
        GarrMissionEntry const* Entry = nullptr;
        WorldPackets::Garrison::GarrisonMission PacketInfo;
    };

    // what a team makes of a mission, as the client shows it before the mission is started
    struct TeamEffects
    {
        uint32 SuccessChance = 0;
        uint32 Cost = 0;
        uint32 MissionDuration = 0;
        bool Lethal = false;    // the troops on the mission are killed
    };

    static bool IsClassHallMission(GarrMissionEntry const* mission);
    static void InitMission(Mission& mission, GarrMissionEntry const* entry, uint64 dbId, time_t offerTime);
    bool IsConditionMet(GarrMissionEntry const* mission) const;
    void AddMission(GarrMissionEntry const* entry, time_t offerTime, bool notify);
    void SendAddMission(Mission const& mission) const;
    static bool IsOfferExpired(Mission const& mission, time_t now);
    bool IsOpenNpc(ObjectGuid npcGuid) const;
    GarrisonError CanStartMission(Mission const* mission, std::vector<ClassHallFollowers::Follower const*> const& team) const;
    void GiveRewards(uint32 garrMissionId);

    static std::vector<GarrMechanicEntry const*> GetMechanics(GarrMissionEntry const* mission, uint64 dbId);
    static double GetBias(GarrMissionEntry const* mission, ClassHallFollowers::Follower const& follower);
    TeamEffects GetTeamEffects(Mission const& mission, std::vector<ClassHallFollowers::Follower const*> const& team) const;
    static uint32 GetExperienceBonus(Mission const& mission);

    Player* _owner;
    ClassHall* _classHall;
    std::map<uint32 /*garrMissionId*/, Mission> _missions;
    uint64 _nextMissionDbId;
    time_t _nextOfferTime;

    // the mission table the owner opened last; the client says nothing when it closes
    ObjectGuid _openNpc;

    // missions started since login, which the client is told with each start
    uint16 _sessionMissionCount;
};

#endif // ClassHallMissions_h__
