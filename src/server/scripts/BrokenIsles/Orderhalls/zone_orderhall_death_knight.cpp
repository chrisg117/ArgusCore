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
#include "SpellInfo.h"
#include "SpellScript.h"

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

void AddSC_orderhall_death_knight()
{
    // Playerchoice
    new playerchoice_the_pact_of_necessity();

    // Spellscripts
    RegisterSpellScript(spell_dk_force_specialization);
    RegisterSpellScript(spell_dk_artifacts_choice_reward);
}
