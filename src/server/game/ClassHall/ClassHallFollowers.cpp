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

#include "ClassHallFollowers.h"
#include "ClassHall.h"
#include "ConditionMgr.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GarrisonMgr.h"
#include "Log.h"
#include "Player.h"
#include "SpellInfo.h"
#include <algorithm>
#include <set>

namespace
{
// what activating a follower costs, as the client shows it
constexpr int64 FOLLOWER_ACTIVATION_COST = 250 * GOLD;

// troops are the followers that missions use up; champions have no vitality
bool IsTroop(GarrFollowerEntry const* follower)
{
    return follower->Vitality > 0;
}

// an armament raises one of the two item levels: by an amount up to the row's cap, or to a value
bool IsArmorUpgrade(GarrItemLevelUpgradeDataEntry const* upgrade)
{
    return upgrade->Operation == 0 || upgrade->Operation == 2;
}

uint32 GetUpgradedItemLevel(uint32 itemLevel, GarrItemLevelUpgradeDataEntry const* upgrade, int32 amount)
{
    if (upgrade->Operation == 2 || upgrade->Operation == 3)
        return itemLevel >= uint32(upgrade->MaxItemLevel) ? itemLevel : std::min(itemLevel + amount, uint32(upgrade->MaxItemLevel));

    // never lowered
    return std::max(itemLevel, uint32(std::min(amount, upgrade->MaxItemLevel)));
}

GarrFollowerLevelXPEntry const* GetLevelXP(uint32 garrFollowerTypeId, uint32 level)
{
    for (GarrFollowerLevelXPEntry const* entry : sGarrFollowerLevelXPStore)
        if (entry->GarrFollowerTypeID == garrFollowerTypeId && entry->FollowerLevel == level)
            return entry;

    return nullptr;
}
}

ClassHallFollowers::ClassHallFollowers(Player* owner, ClassHall const* classHall) : _owner(owner), _classHall(classHall),
    _activationsRemaining(DAILY_FOLLOWER_ACTIVATIONS)
{
}

void ClassHallFollowers::LoadFromDB(PreparedQueryResult followers)
{
    if (!followers)
        return;

    do
    {
        //           0           1        2      3                4               5   6           7       8          9
        // SELECT dbId, followerId, quality, level, itemLevelWeapon, itemLevelArmor, xp, durability, status, missionId FROM character_class_hall_follower WHERE guid = ?
        Field* fields = followers->Fetch();
        uint64 dbId = fields[0].GetUInt64();
        uint32 followerId = fields[1].GetUInt32();
        GarrFollowerEntry const* entry = sGarrFollowerStore.LookupEntry(followerId);
        if (!entry || entry->GarrTypeID != GARRISON_TYPE_CLASS_ORDER)
        {
            TC_LOG_ERROR("entities.player", "ClassHallFollowers::LoadFromDB: player {} has GarrFollower {} which is not a class hall follower, skipped.",
                _owner->GetGUID().ToString(), followerId);
            continue;
        }

        Follower& follower = _followers[dbId];
        follower.Entry = entry;
        follower.PacketInfo.DbID = dbId;
        follower.PacketInfo.GarrFollowerID = followerId;
        follower.PacketInfo.Quality = fields[2].GetUInt32();
        follower.PacketInfo.FollowerLevel = fields[3].GetUInt32();
        follower.PacketInfo.ItemLevelWeapon = fields[4].GetUInt32();
        follower.PacketInfo.ItemLevelArmor = fields[5].GetUInt32();
        follower.PacketInfo.Xp = fields[6].GetUInt32();
        follower.PacketInfo.Durability = fields[7].GetUInt32();
        follower.PacketInfo.FollowerStatus = fields[8].GetUInt32();
        follower.PacketInfo.CurrentMissionID = fields[9].GetUInt32();
        follower.PacketInfo.AbilityID = GetAbilities(entry, follower.PacketInfo.Quality);
    } while (followers->NextRow());
}

