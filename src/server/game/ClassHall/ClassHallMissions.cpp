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
#include "AdventureMapPackets.h"
#include "ClassHall.h"
#include "ConditionMgr.h"
#include "Containers.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameTime.h"
#include "Garrison.h"
#include "GarrisonMgr.h"
#include "Item.h"
#include "ItemEnchantmentMgr.h"
#include "Log.h"
#include "Player.h"
#include "Random.h"
#include <algorithm>
#include <cmath>
#include <random>
#include <set>

namespace
{
// how often the offers are looked at
constexpr time_t MISSION_OFFER_INTERVAL = MINUTE;

// recruitment, zone support and invasion missions are other mechanisms, never on the table
constexpr uint8 NonTableMissionTypes[] = { 11, 51, 52, 57 };

// what an ability effect (GarrAbilityEffect.AbilityAction) does, for those the success chance reads
enum AbilityAction : uint8
{
    ABILITY_ACTION_COUNTER_MECHANIC             = 0,    // counters a threat type, with a weight
    ABILITY_ACTION_SUCCESS_CHANCE               = 2,    // a flat bonus
    ABILITY_ACTION_SUCCESS_CHANCE_WITH_SPEC     = 22,   // a flat bonus when a follower of a class spec is on the team
    ABILITY_ACTION_SUCCESS_CHANCE_WITH_MECHANIC = 23,   // a flat bonus when the mission has a threat of a type
    ABILITY_ACTION_NO_BONUS_LOOT                = 29    // a threat's: the chance stops at 100
};

bool HasEffect(uint32 garrAbilityId, uint8 abilityAction)
{
    for (GarrAbilityEffectEntry const* effect : sGarrAbilityEffectStore)
        if (effect->GarrAbilityID == garrAbilityId && effect->AbilityAction == abilityAction)
            return true;

    return false;
}
}

ClassHallMissions::ClassHallMissions(Player* owner, ClassHall* classHall) : _owner(owner), _classHall(classHall), _nextMissionDbId(1),
    _nextOfferTime(0), _sessionMissionCount(0)
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
        if (IsOfferExpired(itr->second, now))
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

// The threats a mission has. Where the client data names an encounter set or a threat set, the
// client draws one member with std::minstd_rand seeded with the mission's db id, one value per set,
// in order: each encounter row (by OrderIndex) and then the threat rows of the encounter drawn.
// The server makes the same draw, so both work out the chance from the same threats.
std::vector<GarrMechanicEntry const*> ClassHallMissions::GetMechanics(GarrMissionEntry const* mission, uint64 dbId)
{
    std::vector<GarrMissionXEncounterEntry const*> encounters;
    for (GarrMissionXEncounterEntry const* encounter : sGarrMissionXEncounterStore)
        if (encounter->GarrMissionID == mission->ID)
            encounters.push_back(encounter);

    std::stable_sort(encounters.begin(), encounters.end(), [](GarrMissionXEncounterEntry const* left, GarrMissionXEncounterEntry const* right)
    {
        return left->OrderIndex < right->OrderIndex;
    });

    std::minstd_rand random{ std::minstd_rand::result_type(dbId) };
    std::vector<GarrMechanicEntry const*> mechanics;
    for (GarrMissionXEncounterEntry const* missionEncounter : encounters)
    {
        uint32 encounterId = missionEncounter->GarrEncounterID;
        if (missionEncounter->GarrEncounterSetID)
        {
            std::vector<uint32> members;
            for (GarrEncounterSetXEncounterEntry const* member : sGarrEncounterSetXEncounterStore)
                if (member->GarrEncounterSetID == missionEncounter->GarrEncounterSetID)
                    members.push_back(member->GarrEncounterID);

            if (members.empty())
                continue;

            encounterId = members[random() % members.size()];
        }

        for (GarrEncounterXMechanicEntry const* encounterMechanic : sGarrEncounterXMechanicStore)
        {
            if (encounterMechanic->GarrEncounterID != encounterId)
                continue;

            uint32 mechanicId = encounterMechanic->GarrMechanicID;
            if (encounterMechanic->GarrMechanicSetID)
            {
                std::vector<uint32> members;
                for (GarrMechanicSetXMechanicEntry const* member : sGarrMechanicSetXMechanicStore)
                    if (member->GarrMechanicSetID == encounterMechanic->GarrMechanicSetID)
                        members.push_back(member->GarrMechanicID);

                if (members.empty())
                    continue;

                mechanicId = members[random() % members.size()];
            }

            if (GarrMechanicEntry const* mechanic = sGarrMechanicStore.LookupEntry(mechanicId))
                mechanics.push_back(mechanic);
        }
    }

    return mechanics;
}

