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

// Map 1480, the death knight's Icecrown Citadel: scenario 901 "The Blades of the Fallen Prince" (Frost artifact,
// quest 38990 The Call of Icecrown). The map shares ICC 631's coordinates; the client's WorldSafeLocs name every
// place the scenario moves the player to.

#include "ScriptMgr.h"
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Conversation.h"
#include "Creature.h"
#include "DB2Stores.h"
#include "GameEventSender.h"
#include "GameObject.h"
#include "GameObjectAI.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "ScriptedCreature.h"
#include "SharedDefines.h"
#include "SpellMgr.h"
#include "SpellScript.h"

#define DataHeader "IcecrownCitadelScenario"

enum IcecrownCitadelScenarioData
{
    MAP_ICECROWN_CITADEL_SCENARIO               = 1480,

    QUEST_THE_CALL_OF_ICECROWN                  = 38990,
    QUEST_OBJECTIVE_INTRO_SCENE                 = 279705,

    SCENARIO_STEP_THE_CALL_OF_THE_NORTH         = 1809,
    SCENARIO_STEP_THE_GATES_ARE_OPEN            = 1973,
    SCENARIO_STEP_SEEK_THE_FRAGMENTS            = 1810,

    GAME_EVENT_OPEN_THE_GATE_TO_ICECROWN        = 47614,
    GAME_EVENT_ENTER_ICECROWN_CITADEL           = 45677,

    SPELL_DEATH_GATE_TO_ACHERUS                 = 187753,
    SPELL_ENTER_ICECROWN_CREDIT                 = 185861,
    SPELL_FROST_DK_ENTRANCE                     = 196966, // conversation 744, Mograine's welcome
    SPELL_ICECROWN_MOGRAINE_PING                = 188022,

    NPC_HIGHLORD_DARION_MOGRAINE                = 95193,

    // The Lich King's "Enter, $n, there is much to be done." (line 1773) as a talking head; no sniffed conversation
    CONVERSATION_THE_LICH_KING_ENTER            = 14800001,
    CONVERSATION_SEEK_THE_FRAGMENTS             = 14800002,
    CONVERSATION_BAELGUN                        = 14800003,
    CONVERSATION_HALAHK                         = 14800004,
    CONVERSATION_MAGROTH                        = 14800005,
    CONVERSATION_GAVINRAD                       = 14800006,

    NPC_BAELGUN_FLAMEBEARD                      = 93948,
    NPC_HALAHK_THE_DEATHBRINGER                 = 93949,
    NPC_MAGROTH_THE_DEFENDER                    = 93950,
    NPC_GAVINRAD_THE_CRUEL                      = 93951,

    // Four chests, each its own piece of the blade
    GO_FRAGMENT_OF_FROSTMOURNE_1                = 251882,
    GO_FRAGMENT_OF_FROSTMOURNE_2                = 251884,
    GO_FRAGMENT_OF_FROSTMOURNE_3                = 251885,
    GO_FRAGMENT_OF_FROSTMOURNE_4                = 251886,

    // The souls' abilities as the user's retail recording names them; no sniff, so the ids are the client's spells of that
    // name that fit what the recording shows (a 6 s whirlwind, a self heal, a stun)
    SPELL_WHIRLWIND                             = 55266,  // Baelgun
    SPELL_FLASH_OF_LIGHT                        = 180571, // Halahk, 30 % self heal
    SPELL_AVENGING_WRATH                        = 185413, // Halahk, "angel wings"
    SPELL_HAMMER_OF_JUSTICE                     = 66940,  // Halahk, "lock you in place"
    SPELL_AVENGERS_SHIELD                       = 187219, // Magroth
    SPELL_HAMMER_OF_WRATH                       = 37259,  // Gavinrad

