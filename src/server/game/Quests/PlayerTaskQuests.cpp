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

#include "PlayerTaskQuests.h"
#include "ConditionMgr.h"
#include "Containers.h"
#include "DB2Stores.h"
#include "DisableMgr.h"
#include "Log.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include <vector>

static_assert(MAX_QUEST_COUNTS == 24, "PlayerTaskQuests::OffLogCounts must match MAX_QUEST_COUNTS");

namespace
{
bool Refuse(std::string* why, char const* filter)
{
    if (why)
        *why = filter;
    return false;
}
}

bool PlayerTaskQuests::IsEligible(Quest const* quest, std::string* why /*= nullptr*/) const
{
    uint32 questId = quest->GetQuestId();
    if (_owner.GetQuestRewardStatus(questId))
        return Refuse(why, "rewarded");

    if (DisableMgr::IsDisabledFor(DISABLE_TYPE_QUEST, questId, &_owner))
        return Refuse(why, "disabled");

    if (!_owner.CanTakeQuest(quest, false))
        return Refuse(why, "quest");

    QuestV2CliTaskEntry const* task = sQuestV2CliTaskStore.LookupEntry(questId);
    if (!task)
        return true;

    if (task->FiltRaces.RawValue != -1 && !task->FiltRaces.IsEmpty() && !task->FiltRaces.HasRace(_owner.GetRace()))
        return Refuse(why, "race");

    if (task->FiltClasses != 0 && task->FiltClasses != -1 && !(uint16(task->FiltClasses) & _owner.GetClassMask()))
        return Refuse(why, "class");

    uint8 level = _owner.GetLevel();
    if (level < task->FiltMinLevel || (task->FiltMaxLevel && task->FiltMaxLevel != 255 && level > task->FiltMaxLevel))
        return Refuse(why, "level");

    // logic 2: any one of the listed quests; every other value: all of them
    bool anyOf = task->FiltCompletedQuestLogic == 2;
    bool listed = false, anyDone = false, allDone = true;
    for (uint16 completedQuestId : task->FiltCompletedQuest)
    {
        if (!completedQuestId)
            continue;

        listed = true;
        if (_owner.GetQuestRewardStatus(completedQuestId))
            anyDone = true;
        else
            allDone = false;
    }
    if (listed && !(anyOf ? anyDone : allDone))
        return Refuse(why, "completed");

    if (task->FiltMinFactionID && _owner.GetReputation(task->FiltMinFactionID) < int32(task->FiltMinFactionValue))
        return Refuse(why, "faction");

    if (task->FiltMaxFactionID && _owner.GetReputation(task->FiltMaxFactionID) > int32(task->FiltMaxFactionValue))
        return Refuse(why, "faction");

    if (task->FiltMinSkillID && _owner.GetSkillValue(task->FiltMinSkillID) < task->FiltMinSkillValue)
        return Refuse(why, "skill");

    if (!ConditionMgr::IsPlayerMeetingCondition(&_owner, task->ConditionID))
        return Refuse(why, "condition");

    if (task->WorldStateExpressionID > 0)
        if (WorldStateExpressionEntry const* expression = sWorldStateExpressionStore.LookupEntry(task->WorldStateExpressionID))
            if (!ConditionMgr::IsMeetingWorldStateExpression(_owner.GetMap(), expression))
                return Refuse(why, "worldstate");

    // FiltActiveQuest / FiltNonActiveQuest are zero in every 7.3.5 row; not checked
    return true;
}

void PlayerTaskQuests::OnAreaChanged(uint32 /*oldArea*/, uint32 newArea)
{
    // Map::AddPlayerToMap updates the zone with area 0 before the real one
    if (!newArea)
        return;

    // both lists first: granting and revoking change m_QuestStatus
    std::vector<uint32> revoke;
    std::vector<Quest const*> grant;
    for (auto const& [questId, status] : _owner.m_QuestStatus)
    {
        if (status.Status != QUEST_STATUS_INCOMPLETE && status.Status != QUEST_STATUS_COMPLETE)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest || !quest->IsTask())
            continue;

        bool inArea = sObjectMgr->IsTaskQuestArea(questId, newArea);
        if (status.Slot == QUEST_SLOT_NONE)
        {
            // held task coming back: no eligibility check, CanTakeQuest refuses any quest already held
            if (inArea)
                grant.push_back(quest);
        }
        else if (!inArea)
            revoke.push_back(questId);
    }

    for (Quest const* quest : sObjectMgr->GetTaskQuestsForArea(newArea))
        if (!_owner.m_QuestStatus.contains(quest->GetQuestId()) && IsEligible(quest))
            grant.push_back(quest);

    // a bar that filled as the player crossed the area line is rewarded, not carried off-log
    for (uint32 questId : revoke)
    {
        _owner.RewardAutoCompleteTask(questId);
        Revoke(questId);
    }

    for (Quest const* quest : grant)
        Grant(quest);
}