void ClassHallFollowers::SaveToDB(CharacterDatabaseTransaction trans) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_FOLLOWERS);
    stmt->setUInt64(0, _owner->GetGUID().GetCounter());
    trans->Append(stmt);

    for (auto const& [dbId, follower] : _followers)
    {
        uint8 index = 0;
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_CLASS_HALL_FOLLOWER);
        stmt->setUInt64(index++, dbId);
        stmt->setUInt64(index++, _owner->GetGUID().GetCounter());
        stmt->setUInt32(index++, follower.PacketInfo.GarrFollowerID);
        stmt->setUInt32(index++, follower.PacketInfo.Quality);
        stmt->setUInt32(index++, follower.PacketInfo.FollowerLevel);
        stmt->setUInt32(index++, follower.PacketInfo.ItemLevelWeapon);
        stmt->setUInt32(index++, follower.PacketInfo.ItemLevelArmor);
        stmt->setUInt32(index++, follower.PacketInfo.Xp);
        stmt->setUInt32(index++, follower.PacketInfo.Durability);
        stmt->setUInt32(index++, follower.PacketInfo.FollowerStatus);
        stmt->setUInt32(index++, follower.PacketInfo.CurrentMissionID);
        trans->Append(stmt);
    }
}

void ClassHallFollowers::DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_FOLLOWERS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);
}

void ClassHallFollowers::BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const
{
    info.NumFollowerActivationsRemaining = _activationsRemaining;

    for (auto const& [dbId, follower] : _followers)
        info.Followers.push_back(&follower.PacketInfo);
}

GarrisonError ClassHallFollowers::AddFollower(uint32 garrFollowerId)
{
    GarrFollowerEntry const* entry = sGarrFollowerStore.LookupEntry(garrFollowerId);

    WorldPackets::Garrison::GarrisonAddFollowerResult addFollowerResult;
    addFollowerResult.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
    GarrisonError error = CanAddFollower(entry);
    addFollowerResult.Result = error;
    if (error != GARRISON_SUCCESS)
    {
        _owner->SendDirectMessage(addFollowerResult.Write());
        return error;
    }

    // a champion gained while the active ones are at their limit starts inactive
    uint32 status = 0;
    if (IsTroop(entry))
        status = FOLLOWER_STATUS_TROOP;
    else if (GarrFollowerTypeEntry const* followerType = sGarrFollowerTypeStore.LookupEntry(entry->GarrFollowerTypeID))
        if (CountActiveChampions() >= followerType->MaxFollowers)
            status = FOLLOWER_STATUS_INACTIVE;

    uint64 dbId = sGarrisonMgr.GenerateFollowerDbId();
    Follower& follower = _followers[dbId];
    follower.Entry = entry;
    follower.PacketInfo.DbID = dbId;
    follower.PacketInfo.GarrFollowerID = garrFollowerId;
    follower.PacketInfo.Quality = entry->Quality;
    follower.PacketInfo.FollowerLevel = entry->FollowerLevel;
    follower.PacketInfo.ItemLevelWeapon = entry->ItemLevelWeapon;
    follower.PacketInfo.ItemLevelArmor = entry->ItemLevelArmor;
    follower.PacketInfo.Durability = entry->Vitality;
    follower.PacketInfo.FollowerStatus = status;
    follower.PacketInfo.AbilityID = GetAbilities(entry, entry->Quality);

    addFollowerResult.Follower = follower.PacketInfo;
    _owner->SendDirectMessage(addFollowerResult.Write());

    _owner->UpdateCriteria(CriteriaType::RecruitGarrisonFollower, garrFollowerId);
    return GARRISON_SUCCESS;
}

void ClassHallFollowers::RemoveFollower(uint64 dbId)
{
    WorldPackets::Garrison::GarrisonRemoveFollowerResult removeFollowerResult;
    removeFollowerResult.FollowerDBID = dbId;
    removeFollowerResult.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;

    // only troops can be dismissed, and not while they are on a mission
    auto itr = _followers.find(dbId);
    if (itr == _followers.end() || !IsTroop(itr->second.Entry))
        removeFollowerResult.Result = GARRISON_ERROR_INVALID_FOLLOWER;
    else if (itr->second.PacketInfo.CurrentMissionID)
        removeFollowerResult.Result = GARRISON_ERROR_FOLLOWER_ALREADY_ON_MISSION;
    else
    {
        _followers.erase(itr);
        removeFollowerResult.Destroyed = 1;
    }

    _owner->SendDirectMessage(removeFollowerResult.Write());
}

