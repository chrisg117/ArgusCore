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
#include "CreatureAI.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "ObjectAccessor.h"
#include "QuestDef.h"
#include "SceneMgr.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"
#include "Vehicle.h"

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

enum StopGuldanData
{
    QUEST_STOP_GULDAN_HAVOC                 = 38723,
    QUEST_STOP_GULDAN_VENGEANCE             = 40253,

    NPC_CRUSHER                             = 97632,
    NPC_SLEDGE                              = 92990,
    NPC_KAYN_SUNFURY_VAULT                  = 96665,
    NPC_ALTRUIS_THE_SUFFERER_BETRAYER       = 92985,

    SCENE_STOP_GULDAN                       = 14680001,

    SPELL_STOP_GULDAN_QUEST_ACCEPT          = 223661,
    SPELL_BRUTAL_ATTACKS                    = 199556,
    SPELL_TAKING_POWER_VENGEFUL_RETREAT     = 210453,
    SPELL_TAKING_POWER_FIERY_BRAND          = 210467,

    ITEM_MASSIVE_SWORD                      = 128520,

    SAY_BRUTE_EQUIPS_MASSIVE_SWORD          = 0,
    SAY_BRUTE_ATTACK_THE_OTHER_BRUTE        = 1,

    ACTION_BRUTE_TAKE_SWORD                 = 1,
    ACTION_BRUTE_BANTER_STARTED,

    EVENT_BRUTE_PASS_SWORD                  = 1,
    EVENT_BRUTE_BANTER
};

// Where the scene leaves the player's clone: the stairs up to Illidan's platform in the Vault of the Betrayer
static constexpr Position VaultOfTheBetrayerStairs = { 4085.05f, -297.99f, -282.44f, 3.2004f };
// Where the scene's brutes walk to, out of the side portals
static constexpr Position SledgeFightPosition = { 4048.28f, -307.11f, -281.59f, 0.4536f };
static constexpr Position CrusherFightPosition = { 4046.49f, -286.22f, -281.59f, 5.9626f };
// Kayn and Altruis (spawns with this StringId) fight them
static constexpr std::string_view VaultOfTheBetrayerSquad = "VaultOfTheBetrayerSquad";

// 38723 - Stop Gul'dan! (Havoc), 40253 - Stop Gul'dan! (Vengeance): accepting it plays Gul'dan taking Illidan's body, a
// client scene nothing in the data starts. Its accept spell gives Face Gul'dan.
class quest_stop_guldan : public QuestScript
{
public:
    quest_stop_guldan() : QuestScript("quest_stop_guldan") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus oldStatus, QuestStatus newStatus) override
    {
        if (oldStatus != QUEST_STATUS_NONE || newStatus != QUEST_STATUS_INCOMPLETE)
            return;

        player->CastSpell(player, SPELL_STOP_GULDAN_QUEST_ACCEPT, true);
        player->GetSceneMgr().PlayScene(SCENE_STOP_GULDAN);
    }
};

// Scene package 1423, "Illidan's Chamber - Illidan Breakout": it ends with the Illidari charging into the Vault of the
// Betrayer, where the player is put. The brutes Gul'dan leaves behind come through the portals for that player only
// once (they do not respawn), and stay ten minutes, their corpses long enough to take their power.
class scene_stop_guldan : public SceneScript
{
public:
    scene_stop_guldan() : SceneScript("scene_stop_guldan") { }

    void OnSceneComplete(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        EnterTheVaultOfTheBetrayer(player);
    }

    void OnSceneCancel(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        EnterTheVaultOfTheBetrayer(player);
    }

private:
    static void EnterTheVaultOfTheBetrayer(Player* player)
    {
        player->NearTeleportTo(VaultOfTheBetrayerStairs);

        for (auto [entry, position] : { std::pair(NPC_SLEDGE, &SledgeFightPosition), std::pair(NPC_CRUSHER, &CrusherFightPosition) })
            if (TempSummon* brute = player->SummonCreature(entry, *position, TEMPSUMMON_TIMED_DESPAWN, 10min))
                brute->SetScriptStringId(std::string(VaultOfTheBetrayerSquad));
    }
};

// Altruis and Kayn argue while they fight beside the player: speaker, text group, then the wait before the next line
struct BetrayerBanterLine
{
    uint32 Speaker;
    uint8 TextGroup;
    Milliseconds Next;
};

