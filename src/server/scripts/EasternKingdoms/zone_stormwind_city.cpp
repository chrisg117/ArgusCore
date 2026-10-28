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

#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Conversation.h"
#include "CreatureAIImpl.h"
#include "MotionMaster.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Spell.h"
#include "SpellScript.h"
#include "TemporarySummon.h"

namespace Scripts::EasternKingdoms::StormwindCity
{
namespace Spells
{
    // The King's Command
    static constexpr uint32 MOPAllianceIntroMoviePlay = 130805;
    static constexpr uint32 FadeToBlack               = 130411;

    // The Mission
    static constexpr uint32 TeleportPrep              = 130832;
    static constexpr uint32 TeleportTimer             = 132032;
    static constexpr uint32 TheMissionTeleportPlayer  = 130321;
}

// 130804 - The King's Command Movie Aura
class spell_the_kings_command_movie_aura : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            Spells::FadeToBlack
        });
    }

    void HandleHitTarget(SpellEffIndex /*effIndex*/) const
    {
        GetHitUnit()->CastSpell(nullptr, Spells::FadeToBlack, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_the_kings_command_movie_aura::HandleHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 130804 - The King's Command Movie Aura
class spell_the_kings_command_movie_aura_aura : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            Spells::MOPAllianceIntroMoviePlay
        });
    }

    void HandleAfterEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        GetTarget()->CastSpell(nullptr, Spells::MOPAllianceIntroMoviePlay, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR
        });
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_the_kings_command_movie_aura_aura::HandleAfterEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// 140885 - Admiral Rogers Script Effect
class spell_admiral_rogers_script_effect : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            Spells::TeleportPrep
        });
    }

    void HandleHitTarget(SpellEffIndex /*effIndex*/) const
    {
        GetHitUnit()->CastSpell(nullptr, Spells::TeleportPrep, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_admiral_rogers_script_effect::HandleHitTarget, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// 130832 - Teleport Prep
class spell_teleport_prep_alliance : public SpellScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            Spells::TeleportTimer
        });
    }

    void HandleHitTarget(SpellEffIndex /*effIndex*/) const
    {
        Unit* hitUnit = GetHitUnit();

        hitUnit->CancelMountAura();
        hitUnit->CastSpell(nullptr, Spells::TeleportTimer, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR,
            .TriggeringSpell = GetSpell()
        });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_teleport_prep_alliance::HandleHitTarget, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
    }
};

// 132032 - Teleport Timer
class spell_teleport_timer_alliance : public AuraScript
{
    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({
            Spells::TheMissionTeleportPlayer
        });
    }

    void HandleAfterEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        Unit* target = GetTarget();

        target->CancelTravelShapeshiftForm();
        target->CastSpell(nullptr, Spells::TheMissionTeleportPlayer, CastSpellExtraArgsInit{
            .TriggerFlags = TRIGGERED_IGNORE_CAST_IN_PROGRESS | TRIGGERED_DONT_REPORT_CAST_ERROR
        });
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_teleport_timer_alliance::HandleAfterEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// The Alliance demon hunter's arrival in Stormwind, after Illidari, We Are Leaving
enum DemonHunterArrivalData
{
    QUEST_THE_CALL_OF_WAR                   = 39691,
    QUEST_SECOND_SIGHT                      = 44471,
    QUEST_DEMONS_AMONG_THEM                 = 44463,

    OBJECTIVE_WARN_ANDUIN_WRYNN             = 286650,

    NPC_JACE_DARKWEAVER_THRONE_ROOM         = 102585,
    NPC_USED_SPECTRAL_SIGHT_CREDIT          = 102563,
    NPC_FELBLADE_ASSASSIN                   = 100993,

    SPELL_SPECTRAL_SIGHT                    = 188501,
    SPELL_SPEAK_WITH_JACE_CREDIT            = 199046,

    AREA_STORMWIND_KEEP                     = 6292,

    SAY_FELBLADE_ASSASSIN_ATTACK            = 0
};

static constexpr uint32 GossipActionDemonsAmongYourRanks = GOSSIP_ACTION_INFO_DEF + 1;
// The throne room's floor; the areatrigger's sphere also reaches the hall below it
static constexpr float ThroneRoomFloorMinZ = 150.0f;
// How close the warning player must come before a Felblade Assassin turns on them, and how often it looks
static constexpr float FelbladeAssassinAggroRange = 15.0f;
static constexpr Milliseconds FelbladeAssassinLookInterval = 1s;

static bool HasWarnedAnduin(Player const* player)
{
    return player->GetQuestStatus(QUEST_DEMONS_AMONG_THEM) == QUEST_STATUS_INCOMPLETE
        && player->IsQuestObjectiveComplete(QUEST_DEMONS_AMONG_THEM, OBJECTIVE_WARN_ANDUIN_WRYNN);
}

