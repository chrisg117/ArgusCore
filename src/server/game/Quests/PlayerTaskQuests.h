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

#ifndef PlayerTaskQuests_h__
#define PlayerTaskQuests_h__

#include "Define.h"
#include <array>
#include <string>
#include <unordered_map>

class Player;
class Quest;
struct QuestObjective;

// Task quests (bonus objectives) of one player: granted into quest log slots 25..49 while the
// player stands in one of the task's areas, revoked when they leave. The progress of a held task
// that has no slot is kept here, off the log, and restored when the task is granted again.
class TC_GAME_API PlayerTaskQuests
{
public:
    explicit PlayerTaskQuests(Player& owner) : _owner(owner) { }

    PlayerTaskQuests(PlayerTaskQuests const&) = delete;
    PlayerTaskQuests& operator=(PlayerTaskQuests const&) = delete;

    // the quest's own checks plus the QuestV2CliTask.db2 filters; `why` gets the first failing filter
    bool IsEligible(Quest const* quest, std::string* why = nullptr) const;

    void OnAreaChanged(uint32 oldArea, uint32 newArea);

    void Grant(Quest const* quest);
    void Revoke(uint32 questId);

    int32 GetOffLogObjectiveData(uint32 questId, QuestObjective const& objective) const;
    void SetOffLogObjectiveData(uint32 questId, QuestObjective const& objective, int32 value);
    bool HasOffLogProgress(uint32 questId) const;
    void Forget(uint32 questId);

    // writes the off-log progress of a held task into its new slot and drops the off-log entry
    void RestoreToSlot(Quest const* quest, uint16 slot);

private:
    static constexpr std::size_t OffLogCounts = 24;          // MAX_QUEST_COUNTS, checked in the .cpp

    Player& _owner;
    std::unordered_map<uint32, std::array<int32, OffLogCounts>> _offLogProgress;
};

#endif // PlayerTaskQuests_h__
