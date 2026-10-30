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
#include "ConversationAI.h"
#include "CreatureAIImpl.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectAccessor.h"
#include "PhasingHandler.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Spell.h"
#include "SpellScript.h"
#include "TemporarySummon.h"
#include "Vehicle.h"

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

// The Fallen Lion, after the Broken Shore: Genn sends the player off from the Skybreaker; in the throne room the court
// mourns Varian, then holds council, and listening to its end delivers Varian's letter
enum TheFallenLionData
{
    QUEST_THE_FALLEN_LION                   = 40517,

    OBJECTIVE_LISTEN_TO_KING_ANDUIN         = 280850,

    NPC_HEAR_LEADERS_MOURNING_CREDIT        = 100454,

    ITEM_LETTER_TO_ANDUIN                   = 132251,

    // 120 s; marks a player whose court scene is playing, so entering again does not start it twice
    SPELL_PLAYER_LISTENING_TO_LEADERS       = 200102,
    SPELL_LISTEN_TO_KING_ANDUIN_CREDIT      = 198244,

    CONVERSATION_LEADERS_MOURNING           = 926,
    // Custom ids: no spell names the retail ones
    CONVERSATION_LEADERS_COUNCIL            = 1519001,
    CONVERSATION_GENN_SEND_OFF              = 1519002,
    CONVERSATION_TESS_AND_CROWLEY_RAGE      = 1519003,
    CONVERSATION_TESS_AND_CROWLEY_THE_KEEP  = 1519004,

    // The optional-looking "Ride a gryphon to Stormwind Keep" has no optional flag, and the letter's objective is
    // sequenced after it: the ride is part of the quest
    NPC_GILNEAN_GRYPHON_RIDE_CREDIT         = 100696,
    NPC_TESS_GREYMANE_FLYING                = 101820,
    NPC_LORNA_CROWLEY_FLYING                = 101822,

    DISPLAY_GILNEAN_ARMORED_GRYPHON         = 34881,

    POINT_GILNEAN_GRYPHON_THE_KEEP          = 1,

    EVENT_GILNEAN_GRYPHON_TAKE_OFF          = 1,
    EVENT_TESS_AND_CROWLEY_RAGE,
    EVENT_TESS_AND_CROWLEY_THE_KEEP
};

// The council begins as Falstad, the last of the mourners, speaks (the retail chat log has both in the same second)
static constexpr Milliseconds LeadersCouncilDelay = 13s;

static bool IsListeningToKingAnduin(Player const* player)
{
    return player->GetQuestStatus(QUEST_THE_FALLEN_LION) == QUEST_STATUS_INCOMPLETE
        && !player->IsQuestObjectiveComplete(QUEST_THE_FALLEN_LION, OBJECTIVE_LISTEN_TO_KING_ANDUIN);
}

static void StartFallenLionCourt(Player* player)
{
    if (!IsListeningToKingAnduin(player) || player->HasAura(SPELL_PLAYER_LISTENING_TO_LEADERS))
        return;

    player->CastSpell(player, SPELL_PLAYER_LISTENING_TO_LEADERS, true);
    Conversation::CreateConversation(CONVERSATION_LEADERS_MOURNING, player, *player, player->GetGUID());
}

// The Call of War: entering the throne room counts (the objective credits Jace Darkweaver); for The Fallen Lion it starts
// the court scene
struct at_the_call_of_war_throne_room : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->GetPositionZ() < ThroneRoomFloorMinZ)
            return;

        if (player->GetQuestStatus(QUEST_THE_CALL_OF_WAR) == QUEST_STATUS_INCOMPLETE)
            player->KilledMonsterCredit(NPC_JACE_DARKWEAVER_THRONE_ROOM);

        StartFallenLionCourt(player);
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

// To Be Prepared, in Stormwind Harbor before the Broken Shore
enum ToBePreparedData
{
    QUEST_TO_BE_PREPARED                    = 42782,

    OBJECTIVE_WARMED_UP_WITH_A_DUEL         = 284739,

    // Ramall Trueoak; every other duelist has him as its KillCredit
    NPC_DUEL_CREDIT                         = 108722,

    SAY_DUELIST_YIELD                       = 0
};

