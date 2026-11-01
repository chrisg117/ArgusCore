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
#include "QuestDef.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
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

enum ByAnyMeansData
{
    QUEST_A_NEW_DIRECTION_ALTRUIS           = 40375,
    QUEST_BY_ANY_MEANS_ALTRUIS              = 41121,
    QUEST_BY_ANY_MEANS_KAYN                 = 39051,

    MAP_ACQUISITION_VIOLET_HOLD             = 1494,

    NPC_KAYN_SUNFURY                        = 94902,
    NPC_ALTRUIS_THE_SUFFERER                = 103156,
    NPC_KIRIN_TOR_PROTECTOR                 = 96308,
    NPC_TALDATH_THE_DESTROYER               = 94900,

    MENU_ALTURAS_GREETING                   = 19303,
    MENU_ALTURAS_THREATENED                 = 19304,

    SPELL_CONVINCE_ALTURAS_FIND_OUT         = 204252,
    SPELL_CONVINCE_ALTURAS_NOTHING_TO_LOSE  = 204327,
    SPELL_LEAVE_THE_HOLD                    = 189688,
    SPELL_FELSTOMP                          = 202583,
    SPELL_FEL_SLAGBLAST                     = 202590,
    SPELL_BREATH_OF_FELFLAME                = 202598,

    // Warden Alturas at the door
    SAY_ALTURAS_REFUSES                     = 0,
    SAY_ALTURAS_FORTUNATE                   = 1,
    SAY_ALTURAS_FAIR_ENOUGH                 = 2,
    SAY_ALTURAS_TERMS                       = 3,
    SAY_ALTURAS_VERY_WELL                   = 4,

    // Warden Alturas inside
    SAY_ALTURAS_EVACUATE                    = 0,
    SAY_ALTURAS_FOLLOW_ME                   = 1,
    SAY_ALTURAS_NEVER_HERE                  = 2,

    SAY_PROTECTOR_BOWS                      = 0,

    SAY_TALDATH_RELEASE_ME                  = 0,
    SAY_TALDATH_RELEASE_ME_PORTRAIT         = 1,
    SAY_TALDATH_AGGRO                       = 2,
    SAY_TALDATH_VERMIN                      = 3,
    SAY_TALDATH_SURAMAR                     = 4,
    SAY_TALDATH_VAREDIS                     = 5,
    SAY_TALDATH_DEATH                       = 6,
    SAY_TALDATH_DEATH_PORTRAIT              = 7,

    // Both leaders
    SAY_LEADER_WE_NEED_HIM                  = 0,
    SAY_LEADER_CLEAR                        = 1,
    SAY_LEADER_THE_HONORS                   = 3,
    SAY_LEADER_TELL_US                      = 4,
    SAY_LEADER_FINISH_HIM                   = 5,
    SAY_LEADER_THE_HUNT                     = 6,

    // Kayn alone: the client has no such lines for Altruis
    SAY_KAYN_AFTER_YOU                      = 2,
    SAY_KAYN_MEET_TALDATH                   = 7,
    SAY_KAYN_YOUR_WRATH                     = 8,
    SAY_KAYN_SPEAK_DEMON                    = 9,
    SAY_KAYN_YOUR_MASTER                    = 10,
    SAY_KAYN_WELL_DONE                      = 11,
    SAY_KAYN_TWINBLADES                     = 12,
    SAY_KAYN_OUR_FIRE                       = 13,

    ACTION_OPEN_THE_CELL                    = 1,
    ACTION_TALDATH_DEAD                     = 2,

    POINT_ALTURAS_AT_THE_CELL               = 1,
    POINT_ALTURAS_LEAVES                    = 2,

    EVENT_LEADER_WE_NEED_HIM                = 1,
    EVENT_ALTURAS_FORTUNATE,
    EVENT_ALTURAS_FAIR_ENOUGH,
    EVENT_ALTURAS_TERMS,
    EVENT_LEADER_CLEAR,
    EVENT_ALTURAS_VERY_WELL,