void ClassHallFollowers::SetFollowerInactive(uint64 dbId, bool inactive)
{
    auto itr = _followers.find(dbId);
    Follower* follower = itr != _followers.end() ? &itr->second : nullptr;

    WorldPackets::Garrison::GarrisonFollowerChangedStatus followerChangedStatus;
    followerChangedStatus.Result = CanSetFollowerInactive(follower, inactive);
    if (followerChangedStatus.Result != GARRISON_SUCCESS)
    {
        if (follower)
            followerChangedStatus.Follower = follower->PacketInfo;
        else
            followerChangedStatus.Follower.DbID = dbId;

        _owner->SendDirectMessage(followerChangedStatus.Write());
        return;
    }

    if (inactive)
        follower->PacketInfo.FollowerStatus |= FOLLOWER_STATUS_INACTIVE;
    else
    {
        _owner->ModifyMoney(-FOLLOWER_ACTIVATION_COST, false);
        --_activationsRemaining;
        SendActivationsRemaining();

        follower->PacketInfo.FollowerStatus &= ~FOLLOWER_STATUS_INACTIVE;
    }

    followerChangedStatus.Follower = follower->PacketInfo;
    _owner->SendDirectMessage(followerChangedStatus.Write());
}

ClassHallFollowers::Follower const* ClassHallFollowers::GetFollower(uint64 dbId) const
{
    auto itr = _followers.find(dbId);
    return itr != _followers.end() ? &itr->second : nullptr;
}

// champions are unique: a character has a GarrFollower at most once
ClassHallFollowers::Follower const* ClassHallFollowers::GetChampion(uint32 garrFollowerId) const
{
    for (auto const& [dbId, follower] : _followers)
        if (follower.Entry->ID == garrFollowerId && !IsTroop(follower.Entry))
            return &follower;

    return nullptr;
}

void ClassHallFollowers::SetFollowerMission(uint64 dbId, uint32 garrMissionId)
{
    auto itr = _followers.find(dbId);
    if (itr != _followers.end())
        itr->second.PacketInfo.CurrentMissionID = garrMissionId;
}

// Experience raises a champion's level up to the cap, then its quality; each step keeps what is
// left over, so one gain can give several. At a quality with no next step the experience stops.
// Troops do not grow.
void ClassHallFollowers::AddExperience(uint64 dbId, uint32 amount, uint32 source)
{
    auto itr = _followers.find(dbId);
    if (itr == _followers.end() || IsTroop(itr->second.Entry))
        return;

    Follower& follower = itr->second;
    WorldPackets::Garrison::GarrisonFollowerChangedXp changedXp;
    changedXp.XPEarned = int32(amount);
    changedXp.XPSource = int32(source);
    changedXp.Before = follower.PacketInfo;

    uint32 followerType = follower.Entry->GarrFollowerTypeID;
    uint32 levelCap = GetLevelCap(followerType);
    uint32 quality = follower.PacketInfo.Quality;
    uint32 level = follower.PacketInfo.FollowerLevel;
    uint32 xp = follower.PacketInfo.Xp + amount;
    while (true)
    {
        if (level < levelCap)
        {
            GarrFollowerLevelXPEntry const* levelXp = GetLevelXP(followerType, level);
            if (!levelXp || !levelXp->XpToNextLevel || xp < levelXp->XpToNextLevel)
                break;

            xp -= levelXp->XpToNextLevel;
            ++level;
            continue;
        }

        GarrFollowerQualityEntry const* followerQuality = GetQuality(followerType, quality);
        if (!followerQuality || !followerQuality->XpToNextQuality)
        {
            xp = 0;
            break;
        }

        if (xp < followerQuality->XpToNextQuality)
            break;

        xp -= followerQuality->XpToNextQuality;
        ++quality;
    }

    bool qualityChanged = quality != follower.PacketInfo.Quality;
    follower.PacketInfo.Quality = quality;
    follower.PacketInfo.FollowerLevel = level;
    follower.PacketInfo.Xp = xp;
    // a higher quality can bring an ability or an equipment slot
    if (qualityChanged)
        follower.PacketInfo.AbilityID = GetAbilities(follower.Entry, quality);

    changedXp.After = follower.PacketInfo;
    _owner->SendDirectMessage(changedXp.Write());

    if (qualityChanged)
    {
        WorldPackets::Garrison::GarrisonFollowerChangedQuality changedQuality;
        changedQuality.Follower = follower.PacketInfo;
        _owner->SendDirectMessage(changedQuality.Write());
    }
}

void ClassHallFollowers::SendUpdateFollower(uint64 dbId) const
{
    auto itr = _followers.find(dbId);
    if (itr == _followers.end())
        return;

    WorldPackets::Garrison::GarrisonUpdateFollower updateFollower;
    updateFollower.Follower = itr->second.PacketInfo;
    _owner->SendDirectMessage(updateFollower.Write());
}