static constexpr std::array<BetrayerBanterLine, 7> BetrayerBanter =
{{
    { NPC_ALTRUIS_THE_SUFFERER_BETRAYER, 0, 4s },   // Just like old times, eh Kayn?
    { NPC_KAYN_SUNFURY_VAULT,            0, 7s },   // Do not speak to me of old times, Altruis...
    { NPC_ALTRUIS_THE_SUFFERER_BETRAYER, 1, 9s },   // After ten-thousand years of imprisonment...
    { NPC_KAYN_SUNFURY_VAULT,            1, 7s },   // He was fighting it, as we all do...
    { NPC_ALTRUIS_THE_SUFFERER_BETRAYER, 2, 6s },   // You followed Illidan blindly, like a dog...
    { NPC_KAYN_SUNFURY_VAULT,            2, 7s },   // He made the hard choices...
    { NPC_ALTRUIS_THE_SUFFERER_BETRAYER, 3, 0s }    // I fought as hard as the rest of you!...
}};

// 97632 - Crusher, 92990 - Sledge: they fight together and pass the Massive Sword between them; its holder has Brutal
// Attacks. Kayn and Altruis hold them off (damage from creatures stops at 30% health). Once both are dead, the players
// who killed the last one take their power.
struct npc_stop_guldan_brute : public ScriptedAI
{
    npc_stop_guldan_brute(Creature* creature) : ScriptedAI(creature), _banterStep(0), _banterStarted(false) { }

    void Reset() override
    {
        _events.Reset();
        _warned.clear();
        _banterStep = 0;
        _banterStarted = false;
        DropSword();
    }

    void JustEngagedWith(Unit* who) override
    {
        if (Creature* partner = GetPartner())
            if (partner->IsAlive() && !partner->IsInCombat())
                partner->AI()->AttackStart(who);

        // Sledge holds the sword first, as in the scene
        if (me->GetEntry() == NPC_SLEDGE)
            TakeSword();
    }

    void DoAction(int32 action) override
    {
        switch (action)
        {
            case ACTION_BRUTE_TAKE_SWORD:
                TakeSword();
                break;
            case ACTION_BRUTE_BANTER_STARTED:
                _banterStarted = true;
                break;
            default:
                break;
        }
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        if (!attacker)
            return;

        if (Player* player = attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
        {
            StartBanter();
            if (me->HasAura(SPELL_BRUTAL_ATTACKS) && _warned.insert(player->GetGUID()).second)
                Talk(SAY_BRUTE_ATTACK_THE_OTHER_BRUTE, player);
            return;
        }

        if (!me->HealthBelowPctDamaged(30, damage))
            return;

        uint64 floorHealth = me->CountPctFromMaxHealth(30);
        damage = me->GetHealth() > floorHealth ? uint32(me->GetHealth() - floorHealth) : 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        DropSword();

        if (Creature* partner = GetPartner())
        {
            if (partner->IsAlive())
            {
                partner->AI()->DoAction(ACTION_BRUTE_TAKE_SWORD);
                return;
            }
        }

        for (ObjectGuid const& tapperGuid : me->GetTapList())
        {
            Player* player = ObjectAccessor::GetPlayer(*me, tapperGuid);
            if (!player)
                continue;

            if (player->GetQuestStatus(QUEST_STOP_GULDAN_HAVOC) == QUEST_STATUS_INCOMPLETE)
                player->CastSpell(me, SPELL_TAKING_POWER_VENGEFUL_RETREAT, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
            else if (player->GetQuestStatus(QUEST_STOP_GULDAN_VENGEANCE) == QUEST_STATUS_INCOMPLETE)
                player->CastSpell(me, SPELL_TAKING_POWER_FIERY_BRAND, TRIGGERED_IGNORE_CAST_IN_PROGRESS);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_BRUTE_PASS_SWORD:
                    if (Creature* partner = GetPartner())
                    {
                        if (partner->IsAlive() && partner->IsInCombat())
                        {
                            DropSword();
                            partner->AI()->DoAction(ACTION_BRUTE_TAKE_SWORD);
                            break;
                        }
                    }
                    _events.ScheduleEvent(EVENT_BRUTE_PASS_SWORD, 20s);
                    break;
                case EVENT_BRUTE_BANTER:
                {
                    BetrayerBanterLine const& line = BetrayerBanter[_banterStep];
                    if (Creature* speaker = me->FindNearestCreature(line.Speaker, 60.0f))
                        speaker->AI()->Talk(line.TextGroup);
                    if (++_banterStep < BetrayerBanter.size())
                        _events.ScheduleEvent(EVENT_BRUTE_BANTER, line.Next);
                    break;
                }
                default:
                    break;
            }
        }

        UpdateVictim();
    }

private:
    Creature* GetPartner() const
    {
        return me->FindNearestCreature(me->GetEntry() == NPC_SLEDGE ? NPC_CRUSHER : NPC_SLEDGE, 50.0f, false);
    }

    void TakeSword()
    {
        if (me->HasAura(SPELL_BRUTAL_ATTACKS))
            return;

        DoCastSelf(SPELL_BRUTAL_ATTACKS, true);
        me->SetVirtualItem(0, ITEM_MASSIVE_SWORD);
        Talk(SAY_BRUTE_EQUIPS_MASSIVE_SWORD);
        _warned.clear();
        _events.ScheduleEvent(EVENT_BRUTE_PASS_SWORD, 20s);
    }

    void DropSword()
    {
        me->RemoveAurasDueToSpell(SPELL_BRUTAL_ATTACKS);
        me->SetVirtualItem(0, 0);
        _events.CancelEvent(EVENT_BRUTE_PASS_SWORD);
    }

    // Once per pair of brutes, when a player first strikes one of them
    void StartBanter()
    {
        if (_banterStarted)
            return;

        _banterStarted = true;
        if (Creature* partner = GetPartner())
            partner->AI()->DoAction(ACTION_BRUTE_BANTER_STARTED);
        _banterStep = 0;
        _events.ScheduleEvent(EVENT_BRUTE_BANTER, 1s);
    }

    EventMap _events;
    GuidUnorderedSet _warned;
    uint8 _banterStep;
    bool _banterStarted;
};

enum GrandTheftFelbatData
{
    QUEST_GRAND_THEFT_FELBAT                = 39682,

