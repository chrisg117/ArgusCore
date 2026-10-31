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
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerChoice.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

enum ThePowerToSurviveData
{
    QUEST_THE_POWER_TO_SURVIVE_ALTRUIS      = 40816,
    QUEST_THE_POWER_TO_SURVIVE_KAYN         = 40814,

    PLAYERCHOICE_RESPONSE_HAVOC             = 2551,
    PLAYERCHOICE_RESPONSE_VENGEANCE         = 2552,

    SPELL_THE_POWER_TO_SURVIVE_HAVOC        = 201093,
    SPELL_THE_POWER_TO_SURVIVE_VENGEANCE    = 201094,

    SPELL_FORCE_SPECIALIZATION_HAVOC        = 211226,

    QUEST_CALL_OF_THE_ILLIDARI_ALTRUIS      = 39047,
    QUEST_CALL_OF_THE_ILLIDARI_KAYN         = 39261,

    ZONE_DALARAN                            = 7502,

    POINT_KORVAS_STEP_OUT                   = 1
};

static constexpr float KorvasFollowDistance = 4.0f;
static constexpr float KorvasFollowAngle = float(M_PI / 6); // ahead and to the left, where the player can see her
static constexpr float KorvasCatchUpDistance = 50.0f;
static constexpr float KorvasCatchUpMaxDrop = 10.0f;
static constexpr float KorvasTooCloseDistance = 1.5f;
static constexpr Milliseconds KorvasCheckInterval = 1s;
static constexpr Milliseconds KorvasLeaveDelay = 10s;

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
// Stays at the player's side, in view, for as long as they are in Dalaran without having taken her quest. The summon
// spell puts her inside the player's model and lasts five minutes; a player who misses her would not get her back
// until they next entered the zone
struct npc_korvas_bloodthorn_dalaran : public ScriptedAI
{
    npc_korvas_bloodthorn_dalaran(Creature* creature) : ScriptedAI(creature), _checkTimer(KorvasCheckInterval), _questTaken(false) { }

    void IsSummonedBy(WorldObject* summoner) override
    {
        _playerGuid = summoner->GetGUID();
        // Her checks must keep running when a flying player leaves her outside the area the map updates around them
        me->setActive(true);
        StepOut(summoner->ToUnit());
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE || id != POINT_KORVAS_STEP_OUT)
            return;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid))
            me->GetMotionMaster()->MoveFollow(player, KorvasFollowDistance, ChaseAngle(KorvasFollowAngle));
    }

    void UpdateAI(uint32 diff) override
    {
        if (_questTaken)
            return;

        _checkTimer -= Milliseconds(diff);
        if (_checkTimer > 0ms)
            return;
        _checkTimer = KorvasCheckInterval;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
        if (!player || player->GetZoneId() != ZONE_DALARAN)
        {
            me->DespawnOrUnsummon();
            return;
        }

        if (player->GetQuestStatus(QUEST_CALL_OF_THE_ILLIDARI_ALTRUIS) != QUEST_STATUS_NONE || player->GetQuestStatus(QUEST_CALL_OF_THE_ILLIDARI_KAYN) != QUEST_STATUS_NONE)
        {
            _questTaken = true;
            me->GetMotionMaster()->Clear();
            me->StopMoving();
            me->SetFacingToObject(player);
            me->DespawnOrUnsummon(KorvasLeaveDelay);
            return;
        }

        // The summon spell's five minutes run out on a player who hasn't noticed her yet
        me->ToTempSummon()->RefreshTimer();

        // Flight and teleports leave her behind. Over open air the near point drops to whatever floor lies below: wait until they land
        if (me->GetExactDist(player) > KorvasCatchUpDistance)
        {
            Position pos = player->GetNearPosition(KorvasFollowDistance, KorvasFollowAngle);
            if (std::abs(pos.GetPositionZ() - player->GetPositionZ()) < KorvasCatchUpMaxDrop)
                me->NearTeleportTo(pos);
        }
        // Following never moves her off a player who stops on top of her, and she can't be clicked inside their model
        else if (me->GetExactDist(player) < KorvasTooCloseDistance && !player->isMoving())
            StepOut(player);
    }

private:
    void StepOut(Unit* target)
    {
        Position pos = target->GetNearPosition(KorvasFollowDistance, KorvasFollowAngle);
        me->GetMotionMaster()->MovePoint(POINT_KORVAS_STEP_OUT, pos, true, pos.GetAbsoluteAngle(target));
    }

    ObjectGuid _playerGuid;
    Milliseconds _checkTimer;
    bool _questTaken;
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