void ClassHallFollowers::UpgradeItemLevel(uint64 dbId, GarrItemLevelUpgradeDataEntry const* upgrade, int32 amount)
{
    auto itr = _followers.find(dbId);
    if (itr == _followers.end())
        return;

    WorldPackets::Garrison::GarrisonFollower& follower = itr->second.PacketInfo;
    uint32& itemLevel = IsArmorUpgrade(upgrade) ? follower.ItemLevelArmor : follower.ItemLevelWeapon;
    uint32 upgraded = GetUpgradedItemLevel(itemLevel, upgrade, amount);
    if (upgraded == itemLevel)
        return;

    itemLevel = upgraded;

    WorldPackets::Garrison::GarrisonFollowerChangedItemLevel changedItemLevel;
    changedItemLevel.Follower = follower;
    _owner->SendDirectMessage(changedItemLevel.Write());
}

GarrItemLevelUpgradeDataEntry const* ClassHallFollowers::GetItemLevelUpgrade(int32 garrItemLevelUpgradeDataId)
{
    GarrItemLevelUpgradeDataEntry const* upgrade = sGarrItemLevelUpgradeDataStore.LookupEntry(garrItemLevelUpgradeDataId);
    return upgrade && upgrade->FollowerTypeID == FOLLOWER_TYPE_CLASS_ORDER ? upgrade : nullptr;
}

// the follower item spells of the class hall: armaments name a class hall upgrade row, the others
// name the class hall follower type
bool ClassHallFollowers::IsClassHallFollowerSpell(SpellInfo const* spellInfo)
{
    for (SpellEffectInfo const& effect : spellInfo->GetEffects())
    {
        if (effect.IsEffect(SPELL_EFFECT_MODIFY_FOLLOWER_ITEM_LEVEL) && GetItemLevelUpgrade(effect.MiscValue))
            return true;

        if (effect.IsEffect(SPELL_EFFECT_INCREASE_FOLLOWER_EXPERIENCE) && effect.MiscValueB == FOLLOWER_TYPE_CLASS_ORDER)
            return true;
    }

    return false;
}

// An item is refused before it is used when it would be wasted: no such champion, one that cannot
// take it now, or a change that would change nothing.
SpellCastResult ClassHallFollowers::CheckItemCast(Player const* caster, SpellInfo const* spellInfo, uint32 garrFollowerId)
{
    ClassHall const* classHall = caster->GetClassHall();
    if (!classHall)
        return SPELL_FAILED_GARRISON_NOT_OWNED;

    ClassHallFollowers const& followers = classHall->GetFollowers();
    Follower const* follower = followers.GetChampion(garrFollowerId);
    if (!follower || !follower->IsActive())
        return SPELL_FAILED_BAD_TARGETS;

    if (follower->PacketInfo.CurrentMissionID)
        return SPELL_FAILED_GARRISON_FOLLOWER_ON_MISSION;

    uint32 followerType = follower->Entry->GarrFollowerTypeID;
    bool atLevelCap = follower->PacketInfo.FollowerLevel >= GetLevelCap(followerType);
    bool itemLevelChanges = false;
    bool hasItemLevelUpgrade = false;
    for (SpellEffectInfo const& effect : spellInfo->GetEffects())
    {
        if (effect.IsEffect(SPELL_EFFECT_INCREASE_FOLLOWER_EXPERIENCE))
        {
            // experience at the top quality would be lost
            GarrFollowerQualityEntry const* quality = GetQuality(followerType, follower->PacketInfo.Quality);
            if (atLevelCap && (!quality || !quality->XpToNextQuality))
                return SPELL_FAILED_GARRISON_FOLLOWER_MAX_QUALITY;
        }
        else if (effect.IsEffect(SPELL_EFFECT_MODIFY_FOLLOWER_ITEM_LEVEL))
        {
            GarrItemLevelUpgradeDataEntry const* upgrade = GetItemLevelUpgrade(effect.MiscValue);
            if (!upgrade)
                continue;

            hasItemLevelUpgrade = true;
            uint32 itemLevel = IsArmorUpgrade(upgrade) ? follower->PacketInfo.ItemLevelArmor : follower->PacketInfo.ItemLevelWeapon;
            if (GetUpgradedItemLevel(itemLevel, upgrade, effect.CalcValue(caster)) != itemLevel)
                itemLevelChanges = true;
        }
    }

    if (hasItemLevelUpgrade)
    {
        // a champion has an item level only at the level cap
        if (!atLevelCap)
            return SPELL_FAILED_GARRISON_FOLLOWER_NOT_MAX_LEVEL;

        if (!itemLevelChanges)
            return SPELL_FAILED_GARRISON_FOLLOWER_MAX_ITEM_LEVEL;
    }

    return SPELL_CAST_OK;
}