void PlayerTaskQuests::Grant(Quest const* quest)
{
    uint16 slot = _owner.FindTaskQuestSlot(0);
    if (slot == QUEST_SLOT_NONE)
    {
        TC_LOG_DEBUG("entities.player.quest", "PlayerTaskQuests::Grant: player {} has no free task quest slot for quest {}",
            _owner.GetGUID().ToString(), quest->GetQuestId());
        return;
    }

    bool held = _owner.m_QuestStatus.contains(quest->GetQuestId());
    _owner.AddQuestInternal(quest, nullptr, slot, held);
    if (held)
        _owner.RewardAutoCompleteTask(quest->GetQuestId());
}

void PlayerTaskQuests::Revoke(uint32 questId)
{
    auto itr = _owner.m_QuestStatus.find(questId);
    if (itr == _owner.m_QuestStatus.end() || itr->second.Slot == QUEST_SLOT_NONE)
        return;

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return;

    uint16 slot = itr->second.Slot;
    std::array<int32, OffLogCounts>& progress = _offLogProgress[questId];
    progress.fill(0);
    for (QuestObjective const& objective : quest->GetObjectives())
        if (objective.StorageIndex >= 0 && objective.StorageIndex < int8(OffLogCounts))
            progress[objective.StorageIndex] = _owner.GetQuestSlotObjectiveData(slot, objective);

    _owner.SetQuestSlot(slot, 0);
    itr->second.Slot = QUEST_SLOT_NONE;

    // no credit outside the task's area
    for (auto objectiveItr = _owner.m_questObjectiveStatus.begin(); objectiveItr != _owner.m_questObjectiveStatus.end(); )
    {
        if (objectiveItr->second.QuestStatusItr == itr)
            objectiveItr = _owner.m_questObjectiveStatus.erase(objectiveItr);
        else
            ++objectiveItr;
    }

    _owner.m_QuestStatusSave[questId] = QUEST_DEFAULT_SAVE_TYPE;
    _owner.SendQuestUpdate(questId);
}

int32 PlayerTaskQuests::GetOffLogObjectiveData(uint32 questId, QuestObjective const& objective) const
{
    if (objective.StorageIndex < 0 || objective.StorageIndex >= int8(OffLogCounts))
        return 0;

    auto itr = _offLogProgress.find(questId);
    return itr != _offLogProgress.end() ? itr->second[objective.StorageIndex] : 0;
}

void PlayerTaskQuests::SetOffLogObjectiveData(uint32 questId, QuestObjective const& objective, int32 value)
{
    if (objective.StorageIndex < 0 || objective.StorageIndex >= int8(OffLogCounts))
        return;

    auto [itr, inserted] = _offLogProgress.try_emplace(questId);
    if (inserted)
        itr->second.fill(0);
    itr->second[objective.StorageIndex] = value;
}

bool PlayerTaskQuests::HasOffLogProgress(uint32 questId) const
{
    return _offLogProgress.contains(questId);
}

void PlayerTaskQuests::Forget(uint32 questId)
{
    _offLogProgress.erase(questId);
}

void PlayerTaskQuests::RestoreToSlot(Quest const* quest, uint16 slot)
{
    auto itr = _offLogProgress.find(quest->GetQuestId());
    if (itr == _offLogProgress.end())
        return;

    for (QuestObjective const& objective : quest->GetObjectives())
    {
        if (objective.StorageIndex < 0 || objective.StorageIndex >= int8(OffLogCounts))
            continue;

        int32 data = std::min(itr->second[objective.StorageIndex], objective.Amount);
        if (!objective.IsStoringFlag())
            _owner.SetQuestSlotCounter(slot, objective.StorageIndex, uint16(std::max(data, 0)));
        else if (data)
            _owner.SetQuestSlotObjectiveFlag(slot, objective.StorageIndex);
    }

    _offLogProgress.erase(itr);
}