    // The gargoyle gate before Baelgun: two Gatekeepers on their pressure plates and the portcullis behind them
    NPC_STONESKIN_GATEKEEPER                    = 97320,
    GO_SPIRE_PORTCULLIS                         = 251449,
    SPELL_HARDENED                              = 192188,
    BROADCAST_TEXT_GARGOYLES_LIFT_OFF           = 99548,

    WORLD_SAFE_LOC_ACHERUS_AREA                 = 5095,
    WORLD_SAFE_LOC_DEATHBRINGERS_RISE           = 5075,
    WORLD_SAFE_LOC_THE_SPIRE_AREA               = 5214,

    DATA_ENTERED_THE_CITADEL                    = 1,
    DATA_GATEKEEPER_LIFTED                      = 2,

    POINT_MOGRAINE_RAMP_FOOT                    = 1,
    POINT_MOGRAINE_RAMP_MIDDLE                  = 2,
    POINT_MOGRAINE_RAMP_TOP                     = 3
};

static constexpr Milliseconds SendBackDelay = 2s;
static constexpr Milliseconds MograineWait = 2s;
static constexpr Milliseconds SpireLineDelay = 3s;
static constexpr float SoulGreetRange = 40.0f;
static constexpr float FragmentSearchRange = 15.0f;
static constexpr float GargoyleGateRange = 60.0f;
static constexpr float PressurePlateRadius = 6.0f;
static constexpr Milliseconds PressurePlateCheck = 250ms;

// Mograine's walk from the landing up the ramp to Acherus's exit, on the 1480 mesh; he stops beside the path at the top
static constexpr Position MograineRampFoot   = { -704.00f, 2205.87f, 535.96f };
static constexpr Position MograineRampMiddle = { -692.53f, 2206.67f, 541.56f };
static constexpr Position MograineRampTop    = { -683.50f, 2210.50f, 545.11f };

static bool IsOnStep(Map const* map, uint32 stepId)
{
    InstanceMap const* instance = map->ToInstanceMap();
    InstanceScenario const* scenario = instance ? instance->GetInstanceScenario() : nullptr;
    return scenario && scenario->GetStep() && scenario->GetStep()->ID == stepId;
}

// WorldSafeLocs keep their facing in degrees
static void TeleportToSafeLoc(Player* player, uint32 worldSafeLocId)
{
    if (WorldSafeLocsEntry const* loc = sWorldSafeLocsStore.LookupEntry(worldSafeLocId))
        player->NearTeleportTo(loc->Loc.X, loc->Loc.Y, loc->Loc.Z, loc->Facing * float(M_PI) / 180.0f);
}

// Only a death knight on The Call of Icecrown belongs here (a login inside after the quest, or a GM's `go`)
static bool BelongsInTheScenario(Player const* player)
{
    return player->GetClass() == CLASS_DEATH_KNIGHT && player->GetQuestStatus(QUEST_THE_CALL_OF_ICECROWN) == QUEST_STATUS_INCOMPLETE;
}

class instance_icecrown_citadel_scenario : public InstanceMapScript
{
public:
    instance_icecrown_citadel_scenario() : InstanceMapScript("instance_icecrown_citadel_scenario", MAP_ICECROWN_CITADEL_SCENARIO) { }

    struct instance_icecrown_citadel_scenario_InstanceMapScript : public InstanceScript
    {
        instance_icecrown_citadel_scenario_InstanceMapScript(InstanceMap* map) : InstanceScript(map)
        {
            SetHeaders(DataHeader);

            // Where a player who releases after dying comes back: Acherus beside the citadel, then the Spire
            SetEntranceLocation(WORLD_SAFE_LOC_ACHERUS_AREA);
        }

