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

#include "ClassHall.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "Garrison.h"
#include "GarrisonMgr.h"
#include "GarrisonPackets.h"
#include "Log.h"
#include "Player.h"

ClassHall::ClassHall(Player* owner) : _owner(owner), _siteLevel(nullptr)
{
}

bool ClassHall::LoadFromDB(PreparedQueryResult classHall)
{
    if (!classHall)
        return false;

    // SELECT siteLevelId FROM character_class_hall WHERE guid = ?
    uint32 siteLevelId = classHall->Fetch()[0].GetUInt32();
    _siteLevel = sGarrSiteLevelStore.LookupEntry(siteLevelId);
    if (!_siteLevel)
    {
        TC_LOG_ERROR("entities.player", "ClassHall::LoadFromDB: player {} has a class hall with unknown GarrSiteLevel {}, not loaded.",
            _owner->GetGUID().ToString(), siteLevelId);
        return false;
    }

    return true;
}

void ClassHall::SaveToDB(CharacterDatabaseTransaction trans) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_REP_CHARACTER_CLASS_HALL);
    stmt->setUInt64(0, _owner->GetGUID().GetCounter());
    stmt->setUInt32(1, _siteLevel->ID);
    trans->Append(stmt);
}

void ClassHall::DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);
}

bool ClassHall::Create(uint32 garrSiteId)
{
    GarrSiteLevelEntry const* siteLevel = sGarrisonMgr.GetGarrSiteLevelEntry(garrSiteId, 1);
    if (!siteLevel)
        return false;

    _siteLevel = siteLevel;

    WorldPackets::Garrison::GarrisonCreateResult garrisonCreateResult;
    garrisonCreateResult.GarrSiteLevelID = _siteLevel->ID;
    _owner->SendDirectMessage(garrisonCreateResult.Write());
    return true;
}

void ClassHall::BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const
{
    info.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
    info.GarrSiteID = _siteLevel->GarrSiteID;
    info.GarrSiteLevelID = _siteLevel->ID;
}