// The Call of War: entering the throne room counts (the objective credits Jace Darkweaver)
struct at_the_call_of_war_throne_room : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        if (Player* player = unit->ToPlayer())
            if (player->GetPositionZ() >= ThroneRoomFloorMinZ && player->GetQuestStatus(QUEST_THE_CALL_OF_WAR) == QUEST_STATUS_INCOMPLETE)
                player->KilledMonsterCredit(NPC_JACE_DARKWEAVER_THRONE_ROOM);
    }
};

// Second Sight: Spectral Sight used in Stormwind Keep counts
class player_second_sight : public PlayerScript
{
public:
    player_second_sight() : PlayerScript("player_second_sight") { }

    void OnSpellCast(Player* player, Spell* spell, bool /*skipCheck*/) override
    {
        if (spell->GetSpellInfo()->Id == SPELL_SPECTRAL_SIGHT && player->GetAreaId() == AREA_STORMWIND_KEEP
            && player->GetQuestStatus(QUEST_SECOND_SIGHT) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_USED_SPECTRAL_SIGHT_CREDIT);
    }
};

// 100973 - Anduin Wrynn (Demons Among Them): warning him (the retail option text) counts and shows the player the
// Felblade Assassins spread through the Keep; the one in the throne room turns on them
struct npc_demons_among_them_anduin : public ScriptedAI
{
    npc_demons_among_them_anduin(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipHello(Player* player) override
    {
        if (me->IsQuestGiver())
            player->PrepareQuestMenu(me->GetGUID());

        if (player->GetQuestStatus(QUEST_DEMONS_AMONG_THEM) == QUEST_STATUS_INCOMPLETE
            && !player->IsQuestObjectiveComplete(QUEST_DEMONS_AMONG_THEM, OBJECTIVE_WARN_ANDUIN_WRYNN))
            AddGossipItemFor(player, GossipOptionNpc::None, "This cannot wait. There are demons among your ranks. Let me show you.",
                GOSSIP_SENDER_MAIN, GossipActionDemonsAmongYourRanks);

        SendGossipMenuFor(player, player->GetGossipTextId(me), me->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 gossipListId) override
    {
        uint32 action = player->PlayerTalkClass->GetGossipOptionAction(gossipListId);
        CloseGossipMenuFor(player);
        if (action != GossipActionDemonsAmongYourRanks || player->GetQuestStatus(QUEST_DEMONS_AMONG_THEM) != QUEST_STATUS_INCOMPLETE)
            return true;

        player->CastSpell(player, SPELL_SPEAK_WITH_JACE_CREDIT, true);

        if (Creature* assassin = me->FindNearestCreature(NPC_FELBLADE_ASSASSIN, 40.0f))
        {
            assassin->AI()->Talk(SAY_FELBLADE_ASSASSIN_ATTACK, player);
            assassin->AI()->AttackStart(player);
        }
        return true;
    }
};

// 100993 - Felblade Assassin (Demons Among Them): waits in the Keep among the guards (who cannot fight it), seen only
// by those who warned Anduin; it turns on such a player who comes close, and leaves everyone else alone
struct npc_demons_among_them_felblade_assassin : public ScriptedAI
{
    npc_demons_among_them_felblade_assassin(Creature* creature) : ScriptedAI(creature), _lookTimer(FelbladeAssassinLookInterval) { }

    void Reset() override
    {
        me->SetReactState(REACT_DEFENSIVE);
    }

    void UpdateAI(uint32 diff) override
    {
        if (UpdateVictim())
            return;

        _lookTimer -= Milliseconds(diff);
        if (_lookTimer > 0s)
            return;
        _lookTimer = FelbladeAssassinLookInterval;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, FelbladeAssassinAggroRange);
        for (Player* player : players)
        {
            if (HasWarnedAnduin(player) && me->IsWithinLOSInMap(player) && me->IsValidAttackTarget(player))
            {
                AttackStart(player);
                return;
            }
        }
    }

private:
    Milliseconds _lookTimer;
};
}

void AddSC_stormwind_city()
{
    using namespace Scripts::EasternKingdoms::StormwindCity;

    // Spells
    RegisterSpellAndAuraScriptPair(spell_the_kings_command_movie_aura, spell_the_kings_command_movie_aura_aura);
    RegisterSpellScript(spell_admiral_rogers_script_effect);
    RegisterSpellScript(spell_teleport_prep_alliance);
    RegisterSpellScript(spell_teleport_timer_alliance);

    // Creatures
    RegisterCreatureAI(npc_demons_among_them_anduin);
    RegisterCreatureAI(npc_demons_among_them_felblade_assassin);

    // Players
    new player_second_sight();

    // Areatriggers
    RegisterAreaTriggerAI(at_the_call_of_war_throne_room);
}