    EVENT_ALTURAS_EVACUATE,
    EVENT_ALTURAS_FOLLOW_ME,
    EVENT_ALTURAS_NEVER_HERE,
    EVENT_LEADER_MEET_TALDATH,
    EVENT_LEADER_THE_HONORS,
    EVENT_OPEN_THE_CELL,

    EVENT_FELSTOMP,
    EVENT_FEL_SLAGBLAST,
    EVENT_BREATH_OF_FELFLAME,

    EVENT_KAYN_TWINBLADES,
    EVENT_KAYN_OUR_FIRE,
    EVENT_LEADER_THE_HUNT,
    EVENT_LEADER_WALKS_OUT
};

static constexpr Position AlturasAtTheCell = { 1412.0f, -264.0f, 52.43f, 0.0f };
static constexpr Position LeaderAtTheCell = { 1408.0f, -269.0f, 52.43f, 0.0f };
static constexpr Position LeaderAtTheDoor = { 1288.0f, -261.0f, 44.36f, 0.0f };
static constexpr Position HoldEntrance = { 1293.19f, -262.72f, 44.36f, 0.0f };

// The leader the player chose at the Vault. Both stand at the door, and inside only the chosen one is summoned.
static uint32 GetChosenLeader(Player const* player)
{
    return player->GetQuestRewardStatus(QUEST_A_NEW_DIRECTION_ALTRUIS) ? NPC_ALTRUIS_THE_SUFFERER : NPC_KAYN_SUNFURY;
}

static Creature* FindLeaderInTheHold(WorldObject const* seeker)
{
    if (Creature* kayn = seeker->FindNearestCreature(NPC_KAYN_SUNFURY, 250.0f))
        return kayn;
    return seeker->FindNearestCreature(NPC_ALTRUIS_THE_SUFFERER, 250.0f);
}

// 96313 - Warden Alturas, at the door of the Violet Hold
struct npc_warden_alturas_dalaran : public ScriptedAI
{
    npc_warden_alturas_dalaran(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId == MENU_ALTURAS_GREETING && gossipListId == 0)
        {
            CloseGossipMenuFor(player);
            Talk(SAY_ALTURAS_REFUSES, player);
            return true;
        }

        if (menuId != MENU_ALTURAS_THREATENED)
            return false;

        // Which of the two answers it was makes no difference to the warden; each has a credit of its own
        CloseGossipMenuFor(player);
        player->CastSpell(player, gossipListId == 0 ? SPELL_CONVINCE_ALTURAS_FIND_OUT : SPELL_CONVINCE_ALTURAS_NOTHING_TO_LOSE, true);

        _playerGuid = player->GetGUID();
        _leaderGuid.Clear();
        if (Creature* leader = me->FindNearestCreature(GetChosenLeader(player), 30.0f))
            _leaderGuid = leader->GetGUID();

        _events.Reset();
        _events.ScheduleEvent(EVENT_LEADER_WE_NEED_HIM, 1s);
        return true;
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
            Creature* leader = ObjectAccessor::GetCreature(*me, _leaderGuid);