        void OnPlayerEnter(Player* player) override
        {
            if (player->IsGameMaster() || BelongsInTheScenario(player))
                return;

            // A teleport during the enter would strand the player on the loading screen: send them back once they stand here
            player->m_Events.AddEventAtOffset([player]
            {
                if (player->GetMapId() != MAP_ICECROWN_CITADEL_SCENARIO || player->IsBeingTeleported() || BelongsInTheScenario(player))
                    return;

                if (SpellTargetPosition const* acherus = sSpellMgr->GetSpellTargetPosition(SPELL_DEATH_GATE_TO_ACHERUS, EFFECT_0))
                    player->TeleportTo(*acherus);
            }, SendBackDelay);
        }

        void SetData(uint32 type, uint32 /*data*/) override
        {
            if (type == DATA_ENTERED_THE_CITADEL)
                SetEntranceLocation(WORLD_SAFE_LOC_THE_SPIRE_AREA);
        }
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_icecrown_citadel_scenario_InstanceMapScript(map);
    }
};

// 95193 - Highlord Darion Mograine, at the landing beside the citadel. Pinged when the player arrives (188022), he
// greets them at once (744 Frost DK - Entrance) and speaks as he walks up the ramp to Acherus's exit, then stops beside
// it and turns back to the player. Once per scenario: he stays at the top afterwards
struct npc_icc_scenario_darion_mograine : public ScriptedAI
{
    using ScriptedAI::ScriptedAI;

    void SetGUID(ObjectGuid const& guid, int32 /*id*/) override
    {
        if (!_playerGuid.IsEmpty())
            return;

        _playerGuid = guid;
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid))
            player->CastSpell(player, SPELL_FROST_DK_ENTRANCE, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });

        me->m_Events.AddEventAtOffset([this]
        {
            me->SetWalk(true);
            me->GetMotionMaster()->MovePoint(POINT_MOGRAINE_RAMP_FOOT, MograineRampFoot);
        }, MograineWait);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_MOGRAINE_RAMP_FOOT:
                me->GetMotionMaster()->MovePoint(POINT_MOGRAINE_RAMP_MIDDLE, MograineRampMiddle);
                break;
            case POINT_MOGRAINE_RAMP_MIDDLE:
                me->GetMotionMaster()->MovePoint(POINT_MOGRAINE_RAMP_TOP, MograineRampTop);
                break;
            case POINT_MOGRAINE_RAMP_TOP:
                if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid))
                    me->SetFacingToObject(player);
                break;
            default:
                break;
        }
    }

private:
    ObjectGuid _playerGuid;
};

// The transporter pad on the lower deck belongs to the map. Stepping onto it opens the gate to Icecrown (step 1's
// criteria is event 47614) and the Lich King bids the player enter; it does not carry them, they walk on to the door
struct at_icc_scenario_scourge_transporter : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || !BelongsInTheScenario(player) || !IsOnStep(at->GetMap(), SCENARIO_STEP_THE_CALL_OF_THE_NORTH))
            return;

        // The core passes a game event on to the scenario only for a player who is its source
        GameEvents::Trigger(GAME_EVENT_OPEN_THE_GATE_TO_ICECROWN, player, nullptr);
        Conversation::CreateConversation(CONVERSATION_THE_LICH_KING_ENTER, player, *player, player->GetGUID());
    }
};

// Saurfang's door: through it, the player enters the citadel and comes out in the Spire, where the fragments are.
// Before the gate is open (the transporter not used) it does nothing
struct at_icc_scenario_door_to_deathbringers_rise : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || IsOnStep(at->GetMap(), SCENARIO_STEP_THE_CALL_OF_THE_NORTH))
            return;

        bool const entering = IsOnStep(at->GetMap(), SCENARIO_STEP_THE_GATES_ARE_OPEN);
        if (entering)
        {
            GameEvents::Trigger(GAME_EVENT_ENTER_ICECROWN_CITADEL, player, nullptr);
            player->CastSpell(player, SPELL_ENTER_ICECROWN_CREDIT, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
            if (InstanceScript* instance = at->GetInstanceScript())
                instance->SetData(DATA_ENTERED_THE_CITADEL, 1);
        }

        TeleportToSafeLoc(player, WORLD_SAFE_LOC_DEATHBRINGERS_RISE);

        // In the Spire the Lich King sends the player after the fragments
        if (entering)
            player->m_Events.AddEventAtOffset([player]
            {
                if (player->GetMapId() == MAP_ICECROWN_CITADEL_SCENARIO)
                    Conversation::CreateConversation(CONVERSATION_SEEK_THE_FRAGMENTS, player, *player, player->GetGUID());
            }, SpireLineDelay);
    }
};

