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

#include "ScriptMgr.h"
#include "DB2Stores.h"
#include "Log.h"
#include "Player.h"
#include "PlayerChoice.h"
#include "QuestDef.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

enum APactOfNecessityData
{
    QUEST_A_PACT_OF_NECESSITY               = 40715,
    QUEST_FROST_CHOSEN                      = 40723,

    PLAYERCHOICE_RESPONSE_FROST             = 2531,

    SPELL_FROST_CHOSEN                      = 200078,
    SPELL_FROST_DEATH_KNIGHT_QUEST_LAUNCH   = 190092,

    SPELL_FORCE_SPECIALIZATION_BLOOD        = 211222,
    SPELL_FORCE_SPECIALIZATION_FROST        = 211223
};

// 253 - A Pact of Necessity: Frost (Blood and Unholy have no artifact quest yet)
class playerchoice_the_pact_of_necessity : public PlayerChoiceScript
{
public:
    playerchoice_the_pact_of_necessity() : PlayerChoiceScript("playerchoice_the_pact_of_necessity") { }

    void OnResponse(WorldObject* /*object*/, Player* player, PlayerChoice const* /*choice*/, PlayerChoiceResponse const* response) override
    {
        // The first pick credits the quest; a late click on the window must not pick a second weapon
        if (player->GetQuestStatus(QUEST_A_PACT_OF_NECESSITY) != QUEST_STATUS_INCOMPLETE)
            return;

        if (response->ResponseId == PLAYERCHOICE_RESPONSE_FROST)
            player->CastSpell(player, SPELL_FROST_CHOSEN, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }
};

// 211222 - Force Specialization (Blood)
// 211223 - Force Specialization (Frost)
// 211225 - Force Specialization (Unholy)
class spell_dk_force_specialization : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitUnit()->ToPlayer();
        if (!player)
            return;

        ChrSpecialization spec = ChrSpecialization::DeathKnightUnholy;
        if (GetSpellInfo()->Id == SPELL_FORCE_SPECIALIZATION_BLOOD)
            spec = ChrSpecialization::DeathKnightBlood;
        else if (GetSpellInfo()->Id == SPELL_FORCE_SPECIALIZATION_FROST)
            spec = ChrSpecialization::DeathKnightFrost;

        player->ActivateTalentGroup(sChrSpecializationStore.AssertEntry(AsUnderlyingType(spec)));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_force_specialization::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 201261 - 7.0 Artifacts - Death Knight
// A Pact of Necessity's reward: sends the player on the quest of the weapon they chose
class spell_dk_artifacts_choice_reward : public SpellScript
{
    void HandleScript(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        if (player->GetQuestStatus(QUEST_FROST_CHOSEN) == QUEST_STATUS_REWARDED)
            player->CastSpell(player, SPELL_FROST_DEATH_KNIGHT_QUEST_LAUNCH, true);
        else
            TC_LOG_ERROR("scripts", "spell_dk_artifacts_choice_reward: {} chose a weapon whose artifact quest is not built", player->GetGUID().ToString());
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_artifacts_choice_reward::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum TheCallOfIcecrownData
{
    QUEST_THE_CALL_OF_ICECROWN              = 38990,
    QUEST_OBJECTIVE_USE_DEATH_GATE          = 278748,

    NPC_DEATH_GATE                          = 95854,
    NPC_WAYGATE_CREDIT                      = 93970,
    NPC_INTRO_SCENE_CREDIT                  = 96227,

    ZONE_ACHERUS                            = 7679
};

static constexpr float DeathGateDistance = 4.0f;
static constexpr float DeathGateAngle = float(M_PI / 2); // to the player's left, clear of the NPC they faced
static constexpr float DeathGateSearchRange = 50.0f;
static constexpr Seconds DeathGateDuration = 10min;

// The quest's own summon (187409 -> 190089 -> 190088) aims at a target type the core does not implement, so the gate
// is placed here: beside the player, seen by them alone, and only one at a time
static void SummonDeathGate(Player* player)
{
    if (player->GetZoneId() == ZONE_ACHERUS || player->GetQuestObjectiveData(QUEST_THE_CALL_OF_ICECROWN, QUEST_OBJECTIVE_USE_DEATH_GATE) > 0)
        return;

    std::list<Creature*> gates;
    player->GetCreatureListWithEntryInGrid(gates, NPC_DEATH_GATE, DeathGateSearchRange);
    for (Creature* gate : gates)
        if (gate->GetPrivateObjectOwner() == player->GetGUID())
            gate->DespawnOrUnsummon();

    Position pos = player->GetNearPosition(DeathGateDistance, DeathGateAngle);
    pos.SetOrientation(pos.GetAbsoluteAngle(player));
    player->SummonCreature(NPC_DEATH_GATE, pos, TEMPSUMMON_TIMED_DESPAWN, DeathGateDuration, 0, 0, player->GetGUID());
}

// 38990 - The Call of Icecrown
class quest_the_call_of_icecrown : public QuestScript
{
public:
    quest_the_call_of_icecrown() : QuestScript("quest_the_call_of_icecrown") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus /*oldStatus*/, QuestStatus newStatus) override
    {
        if (newStatus == QUEST_STATUS_INCOMPLETE)
            SummonDeathGate(player);
    }
};

// The gate is a summon and does not survive a logout: a player who left before using it gets it back
class player_dk_the_call_of_icecrown : public PlayerScript
{
public:
    player_dk_the_call_of_icecrown() : PlayerScript("player_dk_the_call_of_icecrown") { }

    void OnLogin(Player* player, bool /*firstLogin*/) override
    {
        if (player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) == QUEST_STATUS_INCOMPLETE)
            SummonDeathGate(player);
    }
};

// 187753 - Death Gate (to Acherus)
// The quest's first step: the summoned gate and the player's own Death Gate both cast it. The arrival scene credit
// rides along, since the scene the data names has no package here
class spell_dk_the_call_of_icecrown_death_gate : public SpellScript
{
    void HandleAfterHit() const
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) != QUEST_STATUS_INCOMPLETE)
            return;

        player->KilledMonsterCredit(NPC_WAYGATE_CREDIT);
        player->KilledMonsterCredit(NPC_INTRO_SCENE_CREDIT);
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_dk_the_call_of_icecrown_death_gate::HandleAfterHit);
    }
};

// 200428 - Death Gate (to Icecrown Citadel)
// Its effect is a dungeon teleport the core leaves unhandled; the destination row names the citadel's gate
class spell_dk_death_gate_icecrown : public SpellScript
{
    void HandleTeleport(SpellEffIndex effIndex) const
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        if (SpellTargetPosition const* destination = sSpellMgr->GetSpellTargetPosition(GetSpellInfo()->Id, effIndex))
            player->TeleportTo(*destination);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dk_death_gate_icecrown::HandleTeleport, EFFECT_0, SPELL_EFFECT_TELEPORT_TO_LFG_DUNGEON);
    }
};

void AddSC_orderhall_death_knight()
{
    // Quests
    new quest_the_call_of_icecrown();
    new player_dk_the_call_of_icecrown();

    // Playerchoice
    new playerchoice_the_pact_of_necessity();

    // Spellscripts
    RegisterSpellScript(spell_dk_force_specialization);
    RegisterSpellScript(spell_dk_artifacts_choice_reward);
    RegisterSpellScript(spell_dk_the_call_of_icecrown_death_gate);
    RegisterSpellScript(spell_dk_death_gate_icecrown);
}