    NPC_VAMPIRIC_FELBAT_CREDIT              = 96659,
    NPC_KAYN_SUNFURY_RUNNING_AHEAD          = 97265,
    NPC_ALTRUIS_THE_SUFFERER_RUNNING_AHEAD  = 97267,

    CONVERSATION_BASTILLAX_EVERY_FLOOR      = 14680001,

    SAY_KAYN_YOU_HEARD_THE_WARDEN           = 0,
    SAY_KAYN_LEADER_WITH_NO_FOLLOWERS       = 1,
    SAY_ALTRUIS_YOU_WOULD_TRUST_THE_WARDEN  = 0,

    POINT_FELBAT_DEMON_WARD                 = 1,
    POINT_RUNNING_AHEAD_TOMB                = 1,

    EVENT_FELBAT_TAKE_OFF                   = 1,
    EVENT_FELBAT_BASTILLAX
};

// Where Kayn and Altruis wait in the Tomb of the Penitent (the scene's places)
static constexpr Position KaynInTheTomb = { 4238.04f, -290.14f, -281.39f, 3.3685f };
static constexpr Position AltruisInTheTomb = { 4238.93f, -295.14f, -281.39f, 3.2987f };

// 39682 - Grand Theft Felbat: Kayn and Altruis run ahead to the Tomb of the Penitent, arguing about the Warden. The two
// in the Vault of the Betrayer are gone for the player once it is taken; these are the player's own.
class quest_grand_theft_felbat : public QuestScript
{
public:
    quest_grand_theft_felbat() : QuestScript("quest_grand_theft_felbat") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus oldStatus, QuestStatus newStatus) override
    {
        if (oldStatus != QUEST_STATUS_NONE || newStatus != QUEST_STATUS_INCOMPLETE)
            return;

        Creature* kayn = player->FindNearestCreature(NPC_KAYN_SUNFURY_VAULT, 30.0f);
        Creature* altruis = player->FindNearestCreature(NPC_ALTRUIS_THE_SUFFERER_BETRAYER, 30.0f);
        if (!kayn || !altruis)
            return;

        TempSummon* kaynAhead = player->SummonCreature(NPC_KAYN_SUNFURY_RUNNING_AHEAD, kayn->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 2min, 0, 0, player->GetGUID());
        TempSummon* altruisAhead = player->SummonCreature(NPC_ALTRUIS_THE_SUFFERER_RUNNING_AHEAD, altruis->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 2min, 0, 0, player->GetGUID());
        if (!kaynAhead || !altruisAhead)
            return;

        kaynAhead->GetMotionMaster()->MovePoint(POINT_RUNNING_AHEAD_TOMB, KaynInTheTomb, true, KaynInTheTomb.GetOrientation());
        altruisAhead->GetMotionMaster()->MovePoint(POINT_RUNNING_AHEAD_TOMB, AltruisInTheTomb, true, AltruisInTheTomb.GetOrientation());

        kaynAhead->AI()->Talk(SAY_KAYN_YOU_HEARD_THE_WARDEN, player);
        ObjectGuid kaynGuid = kaynAhead->GetGUID();
        ObjectGuid altruisGuid = altruisAhead->GetGUID();
        player->m_Events.AddEventAtOffset([player, altruisGuid]()
        {
            if (Creature* altruis = ObjectAccessor::GetCreature(*player, altruisGuid))
                altruis->AI()->Talk(SAY_ALTRUIS_YOU_WOULD_TRUST_THE_WARDEN, player);
        }, 6s);
        player->m_Events.AddEventAtOffset([player, kaynGuid]()
        {
            if (Creature* kayn = ObjectAccessor::GetCreature(*player, kaynGuid))
                kayn->AI()->Talk(SAY_KAYN_LEADER_WITH_NO_FOLLOWERS, player);
        }, 12s);
    }
};