// The duelist yields at this share of its health, after a countdown like a player duel's
static constexpr int32 DuelistYieldHealthPct = 10;
static constexpr Milliseconds DuelistCountdown = 3s;

static bool CanDuelForToBePrepared(Player const* player)
{
    return player->IsAlive() && player->GetQuestStatus(QUEST_TO_BE_PREPARED) == QUEST_STATUS_INCOMPLETE
        && !player->IsQuestObjectiveComplete(QUEST_TO_BE_PREPARED, OBJECTIVE_WARMED_UP_WITH_A_DUEL);
}

// 108722, 108723, 108742-108759, 108765, 108767 - the duelists of To Be Prepared: "Let's duel." starts a duel with that
// player alone. The duelist turns hostile to players only, so the harbor guards keep out of it; at low health it yields,
// the player gets the credit, and it is friendly again. Damage from anyone else does nothing.
struct npc_to_be_prepared_duelist : public ScriptedAI
{
    npc_to_be_prepared_duelist(Creature* creature) : ScriptedAI(creature), _countdown(0s) { }

    void Reset() override
    {
        _opponent.Clear();
        _countdown = 0s;
        me->RestoreFaction();
        me->SetReactState(REACT_DEFENSIVE);
    }

    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (!_opponent.IsEmpty() || me->IsInCombat() || !CanDuelForToBePrepared(player))
            return true;

        _opponent = player->GetGUID();
        _countdown = DuelistCountdown;
        me->SetFacingToObject(player);
        return true;
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (!attacker || attacker->GetCharmerOrOwnerOrSelf()->GetGUID() != _opponent)
        {
            damage = 0;
            return;
        }

        if (me->HealthBelowPctDamaged(DuelistYieldHealthPct, damage))
        {
            damage = 0;
            Yield();
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (_countdown > 0s)
        {
            _countdown -= Milliseconds(diff);
            if (_countdown <= 0s)
            {
                _countdown = 0s;
                Player* player = ObjectAccessor::GetPlayer(*me, _opponent);
                if (!player || !CanDuelForToBePrepared(player) || !me->IsWithinDistInMap(player, 30.0f))
                {
                    Reset();
                    return;
                }

                me->SetFaction(FACTION_ENEMY);
                AttackStart(player);
            }
            return;
        }

        UpdateVictim();
    }

private:
    void Yield()
    {
        if (Player* player = ObjectAccessor::GetPlayer(*me, _opponent))
        {
            if (CanDuelForToBePrepared(player))
                player->KilledMonsterCredit(NPC_DUEL_CREDIT);
            Talk(SAY_DUELIST_YIELD, player);
        }

        me->SetFullHealth();
        EnterEvadeMode(EvadeReason::Other);
    }

    ObjectGuid _opponent;
    Milliseconds _countdown;
};

// 40517 - The Fallen Lion: Genn's send-off when the player takes it on the Skybreaker
class quest_the_fallen_lion : public QuestScript
{
public:
    quest_the_fallen_lion() : QuestScript("quest_the_fallen_lion") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus oldStatus, QuestStatus newStatus) override
    {
        if (oldStatus == QUEST_STATUS_NONE && newStatus == QUEST_STATUS_INCOMPLETE)
            Conversation::CreateConversation(CONVERSATION_GENN_SEND_OFF, player, *player, player->GetGUID());
    }
};