// how a champion measures up to a mission, from -1 to 1: its level, and its item level unless it is
// above the mission's level; troops are always 0
double ClassHallMissions::GetBias(GarrMissionEntry const* mission, ClassHallFollowers::Follower const& follower)
{
    GarrFollowerTypeEntry const* followerType = sGarrFollowerTypeStore.LookupEntry(mission->GarrFollowerTypeID);
    if (!followerType || !followerType->LevelRangeBias || !followerType->ItemLevelRangeBias || (follower.PacketInfo.FollowerStatus & FOLLOWER_STATUS_TROOP))
        return 0.0;

    int32 level = int32(follower.PacketInfo.FollowerLevel);
    double bias = double(level - mission->TargetLevel) / followerType->LevelRangeBias;
    if (level <= mission->TargetLevel)
    {
        int32 itemLevel = int32(follower.PacketInfo.ItemLevelWeapon + follower.PacketInfo.ItemLevelArmor) / 2;
        bias += double(itemLevel - int32(mission->TargetItemLevel)) / followerType->ItemLevelRangeBias;
    }

    return std::clamp(bias, -1.0, 1.0);
}

// The chance the client shows for a team, worked out the way the client does (measured from the
// client's own numbers): champions bring power by their bias, abilities counter threats up to each
// threat type's factor, and both are measured against the threats' factors; flat bonuses are added
// on top. An uncountered threat that takes the bonus loot keeps the chance at 100.
uint32 ClassHallMissions::GetSuccessChance(Mission const& mission, std::vector<ClassHallFollowers::Follower const*> const& team) const
{
    std::vector<GarrMechanicEntry const*> mechanics = GetMechanics(mission.Entry, mission.PacketInfo.DbID);
    std::map<uint8 /*garrMechanicTypeId*/, double> factors;
    double totalFactor = 0.0;
    for (GarrMechanicEntry const* mechanic : mechanics)
    {
        factors[mechanic->GarrMechanicTypeID] += mechanic->Factor;
        totalFactor += mechanic->Factor;
    }

    ClassHallFollowers const& followers = _classHall->GetFollowers();
    std::set<uint32> classSpecs;
    for (ClassHallFollowers::Follower const* follower : team)
        classSpecs.insert(followers.GetClassSpec(follower->Entry));

    double power = 0.0;
    double flat = 0.0;
    std::map<uint8 /*garrMechanicTypeId*/, double> counterWeights;
    for (ClassHallFollowers::Follower const* follower : team)
    {
        bool troop = (follower->PacketInfo.FollowerStatus & FOLLOWER_STATUS_TROOP) != 0;
        double bias = GetBias(mission.Entry, *follower);

        // a champion far below the mission adds nothing, not even its traits
        if (!troop)
        {
            if (bias <= -1.0)
                continue;

            power += bias < 0.0 ? 100.0 * (1.0 + bias) : 100.0 + 50.0 * bias;
        }

        for (GarrAbilityEntry const* ability : follower->PacketInfo.AbilityID)
        {
            for (GarrAbilityEffectEntry const* effect : sGarrAbilityEffectStore)
            {
                if (effect->GarrAbilityID != ability->ID)
                    continue;

                switch (effect->AbilityAction)
                {
                    case ABILITY_ACTION_COUNTER_MECHANIC:
                        if (!effect->AbilityTargetType)
                            counterWeights[effect->GarrMechanicTypeID] += bias < 0.0 ? effect->CombatWeightBase * (1.0 + bias)
                                : effect->CombatWeightBase + (effect->CombatWeightMax - effect->CombatWeightBase) * bias;
                        break;
                    case ABILITY_ACTION_SUCCESS_CHANCE:
                        flat += effect->ActionValueFlat;
                        break;
                    case ABILITY_ACTION_SUCCESS_CHANCE_WITH_SPEC:
                        if (classSpecs.contains(uint32(effect->ActionRecordID)))
                            flat += effect->ActionValueFlat;
                        break;
                    case ABILITY_ACTION_SUCCESS_CHANCE_WITH_MECHANIC:
                        if (factors.contains(effect->GarrMechanicTypeID))
                            flat += effect->ActionValueFlat;
                        break;
                    default:
                        break;
                }
            }
        }
    }

    double countered = 0.0;
    for (auto const& [mechanicTypeId, factor] : factors)
    {
        auto itr = counterWeights.find(mechanicTypeId);
        if (itr != counterWeights.end())
            countered += std::min(itr->second, factor);
    }

    double chance = 100.0 * (power + countered) / (totalFactor + 100.0) + flat;
    for (GarrMechanicEntry const* mechanic : mechanics)
        if (!counterWeights.contains(mechanic->GarrMechanicTypeID) && HasEffect(mechanic->GarrAbilityID, ABILITY_ACTION_NO_BONUS_LOOT))
            chance = std::min(chance, 100.0);

    return uint32(std::max(0.0, std::nearbyint(chance)));
}