// 97265 - Kayn Sunfury, 97267 - Altruis the Sufferer (Grand Theft Felbat): gone once they reach the Tomb, where the
// player finds the Tomb's own Kayn and Altruis
struct npc_grand_theft_felbat_running_ahead : public ScriptedAI
{
    npc_grand_theft_felbat_running_ahead(Creature* creature) : ScriptedAI(creature) { }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_RUNNING_AHEAD_TOMB)
            me->DespawnOrUnsummon();
    }
};

// The felbat's way up: off the landing by the elevator, over the elevator resting at the bottom of the shaft and up its
// middle to the round hall the shaft rises through, then along the walkway from there into the Demon Ward (z 126) and
// down beside Kor'vas. The shaft's bottom legs were checked for line of sight with the agent character; the walkway is
// one straight stretch of the navmesh.
static constexpr std::array<Position, 11> FelbatFlightToTheDemonWard =
{{
    { 4436.0f, -284.1f, -243.0f },
    { 4444.0f, -303.0f, -236.0f },
    { 4451.0f, -321.5f, -230.0f },
    { 4451.0f, -321.5f, -200.0f },
    { 4451.0f, -321.5f,    0.0f },
    { 4451.0f, -321.5f,  120.0f },
    { 4451.0f, -321.5f,  129.5f },
    { 4451.0f, -342.0f,  129.0f },
    { 4451.0f, -365.0f,  128.5f },
    { 4451.3f, -380.0f,  127.2f },
    { 4451.5f, -383.0f,  126.6f }
}};

// 99443 - Vampiric Felbat (195537, from the felbat by the elevator shaft): flies its rider up to the Demon Ward, where the
// ride counts for Grand Theft Felbat. Bastillax taunts on the way up.
struct npc_grand_theft_felbat_vampiric_felbat : public ScriptedAI
{
    npc_grand_theft_felbat_vampiric_felbat(Creature* creature) : ScriptedAI(creature) { }

    void InitializeAI() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetDisableGravity(true);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (!apply || !passenger->IsPlayer())
            return;

        _riderGuid = passenger->GetGUID();
        _events.ScheduleEvent(EVENT_FELBAT_TAKE_OFF, 1s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != EFFECT_MOTION_TYPE || id != POINT_FELBAT_DEMON_WARD)
            return;

        if (Player* rider = ObjectAccessor::GetPlayer(*me, _riderGuid))
            rider->KilledMonsterCredit(NPC_VAMPIRIC_FELBAT_CREDIT);

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
                case EVENT_FELBAT_TAKE_OFF:
                    me->GetMotionMaster()->LaunchMoveSpline([](Movement::MoveSplineInit& init)
                    {
                        Movement::PointsArray path;
                        for (Position const& point : FelbatFlightToTheDemonWard)
                            path.emplace_back(point.GetPositionX(), point.GetPositionY(), point.GetPositionZ());
                        init.MovebyPath(path);
                        init.SetFly();
                        init.SetVelocity(28.0f);
                    }, POINT_FELBAT_DEMON_WARD);
                    _events.ScheduleEvent(EVENT_FELBAT_BASTILLAX, 6s);
                    break;
                case EVENT_FELBAT_BASTILLAX:
                    if (Player* rider = ObjectAccessor::GetPlayer(*me, _riderGuid))
                        Conversation::CreateConversation(CONVERSATION_BASTILLAX_EVERY_FLOOR, rider, *rider, rider->GetGUID());
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    ObjectGuid _riderGuid;
};

enum FrozenInTimeData
{
    QUEST_FROZEN_IN_TIME                    = 39685,

