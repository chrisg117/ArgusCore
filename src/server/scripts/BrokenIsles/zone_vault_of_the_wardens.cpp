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

#include "CreatureAI.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum BreakingOutData
{
    QUEST_BREAKING_OUT                      = 38672,

    OBJECTIVE_BREAKING_OUT_ALTRUIS_FREED    = 278274,
    OBJECTIVE_BREAKING_OUT_KAYN_FREED       = 278275,

    NPC_KAYN_CELL                           = 103658,
    NPC_KAYN_SUNFURY_FREED                  = 92980,
    NPC_ALTRUIS_THE_SUFFERER_FREED          = 92986,

    NPC_KAYN_FREED_CREDIT                   = 92848,
    NPC_ALTRUIS_FREED_CREDIT                = 92849,
    NPC_KAYN_WEAPONS_CREDIT                 = 112276,
    NPC_ALTRUIS_WEAPONS_CREDIT              = 112277,
    NPC_KAYN_REUNION_CREDIT                 = 99326,
    NPC_ALTRUIS_REUNION_CREDIT              = 112287,

    SAY_KAYN_FREED                          = 0,
    SAY_KAYN_WHY_FREE_HIM                   = 1,
    SAY_KAYN_RELEASE_OUR_ALLIES             = 2,
    SAY_ALTRUIS_FREED                       = 0,
    SAY_ALTRUIS_LONG_TIME                   = 1,
    SAY_ALTRUIS_ANSWERS_LATER               = 2
};

struct BreakingOutReunionLine
{
    uint32 Speaker;
    uint8 TextGroup;
    Milliseconds Delay;
};

// Once both are free they argue about Altruis (retail lines, voice files vo_701_kayn_sunfury / vo_701_altruis_the_sufferer)
static constexpr std::array<BreakingOutReunionLine, 4> BreakingOutReunion =
{ {
    { NPC_KAYN_SUNFURY_FREED,           SAY_KAYN_WHY_FREE_HIM,          4s },
    { NPC_ALTRUIS_THE_SUFFERER_FREED,   SAY_ALTRUIS_LONG_TIME,          9s },
    { NPC_KAYN_SUNFURY_FREED,           SAY_KAYN_RELEASE_OUR_ALLIES,    18s },
    { NPC_ALTRUIS_THE_SUFFERER_FREED,   SAY_ALTRUIS_ANSWERS_LATER,      24s }
} };
static constexpr Milliseconds BreakingOutReunionEnd = 28s;

// 103655 - Altruis's Cell, 103658 - Kayn's Cell
// Clicking a cell frees its prisoner for that player: visibility conditions swap the cell and its prisoner for the freed
// Illidari waiting by the lever, who answer; once both are free they talk, which completes the quest's hidden objectives
struct npc_breaking_out_cell : public ScriptedAI
{
    npc_breaking_out_cell(Creature* creature) : ScriptedAI(creature) { }

    void OnSpellClick(Unit* clicker, bool /*spellClickHandled*/) override
    {
        Player* player = clicker->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_BREAKING_OUT) != QUEST_STATUS_INCOMPLETE)
            return;

        bool kayn = me->GetEntry() == NPC_KAYN_CELL;
        if (player->IsQuestObjectiveComplete(QUEST_BREAKING_OUT, kayn ? OBJECTIVE_BREAKING_OUT_KAYN_FREED : OBJECTIVE_BREAKING_OUT_ALTRUIS_FREED))
            return;

        player->KilledMonsterCredit(kayn ? NPC_KAYN_FREED_CREDIT : NPC_ALTRUIS_FREED_CREDIT);
        player->KilledMonsterCredit(kayn ? NPC_KAYN_WEAPONS_CREDIT : NPC_ALTRUIS_WEAPONS_CREDIT);

        if (Creature* freed = player->FindNearestCreature(kayn ? NPC_KAYN_SUNFURY_FREED : NPC_ALTRUIS_THE_SUFFERER_FREED, 50.0f))
            freed->AI()->Talk(kayn ? SAY_KAYN_FREED : SAY_ALTRUIS_FREED, player);

        if (player->IsQuestObjectiveComplete(QUEST_BREAKING_OUT, OBJECTIVE_BREAKING_OUT_KAYN_FREED)
            && player->IsQuestObjectiveComplete(QUEST_BREAKING_OUT, OBJECTIVE_BREAKING_OUT_ALTRUIS_FREED))
            StartReunion(player);
    }

private:
    static void StartReunion(Player* player)
    {
        // On the player's own event queue, so the talk follows them and stops if they log out
        for (BreakingOutReunionLine const& line : BreakingOutReunion)
        {
            player->m_Events.AddEventAtOffset([player, line]
            {
                if (Creature* speaker = player->FindNearestCreature(line.Speaker, 50.0f))
                    speaker->AI()->Talk(line.TextGroup, player);
            }, line.Delay);
        }

        player->m_Events.AddEventAtOffset([player]
        {
            if (player->GetQuestStatus(QUEST_BREAKING_OUT) != QUEST_STATUS_INCOMPLETE)
                return;

            player->KilledMonsterCredit(NPC_KAYN_REUNION_CREDIT);
            player->KilledMonsterCredit(NPC_ALTRUIS_REUNION_CREDIT);
        }, BreakingOutReunionEnd);
    }
};

void AddSC_zone_vault_of_the_wardens()
{
    // Creature
    RegisterCreatureAI(npc_breaking_out_cell);
}