void ClassHallMissions::OpenNpc(ObjectGuid npcGuid)
{
    _openNpc = npcGuid;

    // the class hall's table is the adventure map frame, which the garrison's open message does not show
    WorldPackets::AdventureMap::ShowAdventureMap showAdventureMap;
    showAdventureMap.NpcGUID = npcGuid;
    _owner->SendDirectMessage(showAdventureMap.Write());
}

// an offer that has run out stays held until the next look at the offers, but can no longer be taken
bool ClassHallMissions::IsOfferExpired(Mission const& mission, time_t now)
{
    return mission.PacketInfo.MissionState == CLASS_HALL_MISSION_STATE_OFFERED && mission.Entry->OfferDuration
        && time_t(mission.PacketInfo.OfferTime) + time_t(mission.Entry->OfferDuration) <= now;
}

// the requests made at the mission table name its NPC, which has to be the one opened and in reach
bool ClassHallMissions::IsOpenNpc(ObjectGuid npcGuid) const
{
    return !npcGuid.IsEmpty() && npcGuid == _openNpc
        && _owner->GetNPCIfCanInteractWith(npcGuid, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG_2_GARRISON_MISSION_NPC);
}

GarrisonError ClassHallMissions::CanStartMission(Mission const* mission, std::vector<ClassHallFollowers::Follower const*> const& team) const
{
    if (!mission || mission->PacketInfo.MissionState != CLASS_HALL_MISSION_STATE_OFFERED || IsOfferExpired(*mission, GameTime::GetGameTime()))
        return GARRISON_ERROR_MISSION_NOT_OFFERED;

    // the client starts a mission only with at least one champion on it
    if (team.empty() || team.size() > mission->Entry->MaxFollowers || std::all_of(team.begin(), team.end(), [](ClassHallFollowers::Follower const* follower)
    {
        return follower && (follower->PacketInfo.FollowerStatus & FOLLOWER_STATUS_TROOP);
    }))
        return GARRISON_ERROR_MISSION_SIZE_INVALID;

    for (ClassHallFollowers::Follower const* follower : team)
    {
        if (!follower || follower->Entry->GarrFollowerTypeID != mission->Entry->GarrFollowerTypeID || !follower->IsActive()
            || std::count(team.begin(), team.end(), follower) > 1)
            return GARRISON_ERROR_INVALID_FOLLOWER;

        if (follower->PacketInfo.CurrentMissionID)
            return GARRISON_ERROR_FOLLOWER_ALREADY_ON_MISSION;
    }

    if (mission->Entry->MissionCost && mission->Entry->MissionCostCurrencyTypesID
        && !_owner->HasCurrency(mission->Entry->MissionCostCurrencyTypesID, mission->Entry->MissionCost))
        return GARRISON_ERROR_NOT_ENOUGH_CURRENCY;

    return GARRISON_SUCCESS;
}

void ClassHallMissions::StartMission(ObjectGuid npcGuid, uint32 garrMissionId, std::vector<uint64> const& followerDbIds)
{
    if (!IsOpenNpc(npcGuid))
        return;

    auto itr = _missions.find(garrMissionId);
    Mission* mission = itr != _missions.end() ? &itr->second : nullptr;

    ClassHallFollowers& followers = _classHall->GetFollowers();
    std::vector<ClassHallFollowers::Follower const*> team;
    for (uint64 followerDbId : followerDbIds)
        team.push_back(followers.GetFollower(followerDbId));

    WorldPackets::Garrison::GarrisonStartMissionResult startMissionResult;
    startMissionResult.Result = CanStartMission(mission, team);
    startMissionResult.FollowerDBIDs = followerDbIds;
    if (startMissionResult.Result == GARRISON_SUCCESS)
    {
        if (mission->Entry->MissionCost && mission->Entry->MissionCostCurrencyTypesID)
            _owner->RemoveCurrency(mission->Entry->MissionCostCurrencyTypesID, int32(mission->Entry->MissionCost), CurrencyDestroyReason::Garrison);

        mission->PacketInfo.SuccessChance = GetSuccessChance(*mission, team);
        mission->PacketInfo.MissionState = CLASS_HALL_MISSION_STATE_IN_PROGRESS;
        mission->PacketInfo.StartTime = GameTime::GetGameTime();
        for (uint64 followerDbId : followerDbIds)
            followers.SetFollowerMission(followerDbId, garrMissionId);

        ++_sessionMissionCount;
    }

    if (mission)
        startMissionResult.Mission = mission->PacketInfo;
    else
        startMissionResult.Mission.MissionRecID = garrMissionId;

    startMissionResult.SessionMissionCount = _sessionMissionCount;
    _owner->SendDirectMessage(startMissionResult.Write());
}