    NPC_KORVAS_BLOODTHORN_DEMON_WARD        = 97643,
    NPC_KAYN_SUNFURY_VAULT_OF_ICE           = 99228,

    GO_VAULT_OF_ICE_ICICLES                 = 246418,

    SAY_KORVAS_MOST_POWERFUL_PRISONERS      = 1,
    SAY_KORVAS_HELP_THEM_QUICKLY            = 2,
    SAY_KAYN_TOO_STRONG                     = 0,
    SAY_KAYN_HOLD_HIM                       = 1,
    SAY_KAYN_MONSTER_CONTAINED              = 2,

    SPELL_LAVA_WREATH                       = 192631,
    SPELL_LAVA_WREATH_PATCH                 = 192571,
    SPELL_LAVA                              = 192519,

    EVENT_ASHGOLM_FIND_OPPONENT             = 1,
    EVENT_ASHGOLM_KAYN_CALLS_OUT,
    EVENT_ASHGOLM_LAVA_WREATH
};

// The Vault of Ice's squad fight: Ash'golm against Kayn and the Freed Illidari
static constexpr std::string_view VaultOfIceSquad = "VaultOfIceSquad";
static constexpr float VaultOfIceRange = 40.0f;
// Each Lava Wreath drops this many patches of lava within this distance of Ash'golm, on his platform
static constexpr uint8 LavaWreathPatches = 3;
static constexpr float LavaWreathRadius = 18.0f;
// The patches last this long; the spell's own ten minutes would cover the platform
static constexpr Milliseconds LavaPatchDuration = 30s;
// Lava burns this share of the player's health every tick: the dungeon's flat amount is for level 110
static constexpr int32 LavaDamagePct = 4;

// 39685 - Frozen in Time: Kor'vas sends the player down to the Vault of Ice
class quest_frozen_in_time : public QuestScript
{
public:
    quest_frozen_in_time() : QuestScript("quest_frozen_in_time") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus oldStatus, QuestStatus newStatus) override
    {
        if (oldStatus != QUEST_STATUS_NONE || newStatus != QUEST_STATUS_INCOMPLETE)
            return;

        Creature* korvas = player->FindNearestCreature(NPC_KORVAS_BLOODTHORN_DEMON_WARD, 30.0f);
        if (!korvas)
            return;

        korvas->AI()->Talk(SAY_KORVAS_MOST_POWERFUL_PRISONERS, player);
        ObjectGuid korvasGuid = korvas->GetGUID();
        player->m_Events.AddEventAtOffset([player, korvasGuid]()
        {
            if (Creature* korvas = ObjectAccessor::GetCreature(*player, korvasGuid))
                korvas->AI()->Talk(SAY_KORVAS_HELP_THEM_QUICKLY, player);
        }, 8s);
    }
};