void ClassHallFollowers::ResetActivations()
{
    _activationsRemaining = DAILY_FOLLOWER_ACTIVATIONS;
    SendActivationsRemaining();
}

void ClassHallFollowers::SendClassSpecCategoryInfo(uint32 garrFollowerTypeId) const
{
    WorldPackets::Garrison::GarrisonGetClassSpecCategoryInfoResult categoryInfoResult;
    categoryInfoResult.GarrFollowerTypeID = garrFollowerTypeId;

    if (garrFollowerTypeId == FOLLOWER_TYPE_CLASS_ORDER)
    {
        // the kinds of troops the class of the owner has
        std::set<uint32> classSpecs;
        for (GarrFollowerEntry const* follower : sGarrFollowerStore)
            if (follower->GarrTypeID == GARRISON_TYPE_CLASS_ORDER && follower->ChrClassID == _owner->GetClass() && IsTroop(follower))
                classSpecs.insert(GetClassSpec(follower));

        for (uint32 classSpec : classSpecs)
        {
            // a kind goes by the last of its names whose condition the owner meets
            GarrClassSpecPlayerCondEntry const* current = nullptr;
            for (GarrClassSpecPlayerCondEntry const* classSpecPlayerCond : sGarrClassSpecPlayerCondStore)
            {
                if (classSpecPlayerCond->GarrClassSpecID != classSpec)
                    continue;

                if (current && current->OrderIndex >= classSpecPlayerCond->OrderIndex)
                    continue;

                if (ConditionMgr::IsPlayerMeetingCondition(_owner, classSpecPlayerCond->PlayerConditionID))
                    current = classSpecPlayerCond;
            }

            if (!current)
                continue;

            WorldPackets::Garrison::GarrisonClassSpecCategoryInfo& categoryInfo = categoryInfoResult.CategoryInfos.emplace_back();
            categoryInfo.GarrClassSpecID = classSpec;
            categoryInfo.GarrClassSpecPlayerCondID = current->ID;
        }
    }

    _owner->SendDirectMessage(categoryInfoResult.Write());
}

void ClassHallFollowers::SendActivationsRemaining() const
{
    WorldPackets::Garrison::GarrisonNumFollowerActivationsRemaining activationsRemaining;
    activationsRemaining.GarrSiteID = _classHall->GetGarrSiteID();
    activationsRemaining.Activations = _activationsRemaining;
    _owner->SendDirectMessage(activationsRemaining.Write());
}

GarrisonError ClassHallFollowers::CanAddFollower(GarrFollowerEntry const* follower) const
{
    if (!follower || follower->GarrTypeID != GARRISON_TYPE_CLASS_ORDER)
        return GARRISON_ERROR_INVALID_FOLLOWER;

    if (follower->ChrClassID && follower->ChrClassID != _owner->GetClass())
        return GARRISON_ERROR_INVALID_CLASS;

    if (follower->Flags & GARRISON_FOLLOWER_FLAG_UNIQUE)
        for (auto const& [dbId, owned] : _followers)
            if (owned.Entry == follower)
                return GARRISON_ERROR_FOLLOWER_EXISTS;

    if (IsTroop(follower))
    {
        // each kind of troop has its own limit
        GarrClassSpecEntry const* classSpec = sGarrClassSpecStore.LookupEntry(GetClassSpec(follower));
        if (!classSpec)
            return GARRISON_ERROR_INVALID_FOLLOWER;

        if (CountTroops(classSpec->ID) >= classSpec->FollowerClassLimit)
            return GARRISON_ERROR_AT_CLASS_SPEC_CAP;
    }

    return GARRISON_SUCCESS;
}