void ClassHallMissions::CompleteMission(ObjectGuid npcGuid, uint32 garrMissionId)
{
    if (!IsOpenNpc(npcGuid))
        return;

    auto itr = _missions.find(garrMissionId);
    if (itr == _missions.end())
        return;

    Mission& mission = itr->second;
    WorldPackets::Garrison::GarrisonCompleteMissionResult completeMissionResult;
    completeMissionResult.GarrMissionID = garrMissionId;

    time_t endTime = time_t(mission.PacketInfo.StartTime) + mission.Entry->TravelDuration + mission.Entry->MissionDuration;
    if (mission.PacketInfo.MissionState != CLASS_HALL_MISSION_STATE_IN_PROGRESS || endTime > GameTime::GetGameTime())
    {
        completeMissionResult.Result = GARRISON_ERROR_INVALID_MISSION_TIME;
        completeMissionResult.Mission = mission.PacketInfo;
        _owner->SendDirectMessage(completeMissionResult.Write());
        return;
    }

    // a chance of 100 or more always succeeds; what lies above 100 is for the bonus rewards
    completeMissionResult.Succeeded = mission.PacketInfo.SuccessChance >= 100 || roll_chance_i(int32(mission.PacketInfo.SuccessChance));

    ClassHallFollowers& followers = _classHall->GetFollowers();
    for (auto const& [dbId, follower] : followers.GetAllFollowers())
        if (follower.PacketInfo.CurrentMissionID == garrMissionId)
            completeMissionResult.Followers.push_back({ .FollowerDBID = dbId, .Flags = 0 });

    for (WorldPackets::Garrison::GarrisonMissionFollowerInfo const& follower : completeMissionResult.Followers)
        followers.SetFollowerMission(follower.FollowerDBID, 0);

    // a successful mission waits for its chest to be opened; the client takes every completed mission
    // for a successful one, so a failed mission goes back on offer, as one never started
    if (completeMissionResult.Succeeded)
        mission.PacketInfo.MissionState = CLASS_HALL_MISSION_STATE_COMPLETED;
    else
    {
        uint64 dbId = mission.PacketInfo.DbID;
        time_t offerTime = time_t(mission.PacketInfo.OfferTime);
        mission.PacketInfo = {};
        InitMission(mission, mission.Entry, dbId, offerTime);
    }

    completeMissionResult.Mission = mission.PacketInfo;
    _owner->SendDirectMessage(completeMissionResult.Write());
}

void ClassHallMissions::BonusRoll(ObjectGuid npcGuid, uint32 garrMissionId)
{
    if (!IsOpenNpc(npcGuid))
        return;

    auto itr = _missions.find(garrMissionId);
    if (itr == _missions.end())
        return;

    WorldPackets::Garrison::GarrisonMissionBonusRollResult bonusRollResult;
    bonusRollResult.Mission = itr->second.PacketInfo;
    bonusRollResult.GarrMissionID = garrMissionId;

    // the chest of a successful mission is opened once
    if (itr->second.PacketInfo.MissionState != CLASS_HALL_MISSION_STATE_COMPLETED)
    {
        bonusRollResult.Result = GARRISON_ERROR_INVALID_MISSION;
        _owner->SendDirectMessage(bonusRollResult.Write());
        return;
    }

    GiveRewards(garrMissionId);
    _missions.erase(itr);
    _owner->SendDirectMessage(bonusRollResult.Write());
}

void ClassHallMissions::GiveRewards(uint32 garrMissionId)
{
    std::vector<WorldPackets::Garrison::GarrisonMissionReward> const* rewards = sGarrisonMgr.GetClassHallMissionRewards(garrMissionId);
    if (!rewards)
        return;

    for (WorldPackets::Garrison::GarrisonMissionReward const& reward : *rewards)
    {
        // an item that does not fit in the bags is mailed
        if (reward.ItemID && reward.ItemQuantity)
        {
            ItemPosCountVec dest;
            if (_owner->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, reward.ItemID, reward.ItemQuantity) == EQUIP_ERR_OK)
            {
                if (Item* item = _owner->StoreNewItem(dest, reward.ItemID, true, GenerateItemRandomPropertyId(reward.ItemID)))
                    _owner->SendNewItem(item, reward.ItemQuantity, true, false);
            }
            else
                _owner->SendItemRetrievalMail(reward.ItemID, reward.ItemQuantity, ItemContext::NONE);
        }

        // currency 0 is money
        if (reward.CurrencyID)
            _owner->AddCurrency(reward.CurrencyID, reward.CurrencyQuantity, CurrencyGainSource::GarrisonMissionReward);
        else if (reward.CurrencyQuantity)
            _owner->ModifyMoney(reward.CurrencyQuantity);
    }
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