// 96681 - Ash'golm (Frozen in Time): holds the middle of the Vault of Ice against Kayn and the Freed Illidari, where
// he is frozen later, and cannot be brought down ("He is too strong!"). While players are near, his Lava Wreath drops
// patches of lava around him for them to dodge. Kayn calls out to the players on the quest who come in.
struct npc_frozen_in_time_ashgolm : public ScriptedAI
{
    npc_frozen_in_time_ashgolm(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_DEFENSIVE);
        // Chasing would take him off his platform, and moving breaks his Lava Wreath
        me->SetControlled(true, UNIT_STATE_ROOT);
        _events.Reset();
        _events.ScheduleEvent(EVENT_ASHGOLM_FIND_OPPONENT, 2s);
        _events.ScheduleEvent(EVENT_ASHGOLM_KAYN_CALLS_OUT, 25s, 35s);
        _events.ScheduleEvent(EVENT_ASHGOLM_LAVA_WREATH, 8s);
    }

    void OnSpellCast(SpellInfo const* spellInfo) override
    {
        if (spellInfo->Id != SPELL_LAVA_WREATH)
            return;

        for (uint8 i = 0; i < LavaWreathPatches; ++i)
            me->CastSpell(me->GetRandomNearPosition(LavaWreathRadius), SPELL_LAVA_WREATH_PATCH, true);
    }

    void MoveInLineOfSight(Unit* who) override
    {
        ScriptedAI::MoveInLineOfSight(who);

        Player* player = who->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_FROZEN_IN_TIME) != QUEST_STATUS_INCOMPLETE || !me->IsWithinDist(player, VaultOfIceRange)
            || !_greeted.insert(player->GetGUID()).second)
            return;

        if (Creature* kayn = me->FindNearestCreature(NPC_KAYN_SUNFURY_VAULT_OF_ICE, VaultOfIceRange))
            kayn->AI()->Talk(SAY_KAYN_TOO_STRONG, player);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        // Players cannot bring him down either: only the countermeasures stop him
        if (!me->HealthBelowPctDamaged(30, damage))
            return;

        uint64 floorHealth = me->CountPctFromMaxHealth(30);
        damage = me->GetHealth() > floorHealth ? uint32(me->GetHealth() - floorHealth) : 0;
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ASHGOLM_FIND_OPPONENT:
                    if (!me->IsEngaged())
                        if (Creature* opponent = FindOpponent())
                            AttackStart(opponent);
                    _events.Repeat(3s);
                    break;
                case EVENT_ASHGOLM_KAYN_CALLS_OUT:
                    if (Player* player = me->SelectNearestPlayer(VaultOfIceRange))
                        if (player->GetQuestStatus(QUEST_FROZEN_IN_TIME) == QUEST_STATUS_INCOMPLETE)
                            if (Creature* kayn = me->FindNearestCreature(NPC_KAYN_SUNFURY_VAULT_OF_ICE, VaultOfIceRange))
                                kayn->AI()->Talk(SAY_KAYN_HOLD_HIM, player);
                    _events.Repeat(25s, 35s);
                    break;
                case EVENT_ASHGOLM_LAVA_WREATH:
                    if (me->IsEngaged() && me->SelectNearestPlayer(VaultOfIceRange))
                        DoCastSelf(SPELL_LAVA_WREATH);
                    _events.Repeat(12s, 16s);
                    break;
                default:
                    break;
            }
        }

        UpdateVictim();
    }

private:
    GuidUnorderedSet _greeted;
    Creature* FindOpponent() const
    {
        std::list<Creature*> fighters;
        me->GetCreatureListWithOptionsInGrid(fighters, 15.0f, { .StringId = VaultOfIceSquad, .IsAlive = FindCreatureAliveState::EffectivelyAlive });
        fighters.remove_if([this](Creature const* fighter) { return !me->IsValidAttackTarget(fighter); });
        if (fighters.empty())
            return nullptr;

        fighters.sort(Trinity::ObjectDistanceOrderPred(me));
        return fighters.front();
    }

    EventMap _events;
};

// The platform's icicles close over Ash'golm, for this player only
static void FreezeAshgolmFor(WorldObject const* searcher, Player const* player)
{
    if (GameObject* icicles = searcher->FindNearestGameObject(GO_VAULT_OF_ICE_ICICLES, 80.0f))
        if (icicles->GetGoStateFor(player->GetGUID()) != GO_STATE_ACTIVE)
            icicles->SetGoStateFor(GO_STATE_ACTIVE, player);
}

// 101591 - Ash'golm (frozen): who has frozen him sees him in the icicles whenever they come back
struct npc_frozen_in_time_ashgolm_frozen : public ScriptedAI
{
    npc_frozen_in_time_ashgolm_frozen(Creature* creature) : ScriptedAI(creature) { }

    void MoveInLineOfSight(Unit* who) override
    {
        Player* player = who->ToPlayer();
        if (!player)
            return;

        QuestStatus status = player->GetQuestStatus(QUEST_FROZEN_IN_TIME);
        if (status == QUEST_STATUS_COMPLETE || status == QUEST_STATUS_REWARDED)
            FreezeAshgolmFor(me, player);
    }
};

// 4919 - Lava Wreath's patch of lava: walking into it burns the player (Lava stacks)
struct at_frozen_in_time_lava : AreaTriggerAI
{
    at_frozen_in_time_lava(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnCreate(Spell const* /*creatingSpell*/) override
    {
        at->SetDuration(int32(LavaPatchDuration.count()));
    }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        Unit* caster = at->GetCaster();
        if (!player || !caster)
            return;

        caster->CastSpell(player, SPELL_LAVA, CastSpellExtraArgs(TRIGGERED_FULL_MASK)
            .AddSpellMod(SPELLVALUE_BASE_POINT0, int32(player->CountPctFromMaxHealth(LavaDamagePct))));
    }
};

