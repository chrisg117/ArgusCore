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
#include "ConditionMgr.h"
#include "DatabaseEnv.h"
#include "DB2Stores.h"
#include "GameTime.h"
#include "GarrisonPackets.h"
#include "Log.h"
#include "Player.h"
#include "SpellInfo.h"
#include "SpellMgr.h"

namespace
{
struct TalentCost
{
    uint32 CurrencyId;
    int32 CurrencyAmount;
    int32 Gold;
};

TalentCost GetTalentCost(GarrTalentEntry const* talent, bool isSwap)
{
    if (isSwap)
        return { talent->RespecCostCurrencyTypesID, talent->RespecCost, talent->RespecGoldCost };

    return { talent->ResearchCostCurrencyTypesID, talent->ResearchCost, talent->ResearchGoldCost };
}
}

time_t ClassHallTalents::Talent::GetResearchEndTime() const
{
    return ResearchStartTime + ((Flags & CLASS_HALL_TALENT_FLAG_RESPEC) ? Entry->RespecDurationSecs : Entry->ResearchDurationSecs);
}

ClassHallTalents::ClassHallTalents(Player* owner) : _owner(owner), _researchEndTime(0), _passivePerksApplied(false)
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

    UpdateResearchEndTime();
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

void ClassHallTalents::Update()
{
    if (!_passivePerksApplied)
    {
        // passive auras are not saved, and nothing can be cast while the character is still loading
        for (auto const& [talentId, talent] : _talents)
            if (talent.IsResearched())
                if (SpellInfo const* perkSpell = sSpellMgr->GetSpellInfo(talent.Entry->PerkSpellID, DIFFICULTY_NONE))
                    if (perkSpell->IsPassive())
                        ApplyPerk(talent);

        _passivePerksApplied = true;
    }

    time_t now = GameTime::GetGameTime();
    if (!_researchEndTime || _researchEndTime > now)
        return;

    for (auto& [talentId, talent] : _talents)
        if (!talent.IsResearched() && talent.GetResearchEndTime() <= now)
            FinishResearch(talent);

    UpdateResearchEndTime();
}

void ClassHallTalents::FinishResearch(Talent& talent)
{
    talent.Flags = (talent.Flags & ~CLASS_HALL_TALENT_FLAG_RESPEC) | CLASS_HALL_TALENT_FLAG_RESEARCHED;

    WorldPackets::Garrison::GarrisonTalentCompleted talentCompleted;
    talentCompleted.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
    talentCompleted.GarrTalentID = talent.Entry->ID;
    talentCompleted.Flags = talent.Flags;
    _owner->SendDirectMessage(talentCompleted.Write());

    ApplyPerk(talent);
}

void ClassHallTalents::ApplyPerk(Talent const& talent)
{
    uint32 spellId = talent.Entry->PerkSpellID;
    if (!spellId || _owner->HasAura(spellId))
        return;

    if (!ConditionMgr::IsPlayerMeetingCondition(_owner, talent.Entry->PerkPlayerConditionID))
        return;

    if (!sSpellMgr->GetSpellInfo(spellId, DIFFICULTY_NONE))
    {
        TC_LOG_ERROR("entities.player", "ClassHallTalents::ApplyPerk: GarrTalent {} has unknown perk spell {}, not cast.",
            talent.Entry->ID, spellId);
        return;
    }

    _owner->CastSpell(_owner, spellId, true);
}

void ClassHallTalents::RemovePerk(Talent const& talent)
{
    if (uint32 spellId = talent.Entry->PerkSpellID)
        _owner->RemoveAurasDueToSpell(spellId);
}

void ClassHallTalents::UpdateResearchEndTime()
{
    _researchEndTime = 0;
    for (auto const& [talentId, talent] : _talents)
        if (!talent.IsResearched() && (!_researchEndTime || talent.GetResearchEndTime() < _researchEndTime))
            _researchEndTime = talent.GetResearchEndTime();
}