GarrisonError ClassHallFollowers::CanSetFollowerInactive(Follower const* follower, bool inactive) const
{
    // only champions can be set aside, and not while they are on a mission
    if (!follower || IsTroop(follower->Entry))
        return GARRISON_ERROR_INVALID_FOLLOWER;

    if (follower->PacketInfo.CurrentMissionID)
        return GARRISON_ERROR_FOLLOWER_ALREADY_ON_MISSION;

    if (inactive)
        return follower->IsActive() ? GARRISON_SUCCESS : GARRISON_ERROR_FOLLOWER_INACTIVE;

    if (follower->IsActive())
        return GARRISON_ERROR_FOLLOWER_ACTIVE;

    GarrFollowerTypeEntry const* followerType = sGarrFollowerTypeStore.LookupEntry(follower->Entry->GarrFollowerTypeID);
    if (followerType && CountActiveChampions() >= followerType->MaxFollowers)
        return GARRISON_ERROR_AT_FOLLOWER_HARD_CAP;

    if (!_activationsRemaining)
        return GARRISON_ERROR_FOLLOWER_ACTIVATION_UNAVAILABLE;

    if (!_owner->HasEnoughMoney(FOLLOWER_ACTIVATION_COST))
        return GARRISON_ERROR_NOT_ENOUGH_GOLD;

    return GARRISON_SUCCESS;
}

// the level a follower of this type stops at: the one that needs no experience to a next level
uint32 ClassHallFollowers::GetLevelCap(uint32 garrFollowerTypeId)
{
    uint32 levelCap = 0;
    for (GarrFollowerLevelXPEntry const* entry : sGarrFollowerLevelXPStore)
        if (entry->GarrFollowerTypeID == garrFollowerTypeId && !entry->XpToNextLevel)
            levelCap = std::max<uint32>(levelCap, entry->FollowerLevel);

    return levelCap;
}

GarrFollowerQualityEntry const* ClassHallFollowers::GetQuality(uint32 garrFollowerTypeId, uint32 quality)
{
    for (GarrFollowerQualityEntry const* entry : sGarrFollowerQualityStore)
        if (entry->GarrFollowerTypeID == garrFollowerTypeId && entry->Quality == quality)
            return entry;

    return nullptr;
}

std::list<GarrAbilityEntry const*> ClassHallFollowers::GetAbilities(GarrFollowerEntry const* follower, uint32 quality) const
{
    std::list<GarrAbilityEntry const*> abilities;

    // how many abilities and traits a follower of this type has at this quality
    GarrFollowerQualityEntry const* followerQuality = GetQuality(follower->GarrFollowerTypeID, quality);
    if (!followerQuality)
        return abilities;

    uint32 factionIndex = _owner->GetTeam() == HORDE ? GARRISON_FACTION_INDEX_HORDE : GARRISON_FACTION_INDEX_ALLIANCE;
    uint32 abilityCount = 0;
    uint32 traitCount = 0;

    // the client data lists every ability the follower can ever have, in the order it gains them
    for (GarrFollowerXAbilityEntry const* followerAbility : sGarrFollowerXAbilityStore)
    {
        if (followerAbility->GarrFollowerID != follower->ID || followerAbility->FactionIndex != factionIndex)
            continue;

        GarrAbilityEntry const* ability = sGarrAbilityStore.LookupEntry(followerAbility->GarrAbilityID);
        if (!ability)
            continue;

        if (ability->Flags & GARRISON_ABILITY_FLAG_TRAIT)
        {
            if (traitCount >= followerQuality->TraitCount)
                continue;

            ++traitCount;
        }
        else
        {
            if (abilityCount >= followerQuality->AbilityCount)
                continue;

            ++abilityCount;
        }

        abilities.push_back(ability);
    }

    return abilities;
}

uint32 ClassHallFollowers::GetClassSpec(GarrFollowerEntry const* follower) const
{
    return _owner->GetTeam() == HORDE ? follower->HordeGarrClassSpecID : follower->AllianceGarrClassSpecID;
}

uint32 ClassHallFollowers::CountActiveChampions() const
{
    return uint32(std::count_if(_followers.begin(), _followers.end(), [](auto const& follower)
    {
        return !IsTroop(follower.second.Entry) && follower.second.IsActive();
    }));
}

uint32 ClassHallFollowers::CountTroops(uint32 garrClassSpecId) const
{
    return uint32(std::count_if(_followers.begin(), _followers.end(), [&](auto const& follower)
    {
        return IsTroop(follower.second.Entry) && GetClassSpec(follower.second.Entry) == garrClassSpecId;
    }));
}