            switch (eventId)
            {
                case EVENT_LEADER_WE_NEED_HIM:
                    if (leader)
                        leader->AI()->Talk(SAY_LEADER_WE_NEED_HIM, player);
                    _events.ScheduleEvent(EVENT_ALTURAS_FORTUNATE, 6s);
                    break;
                case EVENT_ALTURAS_FORTUNATE:
                    Talk(SAY_ALTURAS_FORTUNATE, player);
                    _events.ScheduleEvent(EVENT_ALTURAS_FAIR_ENOUGH, 6s);
                    break;
                case EVENT_ALTURAS_FAIR_ENOUGH:
                    Talk(SAY_ALTURAS_FAIR_ENOUGH, player);
                    _events.ScheduleEvent(EVENT_ALTURAS_TERMS, 4s);
                    break;
                case EVENT_ALTURAS_TERMS:
                    Talk(SAY_ALTURAS_TERMS, player);
                    _events.ScheduleEvent(EVENT_LEADER_CLEAR, 6s);
                    break;
                case EVENT_LEADER_CLEAR:
                    if (leader)
                        leader->AI()->Talk(SAY_LEADER_CLEAR, player);
                    _events.ScheduleEvent(EVENT_ALTURAS_VERY_WELL, 3s);
                    break;
                case EVENT_ALTURAS_VERY_WELL:
                    Talk(SAY_ALTURAS_VERY_WELL, player);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGuid;
    ObjectGuid _leaderGuid;
};

// 99473 - Warden Alturas, inside the hold of By Any Means. The scene runs from him, once for the player who comes in.
struct npc_warden_alturas_violet_hold : public ScriptedAI
{
    npc_warden_alturas_violet_hold(Creature* creature) : ScriptedAI(creature), _started(false) { }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == POINT_ALTURAS_AT_THE_CELL)
            _events.ScheduleEvent(EVENT_ALTURAS_NEVER_HERE, 1s);
        else if (id == POINT_ALTURAS_LEAVES)
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_started)
        {
            Player* player = me->SelectNearestPlayer(40.0f);
            if (!player)
                return;

            _started = true;
            _playerGuid = player->GetGUID();
            if (TempSummon* leader = me->SummonCreature(GetChosenLeader(player), LeaderAtTheDoor))
                _leaderGuid = leader->GetGUID();
            _events.ScheduleEvent(EVENT_ALTURAS_EVACUATE, 2s);
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
            Creature* leader = ObjectAccessor::GetCreature(*me, _leaderGuid);

            switch (eventId)
            {
                case EVENT_ALTURAS_EVACUATE:
                {
                    Talk(SAY_ALTURAS_EVACUATE, player);
                    std::list<Creature*> protectors;
                    me->GetCreatureListWithEntryInGrid(protectors, NPC_KIRIN_TOR_PROTECTOR, 50.0f);
                    if (!protectors.empty())
                        protectors.front()->AI()->Talk(SAY_PROTECTOR_BOWS);
                    for (Creature* protector : protectors)
                    {
                        protector->GetMotionMaster()->MovePoint(0, HoldEntrance);
                        protector->DespawnOrUnsummon(10s);
                    }
                    _events.ScheduleEvent(EVENT_ALTURAS_FOLLOW_ME, 7s);
                    break;
                }
                case EVENT_ALTURAS_FOLLOW_ME:
                    Talk(SAY_ALTURAS_FOLLOW_ME, player);
                    me->GetMotionMaster()->MovePoint(POINT_ALTURAS_AT_THE_CELL, AlturasAtTheCell, false);
                    if (leader)
                    {
                        if (leader->GetEntry() == NPC_KAYN_SUNFURY)
                            leader->AI()->Talk(SAY_KAYN_AFTER_YOU, player);
                        leader->GetMotionMaster()->MovePoint(0, LeaderAtTheCell, false);
                    }
                    break;
                case EVENT_ALTURAS_NEVER_HERE:
                    Talk(SAY_ALTURAS_NEVER_HERE, player);
                    _events.ScheduleEvent(EVENT_LEADER_MEET_TALDATH, 5s);
                    break;
                case EVENT_LEADER_MEET_TALDATH:
                    me->GetMotionMaster()->MovePoint(POINT_ALTURAS_LEAVES, HoldEntrance, false);
                    if (leader && leader->GetEntry() == NPC_KAYN_SUNFURY)
                        leader->AI()->Talk(SAY_KAYN_MEET_TALDATH, player);
                    _events.ScheduleEvent(EVENT_LEADER_THE_HONORS, 7s);
                    break;
                case EVENT_LEADER_THE_HONORS:
                    if (leader)
                        leader->AI()->Talk(SAY_LEADER_THE_HONORS, player);
                    _events.ScheduleEvent(EVENT_OPEN_THE_CELL, 6s);
                    break;
                case EVENT_OPEN_THE_CELL:
                    if (Creature* taldath = me->FindNearestCreature(NPC_TALDATH_THE_DESTROYER, 250.0f))
                        taldath->AI()->DoAction(ACTION_OPEN_THE_CELL);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _playerGuid;
    ObjectGuid _leaderGuid;
    bool _started;
};

// 94900 - Taldath the Destroyer. The objective is his death: the template's kill credit is "Taldath interrogated".
struct npc_taldath_the_destroyer : public ScriptedAI
{
    npc_taldath_the_destroyer(Creature* creature) : ScriptedAI(creature), _questioned(0) { }

    void Reset() override
    {
        _events.Reset();
        _questioned = 0;
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_OPEN_THE_CELL)
            return;

        Player* player = me->SelectNearestPlayer(250.0f);
        Talk(SAY_TALDATH_RELEASE_ME, player);
        if (player)
            Talk(SAY_TALDATH_RELEASE_ME_PORTRAIT, player);
        me->SetFaction(FACTION_MONSTER_2);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        Talk(SAY_TALDATH_AGGRO);
        if (Creature* leader = FindLeaderInTheHold(me))
            if (leader->GetEntry() == NPC_KAYN_SUNFURY)
                if (Player* player = me->SelectNearestPlayer(100.0f))
                    leader->AI()->Talk(SAY_KAYN_YOUR_WRATH, player);

        _events.ScheduleEvent(EVENT_FEL_SLAGBLAST, 6s);
        _events.ScheduleEvent(EVENT_FELSTOMP, 12s);
        _events.ScheduleEvent(EVENT_BREATH_OF_FELFLAME, 18s);
    }

    // The interrogation is the fight: what he lets slip is on the way down
    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        Creature* leader = FindLeaderInTheHold(me);
        Player* player = me->SelectNearestPlayer(100.0f);
        bool kayn = leader && leader->GetEntry() == NPC_KAYN_SUNFURY;

        if (_questioned == 0 && me->HealthBelowPctDamaged(75, damage))
        {
            ++_questioned;
            if (kayn && player)
                leader->AI()->Talk(SAY_KAYN_SPEAK_DEMON, player);
            if (leader)
                leader->AI()->Talk(SAY_LEADER_TELL_US, player);
            Talk(SAY_TALDATH_VERMIN);
        }
        else if (_questioned == 1 && me->HealthBelowPctDamaged(50, damage))
        {
            ++_questioned;
            if (kayn && player)
                leader->AI()->Talk(SAY_KAYN_YOUR_MASTER, player);
            Talk(SAY_TALDATH_SURAMAR);
        }
        else if (_questioned == 2 && me->HealthBelowPctDamaged(25, damage))
        {
            ++_questioned;
            Talk(SAY_TALDATH_VAREDIS);
            if (kayn && player)
                leader->AI()->Talk(SAY_KAYN_WELL_DONE, player);
            if (leader)
                leader->AI()->Talk(SAY_LEADER_FINISH_HIM, player);
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        _events.Reset();
        Player* player = me->SelectNearestPlayer(100.0f);
        Talk(SAY_TALDATH_DEATH);
        if (player)
            Talk(SAY_TALDATH_DEATH_PORTRAIT, player);
        if (Creature* leader = FindLeaderInTheHold(me))
            leader->AI()->DoAction(ACTION_TALDATH_DEAD);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_FEL_SLAGBLAST:
                    DoCastVictim(SPELL_FEL_SLAGBLAST);
                    _events.Repeat(12s);
                    break;
                case EVENT_FELSTOMP:
                    DoCastSelf(SPELL_FELSTOMP);
                    _events.Repeat(18s);
                    break;
                case EVENT_BREATH_OF_FELFLAME:
                    DoCastVictim(SPELL_BREATH_OF_FELFLAME);
                    _events.Repeat(22s);
                    break;
                default:
                    break;
            }

            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
        }
    }