struct FragmentSoul
{
    uint32 Entry;
    uint32 Conversation;
    uint32 Fragment;
};

static constexpr FragmentSoul FragmentSouls[] =
{
    { NPC_BAELGUN_FLAMEBEARD,      CONVERSATION_BAELGUN,  GO_FRAGMENT_OF_FROSTMOURNE_1 },
    { NPC_HALAHK_THE_DEATHBRINGER, CONVERSATION_HALAHK,   GO_FRAGMENT_OF_FROSTMOURNE_2 },
    { NPC_MAGROTH_THE_DEFENDER,    CONVERSATION_MAGROTH,  GO_FRAGMENT_OF_FROSTMOURNE_3 },
    { NPC_GAVINRAD_THE_CRUEL,      CONVERSATION_GAVINRAD, GO_FRAGMENT_OF_FROSTMOURNE_4 },
};

static FragmentSoul const* FindFragmentSoul(uint32 entry)
{
    for (FragmentSoul const& soul : FragmentSouls)
        if (soul.Entry == entry)
            return &soul;
    return nullptr;
}

enum FragmentSoulEvents
{
    EVENT_WHIRLWIND = 1,
    EVENT_HAMMER_OF_JUSTICE,
    EVENT_AVENGERS_SHIELD,
    EVENT_HAMMER_OF_WRATH
};

// 93948 Baelgun Flamebeard, 93949 Halahk the Deathbringer, 93950 Magroth the Defender, 93951 Gavinrad the Cruel: souls
// freed from Frostmourne, each guarding a fragment of the blade. The Lich King speaks of each as the player nears it, and
// the fragment it guards lies beside it, out of reach until it dies; the fragment inside counts for "Seek the Fragments"
struct npc_icc_scenario_fragment_soul : public ScriptedAI
{
    npc_icc_scenario_fragment_soul(Creature* creature) : ScriptedAI(creature), _soul(FindFragmentSoul(creature->GetEntry())) { }

    void MoveInLineOfSight(Unit* who) override
    {
        if (!_greeted && _soul)
            if (Player* player = who->ToPlayer())
                if (BelongsInTheScenario(player) && me->IsWithinDistInMap(player, SoulGreetRange)
                    && IsOnStep(me->GetMap(), SCENARIO_STEP_SEEK_THE_FRAGMENTS))
                {
                    _greeted = true;
                    Conversation::CreateConversation(_soul->Conversation, player, *player, player->GetGUID());
                }

        ScriptedAI::MoveInLineOfSight(who);
    }

    void Reset() override
    {
        _events.Reset();
        _healed = false;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        switch (me->GetEntry())
        {
            case NPC_BAELGUN_FLAMEBEARD:
                _events.ScheduleEvent(EVENT_WHIRLWIND, 10s);
                break;
            case NPC_HALAHK_THE_DEATHBRINGER:
                _events.ScheduleEvent(EVENT_HAMMER_OF_JUSTICE, 15s);
                break;
            case NPC_MAGROTH_THE_DEFENDER:
                _events.ScheduleEvent(EVENT_AVENGERS_SHIELD, 5s);
                break;
            case NPC_GAVINRAD_THE_CRUEL:
                _events.ScheduleEvent(EVENT_HAMMER_OF_WRATH, 8s);
                break;
            default:
                break;
        }
    }

