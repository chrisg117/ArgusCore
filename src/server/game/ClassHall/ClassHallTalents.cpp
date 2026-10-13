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

#include "ClassHallTalents.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GarrisonPackets.h"
#include "Log.h"
#include "Player.h"

ClassHallTalents::ClassHallTalents(Player* owner) : _owner(owner)
{
}

void ClassHallTalents::LoadFromDB(PreparedQueryResult talents)
{
    if (!talents)
        return;

    do
    {
        // SELECT talentId, researchStartTime, flags FROM character_class_hall_talent WHERE guid = ?
        Field* fields = talents->Fetch();
        uint32 talentId = fields[0].GetUInt32();
        GarrTalentEntry const* entry = sGarrTalentStore.LookupEntry(talentId);
        if (!entry)
        {
            TC_LOG_ERROR("entities.player", "ClassHallTalents::LoadFromDB: player {} has unknown GarrTalent {}, skipped.",
                _owner->GetGUID().ToString(), talentId);
            continue;
        }

        Talent& talent = _talents[talentId];
        talent.Entry = entry;
        talent.ResearchStartTime = fields[1].GetInt64();
        talent.Flags = fields[2].GetUInt32();
    } while (talents->NextRow());
}

void ClassHallTalents::SaveToDB(CharacterDatabaseTransaction trans) const
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_TALENTS);
    stmt->setUInt64(0, _owner->GetGUID().GetCounter());
    trans->Append(stmt);

    for (auto const& [talentId, talent] : _talents)
    {
        stmt = CharacterDatabase.GetPreparedStatement(CHAR_INS_CHARACTER_CLASS_HALL_TALENT);
        stmt->setUInt64(0, _owner->GetGUID().GetCounter());
        stmt->setUInt32(1, talentId);
        stmt->setInt64(2, talent.ResearchStartTime);
        stmt->setUInt32(3, talent.Flags);
        trans->Append(stmt);
    }
}

void ClassHallTalents::DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans)
{
    CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_DEL_CHARACTER_CLASS_HALL_TALENTS);
    stmt->setUInt64(0, ownerGuid);
    trans->Append(stmt);
}

void ClassHallTalents::BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const
{
    for (auto const& [talentId, talent] : _talents)
    {
        WorldPackets::Garrison::GarrisonTalent& packetTalent = info.Talents.emplace_back();
        packetTalent.GarrTalentID = talentId;
        packetTalent.ResearchStartTime = talent.ResearchStartTime;
        packetTalent.Flags = talent.Flags;
    }
}