// The ride from the Skyfire to Stormwind Keep: Wowhead's pins of the flying gryphon (101677), 22 yards over what lies
// under each pin and the two after it (measured), from the deck down to the Keep's gate. Tess and Crowley fly 7 yards
// to either side and land where Wowhead pins their gryphons at the Keep (101816, 101819).
static constexpr std::array<Position, 32> GilneanGryphonFlightToTheKeep =
{{
    { -8382.7f, 1354.6f, 138.0f },
    { -8387.3f, 1344.1f, 129.0f },
    { -8387.3f, 1312.9f, 123.0f },
    { -8387.3f, 1274.6f, 117.0f },
    { -8389.7f, 1253.8f, 111.0f },
    { -8396.6f, 1226.0f, 105.0f },
    { -8405.9f, 1208.6f, 99.0f },
    { -8417.5f, 1180.8f, 93.0f },
    { -8426.7f, 1149.5f, 87.0f },
    { -8433.7f, 1118.3f, 81.0f },
    { -8438.3f, 1097.4f, 75.0f },
    { -8449.9f, 1059.2f, 69.0f },
    { -8456.8f, 1045.3f, 63.0f },
    { -8500.9f, 993.2f, 103.1f },
    { -8528.7f, 954.9f, 121.3f },
    { -8542.6f, 934.1f, 121.3f },
    { -8561.1f, 913.2f, 121.3f },
    { -8593.5f, 868.1f, 118.9f },
    { -8607.4f, 850.7f, 119.1f },
    { -8626.0f, 826.4f, 119.1f },
    { -8649.1f, 760.3f, 119.1f },
    { -8658.4f, 746.4f, 118.8f },
    { -8688.5f, 715.2f, 121.9f },
    { -8697.8f, 652.6f, 121.9f },
    { -8700.1f, 617.9f, 121.9f },
    { -8693.1f, 586.6f, 115.4f },
    { -8670.0f, 569.2f, 115.4f },
    { -8619.0f, 537.9f, 115.4f },
    { -8593.5f, 520.6f, 114.6f },
    { -8570.4f, 496.2f, 114.6f },
    { -8552.3f, 475.4f, 110.8f },
    { -8540.2f, 461.5f, 104.8f }
}};
static constexpr std::array<Position, 31> TessFlightToTheKeep =
{{
    { -8376.3f, 1351.8f, 140.0f },
    { -8380.4f, 1343.4f, 131.0f },
    { -8380.3f, 1312.9f, 125.0f },
    { -8380.4f, 1274.4f, 119.0f },
    { -8382.8f, 1252.5f, 113.0f },
    { -8390.0f, 1223.6f, 107.0f },
    { -8399.5f, 1205.7f, 101.0f },
    { -8410.9f, 1178.5f, 95.0f },
    { -8420.0f, 1147.8f, 89.0f },
    { -8426.8f, 1116.8f, 83.0f },
    { -8431.6f, 1095.6f, 77.0f },
    { -8443.3f, 1056.8f, 71.0f },
    { -8451.3f, 1041.0f, 65.0f },
    { -8495.4f, 988.8f, 105.1f },
    { -8522.9f, 950.9f, 123.3f },
    { -8537.0f, 929.8f, 123.3f },
    { -8555.6f, 909.0f, 123.3f },
    { -8587.9f, 863.9f, 120.9f },
    { -8601.9f, 846.4f, 121.1f },
    { -8619.6f, 823.4f, 121.1f },
    { -8642.6f, 757.7f, 121.1f },
    { -8653.1f, 741.8f, 120.8f },
    { -8682.1f, 712.5f, 123.9f },
    { -8690.8f, 651.8f, 123.9f },
    { -8693.1f, 618.4f, 123.9f },
    { -8687.2f, 590.3f, 117.4f },
    { -8666.1f, 575.1f, 117.4f },
    { -8615.3f, 543.9f, 117.4f },
    { -8589.0f, 525.9f, 116.6f },
    { -8565.2f, 501.0f, 116.6f },
    { -8519.4f, 430.2f, 106.0f }
}};
static constexpr std::array<Position, 31> CrowleyFlightToTheKeep =
{{
    { -8389.1f, 1357.4f, 140.0f },
    { -8394.3f, 1344.9f, 131.0f },
    { -8394.3f, 1312.9f, 125.0f },
    { -8394.3f, 1274.9f, 119.0f },
    { -8396.5f, 1255.1f, 113.0f },
    { -8403.2f, 1228.4f, 107.0f },
    { -8412.2f, 1211.6f, 101.0f },
    { -8424.1f, 1183.1f, 95.0f },
    { -8433.5f, 1151.3f, 89.0f },
    { -8440.5f, 1119.8f, 83.0f },
    { -8445.1f, 1099.3f, 77.0f },
    { -8456.5f, 1061.5f, 71.0f },
    { -8462.4f, 1049.6f, 65.0f },
    { -8506.3f, 997.5f, 105.1f },
    { -8534.4f, 959.0f, 123.3f },
    { -8548.1f, 938.4f, 123.3f },
    { -8566.6f, 917.5f, 123.3f },
    { -8599.2f, 872.2f, 120.9f },
    { -8613.0f, 855.0f, 121.1f },
    { -8632.3f, 829.3f, 121.1f },
    { -8655.6f, 763.0f, 121.1f },
    { -8663.7f, 751.0f, 120.8f },
    { -8695.0f, 717.9f, 123.9f },
    { -8704.7f, 653.4f, 123.9f },
    { -8707.1f, 617.4f, 123.9f },
    { -8699.1f, 582.9f, 117.4f },
    { -8673.8f, 563.4f, 117.4f },
    { -8622.8f, 532.0f, 117.4f },
    { -8598.1f, 515.3f, 116.6f },
    { -8575.5f, 491.5f, 116.6f },
    { -8514.8f, 424.0f, 106.0f }
}};