    // Halahk, at half health: wings and a heal, once
    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (_healed || me->GetEntry() != NPC_HALAHK_THE_DEATHBRINGER || !me->HealthBelowPctDamaged(50, damage))
            return;

        _healed = true;
        DoCastSelf(SPELL_AVENGING_WRATH, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
        DoCastSelf(SPELL_FLASH_OF_LIGHT);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_soul)
            if (GameObject* fragment = me->FindNearestGameObject(_soul->Fragment, FragmentSearchRange))
                fragment->RemoveFlag(GO_FLAG_NOT_SELECTABLE);
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
                case EVENT_WHIRLWIND:
                    DoCastSelf(SPELL_WHIRLWIND);
                    _events.Repeat(20s);
                    break;
                case EVENT_HAMMER_OF_JUSTICE:
                    DoCastVictim(SPELL_HAMMER_OF_JUSTICE);
                    _events.Repeat(25s);
                    break;
                case EVENT_AVENGERS_SHIELD:
                    DoCastVictim(SPELL_AVENGERS_SHIELD);
                    _events.Repeat(12s);
                    break;
                case EVENT_HAMMER_OF_WRATH:
                    DoCastVictim(SPELL_HAMMER_OF_WRATH);
                    _events.Repeat(14s);
                    break;
                default:
                    break;
            }
        }
    }

private:
    FragmentSoul const* _soul;
    bool _greeted = false;
    bool _healed = false;
    EventMap _events;
};

static void SetSpirePortcullis(WorldObject const* searcher, bool open)
{
    if (GameObject* portcullis = searcher->FindNearestGameObject(GO_SPIRE_PORTCULLIS, GargoyleGateRange))
        portcullis->SetGoState(open ? GO_STATE_ACTIVE : GO_STATE_READY);
}

// 97320 Stoneskin Gatekeeper: one of the two gargoyles sitting on their pressure plates in the Spire corridor. They lift
// off when a death knight comes near (at_icc_scenario_gargoyle_gate) and stay Hardened, heavy stone that damage cannot
// kill but that may still be moved. Pulled back onto its own plate (its spawn point), a lifted Gatekeeper dies; when both
// are dead the portcullis opens. A static areatrigger only sees players, so each one watches its own plate
struct npc_icc_scenario_stoneskin_gatekeeper : public ScriptedAI
{
    npc_icc_scenario_stoneskin_gatekeeper(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        Land();
    }

    void JustReachedHome() override
    {
        // The death knight died or left: back on the plate, and the way stays open for another try
        Land();
        SetSpirePortcullis(me, true);
    }

    uint32 GetData(uint32 id) const override
    {
        return id == DATA_GATEKEEPER_LIFTED && _lifted ? 1 : 0;
    }

    void SetGUID(ObjectGuid const& guid, int32 id) override
    {
        if (id != DATA_GATEKEEPER_LIFTED || _lifted)
            return;

        _lifted = true;
        _leftPlate = false;
        me->SetImmuneToPC(false);
        me->SetReactState(REACT_AGGRESSIVE);
        if (Player* player = ObjectAccessor::GetPlayer(*me, guid))
            AttackStart(player);
    }