private:
    EventMap _events;
    uint8 _questioned;
};

// 94902 - Kayn Sunfury, 103156 - Altruis the Sufferer: at the door of the Violet Hold and inside it
struct npc_illidari_leader_violet_hold : public ScriptedAI
{
    npc_illidari_leader_violet_hold(Creature* creature) : ScriptedAI(creature) { }

    void DoAction(int32 action) override
    {
        if (action != ACTION_TALDATH_DEAD)
            return;

        _events.Reset();
        _events.ScheduleEvent(me->GetEntry() == NPC_KAYN_SUNFURY ? EVENT_KAYN_TWINBLADES : EVENT_LEADER_THE_HUNT, 4s);
    }

    // The hold has no way out of its own: the turn-in inside it is the way out, next to the door they came through
    void OnQuestReward(Player* player, Quest const* quest, LootItemType /*type*/, uint32 /*opt*/) override
    {
        if (me->GetMapId() == MAP_ACQUISITION_VIOLET_HOLD && (quest->GetQuestId() == QUEST_BY_ANY_MEANS_ALTRUIS || quest->GetQuestId() == QUEST_BY_ANY_MEANS_KAYN))
            player->CastSpell(player, SPELL_LEAVE_THE_HOLD, true);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Player* player = me->SelectNearestPlayer(250.0f);

            switch (eventId)
            {
                case EVENT_KAYN_TWINBLADES:
                    Talk(SAY_KAYN_TWINBLADES, player);
                    _events.ScheduleEvent(EVENT_KAYN_OUR_FIRE, 7s);
                    break;
                case EVENT_KAYN_OUR_FIRE:
                    Talk(SAY_KAYN_OUR_FIRE, player);
                    _events.ScheduleEvent(EVENT_LEADER_THE_HUNT, 8s);
                    break;
                case EVENT_LEADER_THE_HUNT:
                    Talk(SAY_LEADER_THE_HUNT, player);
                    _events.ScheduleEvent(EVENT_LEADER_WALKS_OUT, 7s);
                    break;
                case EVENT_LEADER_WALKS_OUT:
                    me->GetMotionMaster()->MovePoint(0, LeaderAtTheDoor);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
};

// 188163 - Teleport
// The Violet Hold's door for By Any Means: the kill credit of its first effect the core does, the second takes the
// player into a dungeon of the dungeon finder, which it doesn't. The place is the one the spell has in `spell_target_position`.
class spell_dh_by_any_means_teleport : public SpellScript
{
    void HandleTeleport(SpellEffIndex effIndex)
    {
        Player* player = GetHitPlayer();
        if (!player)
            return;

        if (SpellTargetPosition const* destination = sSpellMgr->GetSpellTargetPosition(GetSpellInfo()->Id, effIndex))
            player->TeleportTo(*destination);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_dh_by_any_means_teleport::HandleTeleport, EFFECT_1, SPELL_EFFECT_TELEPORT_TO_LFG_DUNGEON);
    }
};

void AddSC_orderhall_demon_hunter()
{
    // Creature
    RegisterCreatureAI(npc_korvas_bloodthorn_dalaran);
    RegisterCreatureAI(npc_warden_alturas_dalaran);
    RegisterCreatureAI(npc_warden_alturas_violet_hold);
    RegisterCreatureAI(npc_taldath_the_destroyer);
    RegisterCreatureAI(npc_illidari_leader_violet_hold);

    // Playerchoice
    new playerchoice_the_power_to_survive();

    // Spellscripts
    RegisterSpellScript(spell_dh_force_specialization);
    RegisterSpellScript(spell_dh_by_any_means_teleport);
}