void ClassHallTalents::Research(uint32 garrTalentId)
{
    GarrTalentEntry const* entry = sGarrTalentStore.LookupEntry(garrTalentId);
    GarrisonError result = CanResearch(entry);

    WorldPackets::Garrison::GarrisonResearchTalentResult researchTalentResult;
    researchTalentResult.Result = result;
    researchTalentResult.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
    researchTalentResult.Talent.GarrTalentID = garrTalentId;
    if (result == GARRISON_SUCCESS)
    {
        Talent const* replaced = GetResearchedTalentInRow(entry);
        TalentCost cost = GetTalentCost(entry, replaced != nullptr);
        if (cost.CurrencyAmount > 0)
            _owner->RemoveCurrency(cost.CurrencyId, cost.CurrencyAmount, CurrencyDestroyReason::Garrison);

        if (cost.Gold > 0)
            _owner->ModifyMoney(-int64(cost.Gold) * GOLD, false);

        uint32 flags = 0;
        if (replaced)
        {
            // a swap: the row's researched talent goes at once, the new one takes the respec time
            RemovePerk(*replaced);

            WorldPackets::Garrison::GarrisonTalentRemoved talentRemoved;
            talentRemoved.GarrTypeID = GARRISON_TYPE_CLASS_ORDER;
            talentRemoved.GarrTalentID = replaced->Entry->ID;
            _owner->SendDirectMessage(talentRemoved.Write());

            _talents.erase(replaced->Entry->ID);
            flags = CLASS_HALL_TALENT_FLAG_RESPEC;
        }

        Talent& talent = _talents[garrTalentId];
        talent.Entry = entry;
        talent.ResearchStartTime = GameTime::GetGameTime();
        talent.Flags = flags;

        researchTalentResult.Talent.ResearchStartTime = talent.ResearchStartTime;
        researchTalentResult.Talent.Flags = talent.Flags;

        UpdateResearchEndTime();
    }

    _owner->SendDirectMessage(researchTalentResult.Write());
}

bool ClassHallTalents::HasTalent(uint32 garrTalentId) const
{
    return _talents.find(garrTalentId) != _talents.end();
}

bool ClassHallTalents::IsResearched(uint32 garrTalentId) const
{
    auto itr = _talents.find(garrTalentId);
    return itr != _talents.end() && itr->second.IsResearched();
}

bool ClassHallTalents::IsResearching(uint32 garrTalentId) const
{
    auto itr = _talents.find(garrTalentId);
    return itr != _talents.end() && !itr->second.IsResearched();
}

GarrisonError ClassHallTalents::CanResearch(GarrTalentEntry const* talent) const
{
    if (!talent)
        return GARRISON_ERROR_INVALID_TALENT;

    GarrTalentTreeEntry const* tree = sGarrTalentTreeStore.LookupEntry(talent->GarrTalentTreeID);
    if (!tree || tree->GarrTypeID != GARRISON_TYPE_CLASS_ORDER)
        return GARRISON_ERROR_INVALID_TALENT;

    if (tree->ClassID != int32(_owner->GetClass()))
        return GARRISON_ERROR_INVALID_CLASS;

    if (HasTalent(talent->ID))
        return GARRISON_ERROR_INVALID_TALENT;

    // only one talent can be researched at a time
    bool previousRowResearched = talent->Tier == 0;
    for (auto const& [otherId, other] : _talents)
    {
        if (!other.IsResearched())
            return GARRISON_ERROR_ALREADY_RESEARCHING_TALENT;

        if (other.Entry->GarrTalentTreeID == talent->GarrTalentTreeID && other.Entry->Tier == talent->Tier - 1)
            previousRowResearched = true;
    }

    if (!ConditionMgr::IsPlayerMeetingCondition(_owner, talent->PlayerConditionID))
        return GARRISON_ERROR_FAILED_CONDITION;

    // a row opens when a talent of the row before it is researched
    if (!previousRowResearched)
        return GARRISON_ERROR_INVALID_TIER;

    // choosing another talent in a row that has a researched one is a swap, with its own cost
    TalentCost cost = GetTalentCost(talent, GetResearchedTalentInRow(talent) != nullptr);
    if (cost.CurrencyAmount > 0 && !_owner->HasCurrency(cost.CurrencyId, cost.CurrencyAmount))
        return GARRISON_ERROR_NOT_ENOUGH_CURRENCY;

    if (cost.Gold > 0 && !_owner->HasEnoughMoney(uint64(cost.Gold) * GOLD))
        return GARRISON_ERROR_NOT_ENOUGH_GOLD;

    return GARRISON_SUCCESS;
}

ClassHallTalents::Talent const* ClassHallTalents::GetResearchedTalentInRow(GarrTalentEntry const* talent) const
{
    for (auto const& [otherId, other] : _talents)
        if (other.IsResearched() && other.Entry != talent && other.Entry->GarrTalentTreeID == talent->GarrTalentTreeID
            && other.Entry->Tier == talent->Tier)
            return &other;

    return nullptr;
}
