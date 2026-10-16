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
#include <array>
#include <list>
#include <map>

class ClassHall;
class Player;
class SpellInfo;
struct GarrAbilityEntry;
struct GarrFollowerEntry;
struct GarrFollowerQualityEntry;
struct GarrItemLevelUpgradeDataEntry;
enum SpellCastResult : int32;

// The followers (champions and troops) of a character's class hall.
// where a follower's experience came from, as the experience message tells the client
enum ClassHallFollowerExperienceSource : uint32
{
    FOLLOWER_EXPERIENCE_SOURCE_MISSION  = 0,
    FOLLOWER_EXPERIENCE_SOURCE_ITEM     = 1
};

class TC_GAME_API ClassHallFollowers
{
public:
    // followers a character may activate per day
    static constexpr uint32 DAILY_FOLLOWER_ACTIVATIONS = 1;

    // a champion's equipment slots: the empty-slot traits of its abilities
    static constexpr uint32 MAX_EQUIPMENT_SLOTS = 3;

    struct Follower
    {
        GarrFollowerEntry const* Entry = nullptr;
        WorldPackets::Garrison::GarrisonFollower PacketInfo;
        std::array<uint32, MAX_EQUIPMENT_SLOTS> Equipment = { };  // the trait in each equipment slot, 0 for none

        bool IsActive() const { return (PacketInfo.FollowerStatus & FOLLOWER_STATUS_INACTIVE) == 0; }
        bool IsTroop() const { return (PacketInfo.FollowerStatus & FOLLOWER_STATUS_TROOP) != 0; }

        // a troop missions have used up
        bool IsExpired() const { return IsTroop() && !PacketInfo.Durability; }
    };

    ClassHallFollowers(Player* owner, ClassHall const* classHall);

    void LoadFromDB(PreparedQueryResult followers, PreparedQueryResult equipment);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

    GarrisonError AddFollower(uint32 garrFollowerId);
    void RemoveFollower(uint64 dbId);
    void SetFollowerInactive(uint64 dbId, bool inactive);

    Follower const* GetFollower(uint64 dbId) const;
    Follower const* GetChampion(uint32 garrFollowerId) const;
    std::map<uint64 /*dbId*/, Follower> const& GetAllFollowers() const { return _followers; }
    void SetFollowerMission(uint64 dbId, uint32 garrMissionId);
    void SetVitality(uint64 dbId, uint32 vitality);
    void RemoveExpiredTroop(uint64 dbId, bool notify);
    void AddExperience(uint64 dbId, uint32 amount, uint32 source);
    void SendUpdateFollower(uint64 dbId) const;
    void UpgradeItemLevel(uint64 dbId, GarrItemLevelUpgradeDataEntry const* upgrade, int32 amount);
    void Equip(uint64 dbId, uint32 slot, GarrAbilityEntry const* trait);
    static int32 GetEquipmentSlot(Follower const& follower, uint32 abilityId);

    // checks an item's spell aimed at a champion (the client names the champion by its GarrFollower id)
    static SpellCastResult CheckItemCast(Player const* caster, SpellInfo const* spellInfo, uint32 garrFollowerId, uint32 abilityId);
    static GarrItemLevelUpgradeDataEntry const* GetItemLevelUpgrade(int32 garrItemLevelUpgradeDataId);
    static bool IsClassHallFollowerSpell(SpellInfo const* spellInfo);

    uint32 GetActivationsRemaining() const { return _activationsRemaining; }
    void SetActivationsRemaining(uint32 activations) { _activationsRemaining = activations; }
    void ResetActivations();

    void SendClassSpecCategoryInfo(uint32 garrFollowerTypeId) const;

    GarrisonError CanAddFollower(GarrFollowerEntry const* follower) const;
    uint32 GetClassSpec(GarrFollowerEntry const* follower) const;
    uint32 CountTroops(uint32 garrClassSpecId) const;

    static uint32 GetLevelCap(uint32 garrFollowerTypeId);
    static GarrFollowerQualityEntry const* GetQuality(uint32 garrFollowerTypeId, uint32 quality);

private:
    GarrisonError CanSetFollowerInactive(Follower const* follower, bool inactive) const;
    void SendActivationsRemaining() const;
    std::list<GarrAbilityEntry const*> GetAbilities(GarrFollowerEntry const* follower, uint32 quality,
        std::array<uint32, MAX_EQUIPMENT_SLOTS> const& equipment = { }) const;
    uint32 CountActiveChampions() const;

    Player* _owner;
    ClassHall const* _classHall;
    std::map<uint64 /*dbId*/, Follower> _followers;
    uint32 _activationsRemaining;
};

#endif // ClassHallFollowers_h__
