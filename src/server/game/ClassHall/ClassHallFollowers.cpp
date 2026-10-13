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
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "Garrison.h"
#include "Log.h"
#include "Player.h"

ClassHallFollowers::ClassHallFollowers(Player* owner) : _owner(owner)
{
}

void ClassHallFollowers::LoadFromDB(PreparedQueryResult followers)
{
    if (!followers)
        return;

    do
    {
        //           0           1        2      3                4               5   6           7       8
        // SELECT dbId, followerId, quality, level, itemLevelWeapon, itemLevelArmor, xp, durability, status FROM character_class_hall_follower WHERE guid = ?
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
    for (auto const& [dbId, follower] : _followers)
        info.Followers.push_back(&follower.PacketInfo);
}

std::list<GarrAbilityEntry const*> ClassHallFollowers::GetAbilities(GarrFollowerEntry const* follower, uint32 quality) const
{
    std::list<GarrAbilityEntry const*> abilities;

    // how many abilities and traits a follower of this type has at this quality
    GarrFollowerQualityEntry const* followerQuality = nullptr;
    for (GarrFollowerQualityEntry const* entry : sGarrFollowerQualityStore)
    {
        if (entry->GarrFollowerTypeID == follower->GarrFollowerTypeID && entry->Quality == quality)
        {
            followerQuality = entry;
            break;
        }
    }

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