static constexpr float GilneanGryphonSpeed = 28.0f;
static constexpr Milliseconds TessAndCrowleyDespawn = 55s;

template <size_t N>
static void FlyPath(Unit* unit, std::array<Position, N> const& points, uint32 pointId)
{
    unit->GetMotionMaster()->LaunchMoveSpline([&points](Movement::MoveSplineInit& init)
    {
        Movement::PointsArray path;
        for (Position const& point : points)
            path.emplace_back(point.GetPositionX(), point.GetPositionY(), point.GetPositionZ());
        init.MovebyPath(path);
        init.SetFly();
        init.SetVelocity(GilneanGryphonSpeed);
    }, pointId);
}

// 100696 - Gilnean Gryphon (The Fallen Lion): the one on the Skyfire's deck summons the player's own (198209, spell
// click), which takes the player aboard and flies them to Stormwind Keep with Tess and Crowley alongside; landing counts
struct npc_the_fallen_lion_gilnean_gryphon : public ScriptedAI
{
    npc_the_fallen_lion_gilnean_gryphon(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(WorldObject* summoner) override
    {
        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        // The spell brings it to the ground under the player, which on the Skyfire is the harbor below. And it takes the
        // ship's phase, which the player leaves with the harbor: the rider would lose sight of their own gryphon.
        PhasingHandler::SetAlwaysVisible(me, true, true);
        me->NearTeleportTo(player->GetPosition());
        player->EnterVehicle(me, 0);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!apply || !passenger->IsPlayer() || !_riderGuid.IsEmpty())
            return;

        Player* rider = passenger->ToPlayer();
        _riderGuid = rider->GetGUID();
        me->SetDisableGravity(true);

        // Tess and Crowley lead the way
        _tessGuid = SummonEscort(rider, NPC_TESS_GREYMANE_FLYING, TessFlightToTheKeep);
        _crowleyGuid = SummonEscort(rider, NPC_LORNA_CROWLEY_FLYING, CrowleyFlightToTheKeep);

        _events.ScheduleEvent(EVENT_GILNEAN_GRYPHON_TAKE_OFF, 1500ms);
        _events.ScheduleEvent(EVENT_TESS_AND_CROWLEY_RAGE, 3s);
        _events.ScheduleEvent(EVENT_TESS_AND_CROWLEY_THE_KEEP, 33s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE || id != POINT_GILNEAN_GRYPHON_THE_KEEP)
            return;

        if (Player* rider = ObjectAccessor::GetPlayer(*me, _riderGuid))
            rider->KilledMonsterCredit(NPC_GILNEAN_GRYPHON_RIDE_CREDIT);

        if (Vehicle* vehicle = me->GetVehicleKit())
            vehicle->RemoveAllPassengers();
        me->DespawnOrUnsummon(2s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GILNEAN_GRYPHON_TAKE_OFF:
                    FlyPath(me, GilneanGryphonFlightToTheKeep, POINT_GILNEAN_GRYPHON_THE_KEEP);
                    break;
                case EVENT_TESS_AND_CROWLEY_RAGE:
                    StartEscortConversation(CONVERSATION_TESS_AND_CROWLEY_RAGE);
                    break;
                case EVENT_TESS_AND_CROWLEY_THE_KEEP:
                    StartEscortConversation(CONVERSATION_TESS_AND_CROWLEY_THE_KEEP);
                    break;
                default:
                    break;
            }
        }
    }