// 204588 - Activate Countermeasure (the spellclick of 99709 - Eastern, 99731 - Southern, 99732 - Western
// Countermeasures): once its cast is done the countermeasure is on, its own objective's credit. With all three on, the
// icicles close and the quest's visibility conditions show Ash'golm frozen to the player, and Kayn leaves.
class spell_frozen_in_time_activate_countermeasure : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster()->ToPlayer();
        Creature* countermeasure = GetHitCreature();
        if (!player || !countermeasure || player->GetQuestStatus(QUEST_FROZEN_IN_TIME) != QUEST_STATUS_INCOMPLETE)
            return;

        player->KilledMonsterCredit(countermeasure->GetEntry());
        if (player->GetQuestStatus(QUEST_FROZEN_IN_TIME) != QUEST_STATUS_COMPLETE)
            return;

        FreezeAshgolmFor(countermeasure, player);
        if (Creature* kayn = countermeasure->FindNearestCreature(NPC_KAYN_SUNFURY_VAULT_OF_ICE, 80.0f))
            kayn->AI()->Talk(SAY_KAYN_MONSTER_CONTAINED, player);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_frozen_in_time_activate_countermeasure::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

enum BeamMeUpData
{
    QUEST_BEAM_ME_UP                        = 39684,

    NPC_GLAZER_SHIELD_BROKEN_CREDIT         = 96680,
    NPC_GLAZER_STUNNED                      = 99865,
    NPC_ALTRUIS_THE_SUFFERER_VAULT_OF_MIRRORS = 99229,

    GO_REFLECTIVE_MIRROR                    = 244449,

    SAY_ALTRUIS_BEAM_FOCUSED_UPON_ME        = 0,
    SAY_ALTRUIS_SHIELD_BROKEN               = 1,

    SPELL_EYE_BEAM                          = 191916,
    SPELL_EYE_BEAM_REFLECTED                = 191934,
    SPELL_X_SPELL_VISUAL_EYE_BEAM           = 88080,
    SPELL_X_SPELL_VISUAL_EYE_BEAM_REFLECTED = 88110,

    EVENT_GLAZER_EYE_BEAM                   = 1,
    EVENT_GLAZER_MIRROR_TURNS_BACK,

    ACTION_GLAZER_BEAM_REDIRECTED           = 1
};

static constexpr float VaultOfMirrorsRange = 40.0f;
// The Reflective Mirror at rest faces the second lens; once turned it sends the beam back at Glazer for this long
static constexpr float ReflectiveMirrorRestingOrientation = 5.7551f;
static constexpr Milliseconds ReflectiveMirrorTurnedTime = 30s;
// The mirrors (98082 - Lens, spawns with this StringId) that bounce Glazer's beam on its way to Altruis, nearest first
static constexpr std::string_view VaultOfMirrorsLens = "VaultOfMirrorsLens";

// Eye Beam is a channel nothing lets its caster aim at a unit (its nearby-entry target never lands), so the channels
// are set up by hand: the client draws the beam from the channel's visual to its object
static void ChannelEyeBeam(Unit* from, Unit const* to, uint32 spellId, uint32 spellXSpellVisualId)
{
    DynamicFieldStructuredView<ObjectGuid> channelObjects = from->GetChannelObjects();
    if (from->GetChannelSpellId() == spellId && channelObjects.size() == 1 && *channelObjects.begin() == to->GetGUID())
        return;

    from->ClearChannelObjects();
    from->AddChannelObject(to->GetGUID());
    from->SetChannelSpellId(spellId);
    from->SetChannelSpellXSpellVisualId({ .SpellXSpellVisualID = spellXSpellVisualId });
}

// A gameobject's facing only reaches a client when the object is created for them, so the turned mirror is sent again
static void TurnReflectiveMirror(GameObject* mirror, float orientation)
{
    mirror->Relocate(mirror->GetPositionX(), mirror->GetPositionY(), mirror->GetPositionZ(), orientation);
    mirror->RelocateStationaryPosition(mirror->GetPositionX(), mirror->GetPositionY(), mirror->GetPositionZ(), orientation);
    mirror->SetLocalRotationAngles(orientation, 0.0f, 0.0f);
    mirror->DestroyForNearbyPlayers();
    mirror->UpdateObjectVisibility();
}

