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
#include "GameObject.h"
#include "GameObjectAI.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "QuestDef.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"

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

enum RiseOfTheIllidariData
{
    QUEST_RISE_OF_THE_ILLIDARI              = 38690,

    SAY_ILLIDARI_PRISONER_FREED             = 0,

    SPELL_FREE_ILLIDARI_PRISONER            = 193119
};

// The named Illidari held in the clickable cells; each is also an optional objective of the quest
static constexpr std::array<uint32, 8> RiseOfTheIllidariPrisoners =
{
    97614, // Allari the Souleater
    97615, // Jace Darkweaver
    97616, // Cyana Nightglaive
    97617, // Asha Ravensong
    97618, // Falara Nightsong
    97619, // Marius Felbane
    97620, // Lyana Darksorrow
    97622  // Kor'vas Bloodthorn
};
static constexpr float WardenCellPrisonerRange = 5.0f;

// 244588 - Warden Cell
// Each cell frees one prisoner per player: it opens for that player only and stays open for them, and the Illidari held
// in it answers
struct go_rise_of_the_illidari_warden_cell : public GameObjectAI
{
    go_rise_of_the_illidari_warden_cell(GameObject* go) : GameObjectAI(go) { }

    bool OnGossipHello(Player* player) override
    {
        if (player->GetQuestStatus(QUEST_RISE_OF_THE_ILLIDARI) != QUEST_STATUS_INCOMPLETE || !_freedFor.insert(player->GetGUID()).second)
            return true;

        me->SetGoStateFor(GO_STATE_ACTIVE, player);
        // Its effects give the freed-prisoner credit
        player->CastSpell(player, SPELL_FREE_ILLIDARI_PRISONER, true);

        for (uint32 entry : RiseOfTheIllidariPrisoners)
        {
            if (Creature* prisoner = me->FindNearestCreature(entry, WardenCellPrisonerRange))
            {
                prisoner->AI()->Talk(SAY_ILLIDARI_PRISONER_FREED, player);
                player->KilledMonsterCredit(entry);
                break;
            }
        }
        return true;
    }

private:
    GuidUnorderedSet _freedFor;
};

enum FelInfusionData
{
    QUEST_FEL_INFUSION                      = 38689,

    MAP_VAULT_OF_THE_WARDENS_QUESTS         = 1468,

    NPC_FEL_INFUSION_SOUL_FRAGMENT          = 89297,

    SPELL_FEL_INFUSION_BAR                  = 133508
};

// Each demon killed in the Vault while on Fel Infusion fills this much of the 100-point bar
static constexpr int32 FelInfusionEnergyPerDemon = 10;

// 38689 - Fel Infusion: the fel energy bar shows while the quest is in the log (its reward spell 133510 removes it)
class quest_fel_infusion : public QuestScript
{
public:
    quest_fel_infusion() : QuestScript("quest_fel_infusion") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus oldStatus, QuestStatus newStatus) override
    {
        if (oldStatus == QUEST_STATUS_NONE && newStatus == QUEST_STATUS_INCOMPLETE)
            player->CastSpell(player, SPELL_FEL_INFUSION_BAR, true);
        else if (newStatus == QUEST_STATUS_NONE)
            player->RemoveAurasDueToSpell(SPELL_FEL_INFUSION_BAR);
    }
};

// Demons killed in the Vault give back fel energy ("Kill them and gather their souls")
class player_fel_infusion : public PlayerScript
{
public:
    player_fel_infusion() : PlayerScript("player_fel_infusion") { }

    void OnCreatureKill(Player* killer, Creature* killed) override
    {
        if (killer->GetQuestStatus(QUEST_FEL_INFUSION) != QUEST_STATUS_INCOMPLETE || killed->GetMapId() != MAP_VAULT_OF_THE_WARDENS_QUESTS
            || killed->GetCreatureType() != CREATURE_TYPE_DEMON)
            return;

        killer->ModifyPower(POWER_ALTERNATE_POWER, FelInfusionEnergyPerDemon);
        killer->UpdateQuestObjectiveProgress(QUEST_OBJECTIVE_MONSTER, NPC_FEL_INFUSION_SOUL_FRAGMENT, FelInfusionEnergyPerDemon, killed->GetGUID());
    }
};

enum VaultOfTheWardensBonusObjectiveData
{
    QUEST_VAULT_OF_THE_WARDENS              = 39742,

    ZONE_VAULT_OF_THE_WARDENS               = 7814
};

// 39742 - Vault of the Wardens: the Vault's bonus objective. The core gives task quests to no one, so it is added on
// entering the Vault
class player_vault_of_the_wardens_bonus_objective : public PlayerScript
{
public:
    player_vault_of_the_wardens_bonus_objective() : PlayerScript("player_vault_of_the_wardens_bonus_objective") { }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        if (newZone != ZONE_VAULT_OF_THE_WARDENS || player->GetMapId() != MAP_VAULT_OF_THE_WARDENS_QUESTS
            || player->GetQuestStatus(QUEST_VAULT_OF_THE_WARDENS) != QUEST_STATUS_NONE)
            return;

        if (Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_VAULT_OF_THE_WARDENS))
            if (player->CanTakeQuest(quest, false))
                player->AddQuestAndCheckCompletion(quest, nullptr);
    }
};

// 39742 - Vault of the Wardens: as on retail, rewarded as soon as its bar is full
class quest_vault_of_the_wardens_bonus_objective : public QuestScript
{
public:
    quest_vault_of_the_wardens_bonus_objective() : QuestScript("quest_vault_of_the_wardens_bonus_objective") { }

    void OnQuestStatusChange(Player* player, Quest const* quest, QuestStatus /*oldStatus*/, QuestStatus newStatus) override
    {
        if (newStatus != QUEST_STATUS_COMPLETE)
            return;

        // Once the kill that filled the bar is done updating the quest log; the player's own events end with the player
        player->m_Events.AddEventAtOffset([player, quest]()
        {
            if (player->GetQuestStatus(quest->GetQuestId()) == QUEST_STATUS_COMPLETE)
                player->RewardQuest(quest, LootItemType::Item, 0, player);
        }, 1ms);
    }
};

// 202064 - Destroying Legion Portal (the Legion Portals' spellclick): the channel destroys the portal, a kill for the
// player who closed it
class spell_vault_of_the_wardens_destroying_legion_portal : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Creature* portal = GetHitCreature();
        if (!portal)
            return;

        // Kill rewards go to the creature's tappers only, and nothing has struck the portal
        portal->SetTappedBy(GetCaster());
        Unit::Kill(GetCaster(), portal);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_vault_of_the_wardens_destroying_legion_portal::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_zone_vault_of_the_wardens()
{
    // Creature
    RegisterCreatureAI(npc_breaking_out_cell);

    // GameObject
    RegisterGameObjectAI(go_rise_of_the_illidari_warden_cell);

    // Quest
    new quest_fel_infusion();
    new quest_vault_of_the_wardens_bonus_objective();

    // Player
    new player_fel_infusion();
    new player_vault_of_the_wardens_bonus_objective();

    // Spell
    RegisterSpellScript(spell_vault_of_the_wardens_destroying_legion_portal);
}