    void JustDied(Unit* /*killer*/) override
    {
        std::vector<Creature*> gatekeepers;
        me->GetCreatureListWithEntryInGrid(gatekeepers, NPC_STONESKIN_GATEKEEPER, GargoyleGateRange);
        for (Creature* gatekeeper : gatekeepers)
            if (gatekeeper->IsAlive())
                return;

        SetSpirePortcullis(me, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (_lifted && !me->IsInEvadeMode())
        {
            _plateTimer += Milliseconds(diff);
            if (_plateTimer >= PressurePlateCheck)
            {
                _plateTimer = 0ms;
                bool const onPlate = me->GetExactDist(me->GetHomePosition()) <= PressurePlateRadius;
                if (!onPlate)
                    _leftPlate = true;
                else if (_leftPlate)
                {
                    // Back on its plate: the stone gives way
                    me->RemoveAurasDueToSpell(SPELL_HARDENED);
                    me->KillSelf();
                    return;
                }
            }
        }

        UpdateVictim();
    }

private:
    void Land()
    {
        _lifted = false;
        _leftPlate = false;
        _plateTimer = 0ms;
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToPC(true);
        if (!me->HasAura(SPELL_HARDENED))
            DoCastSelf(SPELL_HARDENED, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }

    bool _lifted = false;
    bool _leftPlate = false;
    Milliseconds _plateTimer = 0ms;
};

// The corridor before the gargoyle gate: a death knight seeking the fragments wakes the two Gatekeepers, and the
// portcullis behind them slams shut
struct at_icc_scenario_gargoyle_gate : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || !BelongsInTheScenario(player) || !IsOnStep(at->GetMap(), SCENARIO_STEP_SEEK_THE_FRAGMENTS))
            return;

        std::vector<Creature*> gatekeepers;
        at->GetCreatureListWithEntryInGrid(gatekeepers, NPC_STONESKIN_GATEKEEPER, GargoyleGateRange);
        std::erase_if(gatekeepers, [](Creature const* gatekeeper)
        {
            return !gatekeeper->IsAlive() || gatekeeper->IsInEvadeMode() || gatekeeper->AI()->GetData(DATA_GATEKEEPER_LIFTED);
        });
        if (gatekeepers.empty())
            return;

        gatekeepers.front()->TextEmote(BROADCAST_TEXT_GARGOYLES_LIFT_OFF, player, true);
        SetSpirePortcullis(at, false);
        for (Creature* gatekeeper : gatekeepers)
            gatekeeper->AI()->SetGUID(player->GetGUID(), DATA_GATEKEEPER_LIFTED);
    }
};

// The landing beside the citadel: a death knight who stands here at the start pings Mograine. An areatrigger needs the
// player in the world, so a login or a teleport under the loading screen does not start him unseen. On the first arrival
// the entrance scene plays here first, and its end pings him instead (event 45596)
struct at_icc_scenario_landing : AreaTriggerAI
{
    using AreaTriggerAI::AreaTriggerAI;

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || !BelongsInTheScenario(player) || !IsOnStep(at->GetMap(), SCENARIO_STEP_THE_CALL_OF_THE_NORTH)
            || player->GetQuestObjectiveData(QUEST_THE_CALL_OF_ICECROWN, QUEST_OBJECTIVE_INTRO_SCENE) == 0)
            return;

        player->CastSpell(nullptr, SPELL_ICECROWN_MOGRAINE_PING, CastSpellExtraArgsInit{ .TriggerFlags = TRIGGERED_FULL_MASK });
    }
};

// 188022 - Icecrown - Mograine Ping (aimed at Mograine by conditions)
class spell_icc_scenario_mograine_ping : public SpellScript
{
    void HandleDummy(SpellEffIndex /*effIndex*/) const
    {
        if (Creature* mograine = GetHitCreature())
            mograine->AI()->SetGUID(GetCaster()->GetGUID(), 0);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_icc_scenario_mograine_ping::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_instance_icecrown_citadel_scenario()
{
    new instance_icecrown_citadel_scenario();
    RegisterCreatureAI(npc_icc_scenario_darion_mograine);
    RegisterCreatureAI(npc_icc_scenario_fragment_soul);
    RegisterCreatureAI(npc_icc_scenario_stoneskin_gatekeeper);
    RegisterAreaTriggerAI(at_icc_scenario_scourge_transporter);
    RegisterAreaTriggerAI(at_icc_scenario_door_to_deathbringers_rise);
    RegisterAreaTriggerAI(at_icc_scenario_landing);
    RegisterAreaTriggerAI(at_icc_scenario_gargoyle_gate);
    RegisterSpellScript(spell_icc_scenario_mograine_ping);
}