private:
    template <size_t N>
    static ObjectGuid SummonEscort(Player* rider, uint32 entry, std::array<Position, N> const& points)
    {
        TempSummon* escort = rider->SummonCreature(entry, points.front(), TEMPSUMMON_TIMED_DESPAWN, TessAndCrowleyDespawn, 0, 0, rider->GetGUID());
        if (!escort)
            return ObjectGuid::Empty;

        PhasingHandler::SetAlwaysVisible(escort, true, true);
        escort->Mount(DISPLAY_GILNEAN_ARMORED_GRYPHON);
        escort->SetDisableGravity(true);
        FlyPath(escort, points, 0);
        return escort->GetGUID();
    }

    // Tess is the first actor of both, Crowley the second
    void StartEscortConversation(uint32 conversationId)
    {
        Player* rider = ObjectAccessor::GetPlayer(*me, _riderGuid);
        Creature* tess = ObjectAccessor::GetCreature(*me, _tessGuid);
        Creature* crowley = ObjectAccessor::GetCreature(*me, _crowleyGuid);
        if (!rider || !tess || !crowley)
            return;

        Conversation* conversation = Conversation::CreateConversation(conversationId, rider, *rider, rider->GetGUID(), nullptr, false);
        if (!conversation)
            return;

        conversation->AddActor(0, 0, tess->GetGUID());
        conversation->AddActor(0, 1, crowley->GetGUID());
        // It stays where it began, which the rider leaves behind at the gryphon's speed long before its last line
        conversation->SetVisibilityDistanceOverride(VisibilityDistanceType::Infinite);
        conversation->Start();
    }

    EventMap _events;
    ObjectGuid _riderGuid;
    ObjectGuid _tessGuid;
    ObjectGuid _crowleyGuid;
};

// 926 - Conversation (the court mourns Varian): the council follows, and Varian's letter is handed over as it begins
class conversation_the_fallen_lion_mourning : public ConversationAI
{
public:
    using ConversationAI::ConversationAI;

    void OnStart() override
    {
        conversation->m_Events.AddEvent([conversation = conversation]()
        {
            Player* player = ObjectAccessor::GetPlayer(*conversation, conversation->GetPrivateObjectOwner());
            if (!player || !IsListeningToKingAnduin(player))
                return;

            player->DestroyItemCount(ITEM_LETTER_TO_ANDUIN, 1, true);
            Conversation::CreateConversation(CONVERSATION_LEADERS_COUNCIL, player, *player, player->GetGUID());
        }, LeadersCouncilDelay);
    }
};

// 1519001 - Conversation (the council, custom id): listening to its end delivers the letter
class conversation_the_fallen_lion_council : public ConversationAI
{
public:
    using ConversationAI::ConversationAI;

    void OnStart() override
    {
        conversation->m_Events.AddEvent([conversation = conversation]()
        {
            Player* player = ObjectAccessor::GetPlayer(*conversation, conversation->GetPrivateObjectOwner());
            if (!player)
                return;

            player->RemoveAurasDueToSpell(SPELL_PLAYER_LISTENING_TO_LEADERS);
            if (!IsListeningToKingAnduin(player))
                return;

            player->CastSpell(player, SPELL_LISTEN_TO_KING_ANDUIN_CREDIT, true);
            // "Hear Leaders Mourning" (hidden, optional) only counts once the sequenced letter objective is done
            player->KilledMonsterCredit(NPC_HEAR_LEADERS_MOURNING_CREDIT);
        }, conversation->GetLastLineEndTime(conversation->GetPrivateObjectOwnerLocale()));
    }
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
    RegisterCreatureAI(npc_to_be_prepared_duelist);
    RegisterCreatureAI(npc_the_fallen_lion_gilnean_gryphon);

    // Quests
    new quest_the_fallen_lion();

    // Conversations
    RegisterConversationAI(conversation_the_fallen_lion_mourning);
    RegisterConversationAI(conversation_the_fallen_lion_council);

    // Players
    new player_second_sight();

    // Areatriggers
    RegisterAreaTriggerAI(at_the_call_of_war_throne_room);
}