// 96680 - Glazer (Beam Me Up): his shield (Focusing) holds off Altruis and the Freed Illidari while his Eye Beam,
// bounced off the room's mirrors, burns Altruis. The quest's data does the rest: the Reflective Mirror's own spell
// (191917, cast by the player) gives the credit and sends event 46447, whose SmartAI turns the beam on Glazer
// (action 1) and has Altruis call everyone out; a server-side area trigger has Altruis call the arriving player.
struct npc_beam_me_up_glazer : public ScriptedAI
{
    npc_beam_me_up_glazer(Creature* creature) : ScriptedAI(creature), _beamRedirected(false) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        _events.Reset();
        _events.ScheduleEvent(EVENT_GLAZER_EYE_BEAM, 2s);
        _beamRedirected = false;
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_GLAZER_BEAM_REDIRECTED)
            return;

        if (!_beamRedirected)
            if (GameObject* mirror = me->FindNearestGameObject(GO_REFLECTIVE_MIRROR, VaultOfMirrorsRange))
                TurnReflectiveMirror(mirror, mirror->GetAbsoluteAngle(me));

        _beamRedirected = true;
        _events.RescheduleEvent(EVENT_GLAZER_EYE_BEAM, 1ms);
        _events.RescheduleEvent(EVENT_GLAZER_MIRROR_TURNS_BACK, ReflectiveMirrorTurnedTime);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo = nullptr*/) override
    {
        // Only the reflected beam breaks his shield
        damage = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_GLAZER_EYE_BEAM:
                    if (Creature* altruis = me->FindNearestCreature(NPC_ALTRUIS_THE_SUFFERER_VAULT_OF_MIRRORS, VaultOfMirrorsRange))
                    {
                        std::list<Creature*> lenses;
                        me->GetCreatureListWithOptionsInGrid(lenses, VaultOfMirrorsRange, { .StringId = VaultOfMirrorsLens });
                        lenses.sort(Trinity::ObjectDistanceOrderPred(me));

                        Unit* from = me;
                        for (Creature* lens : lenses)
                        {
                            ChannelEyeBeam(from, lens, from == me ? SPELL_EYE_BEAM : SPELL_EYE_BEAM_REFLECTED,
                                from == me ? SPELL_X_SPELL_VISUAL_EYE_BEAM : SPELL_X_SPELL_VISUAL_EYE_BEAM_REFLECTED);
                            from = lens;
                        }
                        // Sent back at Glazer, the beam ends on him (who sees him stunned sees that one, in the same place)
                        Unit* end = altruis;
                        if (_beamRedirected)
                            if (Creature* stunned = me->FindNearestCreature(NPC_GLAZER_STUNNED, 5.0f))
                                end = stunned;
                        ChannelEyeBeam(from, end, from == me ? SPELL_EYE_BEAM : SPELL_EYE_BEAM_REFLECTED,
                            from == me ? SPELL_X_SPELL_VISUAL_EYE_BEAM : SPELL_X_SPELL_VISUAL_EYE_BEAM_REFLECTED);
                        if (!lenses.empty())
                            me->SetFacingToObject(lenses.front());
                    }
                    _events.Repeat(5s);
                    break;
                case EVENT_GLAZER_MIRROR_TURNS_BACK:
                    _beamRedirected = false;
                    if (GameObject* mirror = me->FindNearestGameObject(GO_REFLECTIVE_MIRROR, VaultOfMirrorsRange))
                        TurnReflectiveMirror(mirror, ReflectiveMirrorRestingOrientation);
                    _events.RescheduleEvent(EVENT_GLAZER_EYE_BEAM, 1ms);
                    break;
                default:
                    break;
            }
        }
    }

private:
    EventMap _events;
    bool _beamRedirected;
};

void AddSC_zone_vault_of_the_wardens()
{
    // Creature
    RegisterCreatureAI(npc_breaking_out_cell);
    RegisterCreatureAI(npc_stop_guldan_brute);
    RegisterCreatureAI(npc_grand_theft_felbat_vampiric_felbat);
    RegisterCreatureAI(npc_grand_theft_felbat_running_ahead);
    RegisterCreatureAI(npc_frozen_in_time_ashgolm);
    RegisterCreatureAI(npc_frozen_in_time_ashgolm_frozen);
    RegisterCreatureAI(npc_beam_me_up_glazer);

    // GameObject
    RegisterGameObjectAI(go_rise_of_the_illidari_warden_cell);

    // Quest
    new quest_fel_infusion();
    new quest_vault_of_the_wardens_bonus_objective();
    new quest_stop_guldan();
    new quest_grand_theft_felbat();
    new quest_frozen_in_time();

    // Player
    new player_fel_infusion();
    new player_vault_of_the_wardens_bonus_objective();

    // Spell
    RegisterSpellScript(spell_vault_of_the_wardens_destroying_legion_portal);
    RegisterSpellScript(spell_frozen_in_time_activate_countermeasure);

    // AreaTrigger
    RegisterAreaTriggerAI(at_frozen_in_time_lava);

    // Scene
    new scene_stop_guldan();
}
