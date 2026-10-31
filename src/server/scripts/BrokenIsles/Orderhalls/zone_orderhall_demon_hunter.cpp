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
#include "MotionMaster.h"
#include "Player.h"
#include "PlayerChoice.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"
#include "SpellScript.h"

enum ThePowerToSurviveData
{
    QUEST_THE_POWER_TO_SURVIVE_ALTRUIS      = 40816,
    QUEST_THE_POWER_TO_SURVIVE_KAYN         = 40814,

    PLAYERCHOICE_RESPONSE_HAVOC             = 2551,
    PLAYERCHOICE_RESPONSE_VENGEANCE         = 2552,

    SPELL_THE_POWER_TO_SURVIVE_HAVOC        = 201093,
    SPELL_THE_POWER_TO_SURVIVE_VENGEANCE    = 201094,

    SPELL_FORCE_SPECIALIZATION_HAVOC        = 211226,

    POINT_KORVAS_APPROACH                   = 1
};

static constexpr float KorvasApproachDistance = 3.0f;

// 255 - The Power to Survive: Havoc/Vengeance
class playerchoice_the_power_to_survive : public PlayerChoiceScript
{
public:
    playerchoice_the_power_to_survive() : PlayerChoiceScript("playerchoice_the_power_to_survive") { }

    void OnResponse(WorldObject* /*object*/, Player* player, PlayerChoice const* /*choice*/, PlayerChoiceResponse const* response) override
    {
        // The first pick credits the quest; a late click on the window must not pick the other weapon as well
        if (player->GetQuestStatus(QUEST_THE_POWER_TO_SURVIVE_ALTRUIS) != QUEST_STATUS_INCOMPLETE
            && player->GetQuestStatus(QUEST_THE_POWER_TO_SURVIVE_KAYN) != QUEST_STATUS_INCOMPLETE)
            return;

        if (response->ResponseId == PLAYERCHOICE_RESPONSE_HAVOC)
            player->CastSpell(player, SPELL_THE_POWER_TO_SURVIVE_HAVOC, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        else if (response->ResponseId == PLAYERCHOICE_RESPONSE_VENGEANCE)
            player->CastSpell(player, SPELL_THE_POWER_TO_SURVIVE_VENGEANCE, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }
};

// 211226 - Force Specialization (Havoc)
// 211227 - Force Specialization (Vengeance)
class spell_dh_force_specialization : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        Player* player = GetHitUnit()->ToPlayer();
        if (!player)
            return;

        ChrSpecialization spec = GetSpellInfo()->Id == SPELL_FORCE_SPECIALIZATION_HAVOC ? ChrSpecialization::DemonHunterHavoc : ChrSpecialization::DemonHunterVengeance;
        player->ActivateTalentGroup(sChrSpecializationStore.AssertEntry(AsUnderlyingType(spec)));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_force_specialization::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

// 99343 - Kor'vas Bloodthorn (Dalaran summon)
// The summon spell puts her exactly where the player stands, inside the player's model: she steps out in front to be clickable
struct npc_korvas_bloodthorn_dalaran : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void IsSummonedBy(WorldObject* summoner) override
    {
        Position pos = summoner->GetNearPosition(KorvasApproachDistance, 0.0f);
        me->GetMotionMaster()->MovePoint(POINT_KORVAS_APPROACH, pos, true, pos.GetAbsoluteAngle(summoner));
    }
};

void AddSC_orderhall_demon_hunter()
{
    // Creature
    RegisterCreatureAI(npc_korvas_bloodthorn_dalaran);

    // Playerchoice
    new playerchoice_the_power_to_survive();

    // Spellscripts
    RegisterSpellScript(spell_dh_force_specialization);
}
