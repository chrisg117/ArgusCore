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
#include "AreaTrigger.h"
#include "AreaTriggerAI.h"
#include "Creature.h"
#include "DB2Stores.h"
#include "GameObject.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Item.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "Position.h"
#include "ScriptedCreature.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include "tirisfal_artifact_scenarios.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <set>

// ScenarioStep.OrderIndex of "Blade in Twilight"
enum BladeInTwilightStages : uint8
{
    STAGE_THE_TWILIGHT_CAMP                 = 0,
    STAGE_RAIDING_THE_TOMB_RAIDERS          = 1,
    STAGE_THE_DESECRATED_TOMB               = 2,
    STAGE_THE_TOMB_OF_TYR                   = 3,
    STAGE_RECONSECRATION                    = 4,
    STAGE_DONT_FEAR_THE_REAPER              = 5,
    STAGE_DARK_PASSAGE                      = 6,
    STAGE_DEATH_TO_THE_DEACON               = 7,
    STAGE_THE_BLADE_OF_THE_BLACK_EMPIRE     = 8,
    STAGE_THE_TRUE_DEATH_OF_ZAKAJZ          = 9
};

// Game events counted by the scenario's criteria. "The Twilight Camp" has none: its three criteria are for
// reading Slaghammer's notes, which the core counts when a player uses the objects.
enum BladeInTwilightGameEvents
{
    GAME_EVENT_TOMB_ENTERED                 = 48121, // "Raiding the Tomb Raiders"
    GAME_EVENT_DOOR_GUARDS_DEFEATED         = 48160, // "The Desecrated Tomb"
    GAME_EVENT_TOMB_OF_TYR_ENTERED          = 48306, // "The Tomb of Tyr"
    GAME_EVENT_DAMPENING_RITUAL_STOPPED     = 48161, // "Reconsecration", once for each of the four rituals
    GAME_EVENT_AMASSING_DARKNESS_DEFEATED   = 48994, // "Don't Fear the Reaper"
    GAME_EVENT_PRISON_CHAMBER_REACHED       = 48393, // "Dark Passage"
    GAME_EVENT_TWILIGHT_DEACON_KILLED       = 48421, // "Death to the Deacon"
    GAME_EVENT_BLADE_TAKEN                  = 48493, // "The Blade of the Black Empire"
    GAME_EVENT_ZAKAJZ_DRAINED               = 48528  // "The True Death of Zakajz"
};

enum BladeInTwilightCreatures
{
    NPC_KILL_CREDIT_FLY_TO_SCENARIO         = 101365, // the quest's "Go to the marked location in Tirisfal Glades"
    NPC_KILL_CREDIT_WON_SCENARIO            = 101364, // the quest's "Stop the Ritual and acquire the Blade"
    NPC_SHADOWLORD_SLAGHAMMER_SPY           = 101899,
    NPC_TWILIGHT_RITUALIST                  = 101875,
    NPC_FLESH_SPAWN                         = 101120,
    NPC_AMASSING_DARKNESS                   = 102693,
    NPC_WARD_OF_TYR                         = 101294, // General Purpose Stalker
    NPC_BORGOTH_THE_MASTER_REAVER           = 101897,
    NPC_TWILIGHT_BLADETWISTER               = 101127,
    NPC_VOID_TENDRIL                        = 102927,
    NPC_FACELESS_GUARDIAN                   = 102015,
    NPC_FACELESS_CORRUPTER                  = 101118,
    NPC_TWILIGHT_DEACON_FARTHING            = 101148,
    NPC_XALATATH                            = 102255
};

enum BladeInTwilightSpells
{
    // Twilight Shadowcaster, Twilight Ritualist
    SPELL_SHADOW_BOLT                       = 183345,
    SPELL_DAMPEN_THE_LIGHT                  = 200923,

    // Twilight Bladetwister
    SPELL_TWILIGHT_WHIRL                    = 203115, // a channel that sets off the other "Twilight Whirl" (203114)

    // The wards of the tomb
    SPELL_WARD_DAMPENED                     = 200924, // "Statue"; TODO: not what a dampened ward looks like on retail
    SPELL_COSMETIC_HOLY_WARDS               = 202014,

    // Amassing Darkness
    SPELL_AMASSING_DARKNESS_AURA            = 202981, // "Dampened", which the other "Dampened" (202925) triggers
    SPELL_ARMAGEDDON                        = 202995,
    SPELL_ARMAGEDDON_BLAST                  = 203014,

    // Borgoth the Master Reaver
    SPELL_TWILIGHT_STRIKE                   = 203112,
    SPELL_EDGE_OF_TWILIGHT                  = 203119,

    // Void Tendril
    SPELL_MIND_FLAY                         = 203313,

    // The cutscene before the prison chamber
    SPELL_BOSS_REVEAL_SCENE                 = 201321,

    // Faceless Guardian, Faceless Corrupter, Twilight Deacon Farthing
    // TODO: which "Shadow Crash" and "Void Bolt" they cast on retail is not known
    SPELL_SHADOW_CRASH                      = 199915,
    SPELL_VOID_BOLT                         = 156722,

    // Twilight Deacon Farthing
    SPELL_CHANNELING_WITH_BLADE             = 201750,
    SPELL_SHADOW_IMAGES                     = 203514, // "Shadow Image", which sets off the two that summon one (203384, 203385)
    SPELL_CONVERSATION_DEATH_OF_THE_DEACON  = 201791,
    SPELL_XALATATH_BEAM                     = 201792, // "Chained"; TODO: a stand-in, what the blade's beam is on retail is not known
    SPELL_COSMETIC_DISSOLVE_FARTHING        = 224283,

    // Zakajz the Corruptor
    SPELL_ZAKAJZ_REBIRTH_STATE              = 206390,

    // The cutscene of a player holding up the blade
    SPELL_BLADE_OF_THE_BLACK_EMPIRE         = 201780,

    // The end of Zakajz
    SPELL_DARK_DRAIN_SPELL_BAR              = 204919, // "Dark Drain Void God Spell Bar": puts "Dark Drain" (201904) on the extra action button
    SPELL_CONVERSATION_DEATH_TO_ZAKAJZ      = 201917,
    SPELL_DRAINED_ZAKAJZ                    = 206467,
    SPELL_COSMETIC_DISSOLVE                 = 207454
};

enum BladeInTwilightTexts
{
    // Twilight Shadowcaster
    SAY_SHADOWCASTER_INTRUDER               = 0,
    SAY_SHADOWCASTER_TRAITOR                = 1,

    // Shadowlord Slaghammer, at the door
    SAY_SLAGHAMMER_EXPECTING                = 0,
    SAY_SLAGHAMMER_MOIRA_SAID               = 1,
    SAY_SLAGHAMMER_DEACON_BELOW             = 2,

    // Shadowlord Slaghammer, in the tomb
    SAY_SLAGHAMMER_WARDS                    = 0,
    SAY_SLAGHAMMER_TRAP                     = 1,
    SAY_SLAGHAMMER_PATH_OPEN                = 2,
    SAY_SLAGHAMMER_MIND_GAMES               = 3,
    SAY_SLAGHAMMER_STAY_BEHIND              = 4,
    SAY_SLAGHAMMER_YE_DID_IT                = 5,
    SAY_SLAGHAMMER_KEEP_AN_EYE              = 6,

    // Amassing Darkness
    EMOTE_AMASSING_DARKNESS_RITUAL_BROKEN   = 0,
    SAY_AMASSING_DARKNESS_ELIMINATE         = 1,

    // Twilight Deacon Farthing
    SAY_FARTHING_DEFEND_ME                  = 0,
    SAY_FARTHING_STOP_ME                    = 1,
    SAY_FARTHING_MIND                       = 2
};

enum BladeInTwilightMisc
{
    POINT_SLAGHAMMER_AMBUSH                 = 1,
    POINT_SLAGHAMMER_BARRIER                = 2,
    POINT_FARTHING_LAST_STAND               = 3,
    POINT_SLAGHAMMER_YE_DID_IT              = 4,
    POINT_SLAGHAMMER_PORTAL_STONE           = 5,

    TASK_GROUP_AMBUSH                       = 1,
    TASK_GROUP_BARRIER                      = 2,
    TASK_GROUP_COMBAT                       = 3,

    ACTION_AMASSING_DARKNESS_AWAKEN         = 1,
    ACTION_SLAGHAMMER_STAY_BEHIND           = 2,
    ACTION_FARTHING_GUARDS_FALLEN           = 3,

    // AnimKit.db2: lying dead (what the cutscene's script calls it)
    ANIM_KIT_DEAD                           = 730,

    GO_XALATATH                             = 247690,
    GO_PORTAL_TO_DALARAN_CITY               = 247351,
    ITEM_XALATATH                           = 128827
};

// `creature`.`guid` and `gameobject`.`guid` of what stands at the door of the tomb
static constexpr ObjectGuid::LowType DoorShadowcasterSpawnId = 15390013;
static constexpr ObjectGuid::LowType DoorBladetwisterSpawnId = 15390014;
static constexpr ObjectGuid::LowType SlaghammerSpawnId = 15390015;
static constexpr ObjectGuid::LowType TombBarrierSpawnId = 15390004;

// `creature`.`guid` of the four Twilight Ritualists in the hall of the tomb, each with that of the ward it
// dampens
struct DampeningRitual
{
    ObjectGuid::LowType RitualistSpawnId;
    ObjectGuid::LowType WardSpawnId;
};

static constexpr std::array<DampeningRitual, 4> DampeningRituals =
{{
    { 15390016, 15390020 }, // north-east of the dais
    { 15390017, 15390021 }, // north-west
    { 15390018, 15390022 }, // south-east
    { 15390019, 15390023 }  // south-west
}};

// The middle of the dais that the four wards stand around, where the Amassing Darkness comes when the last
// ritual is stopped (measured on the tomb's collision), turned to the door
Position const AmassingDarknessPosition = { 1963.2f, 2336.4f, 80.55f, 0.0f };

// The stairs from the hall down to the prison, under the broken seal in the hall's floor: where Borgoth
// and the Twilight Bladetwister with him come up them (measured on the tomb's collision)
Position const BorgothPosition = { 1908.0f, 2335.0f, 68.95f, 0.0f };
Position const BorgothBladetwisterPosition = { 1905.0f, 2333.0f, 66.17f, 0.0f };

// The Void Tendrils stand at the northern edge of the round chamber halfway down, along which the way
// goes: about this place, and how near to it a player is when Slaghammer speaks of them
Position const VoidTendrilsPosition = { 1849.0f, 2309.0f, 47.0f };
static constexpr float VoidTendrilsWarningDistance = 32.0f;

// The cutscene that shows the Twilight Deacon at his work starts when a player comes this near to this
// place: in the passage, a little before the door of the prison chamber (which is at y 2248 to 2257; both
// measured on the tomb's collision). "Dark Passage" is done when the cutscene is over. It lasts 23
// seconds; should the client never say that it is over, the stage is done after this time.
Position const BossRevealPosition = { 1873.0f, 2266.0f, 36.6f };
static constexpr float BossRevealDistance = 9.0f;
static constexpr Milliseconds BossRevealTimeout = 40s;

// `creature`.`guid` of the Deacon and of Zakajz in the prison chamber, and how many guards the Deacon has
static constexpr ObjectGuid::LowType FarthingSpawnId = 15390038;
static constexpr ObjectGuid::LowType ZakajzSpawnId = 15390041;
static constexpr uint32 FarthingsGuards = 2;

// Where the Deacon goes when the blade fails him, turned to the blade, and where the blade floats then,
// turned to him. Both were marked in game against the video of the retail scenario.
Position const FarthingLastStandPosition = { 1888.63f, 2209.15f, 38.37f, 3.85f };
Position const XalatathPosition = { 1870.17f, 2193.32f, 39.0f, 0.71f };

// The object by which the blade is taken stands on the floor under it
Position const XalatathObjectPosition = { 1870.17f, 2193.32f, 37.49f, 0.71f };

// Where the Deacon's Void Tendrils come up, between the dome and the door on the dome's left as seen from
// the door (estimated from the video)
Position const FarthingVoidTendrilPositions[] =
{
    { 1892.0f, 2186.0f, 38.0f, 1.9f },
    { 1896.0f, 2193.0f, 38.0f, 2.3f },
    { 1891.0f, 2199.0f, 38.0f, 2.8f }
};

// When Zakajz is gone Slaghammer comes into the prison chamber: where he speaks, where he goes to use
// his portal stone, turned to where the portal opens, and where it opens, turned to him. All three were
// marked in game against the video of the retail scenario.
Position const SlaghammerYeDidItPosition = { 1876.31f, 2212.9f, 37.83f, 4.71f };
Position const SlaghammerPortalStonePosition = { 1882.19f, 2212.11f, 38.27f, 5.07f };
Position const TwilightPortalPosition = { 1884.09f, 2207.05f, 38.37f, 1.93f };

// He comes from where he stayed behind, in the passage before the chamber. Should he not be within this
// distance of the chamber's door, he is brought to it (the scenario's graveyard there, WorldSafeLocs 5351).
Position const PrisonDoorPosition = { 1877.69f, 2249.0f, 37.55f, 4.71f };
static constexpr float SlaghammerComesDistance = 80.0f;

// How far a Faceless guard of the Deacon calls the other into its fight
static constexpr float FacelessGuardCallDistance = 60.0f;

// "Raiding the Tomb Raiders" is done when a player has swum down the lake and come into the tomb. Nothing
// in the client's data says where that is counted. This is where the tomb's first graveyard is
// ("7.0 Artifacts - Keeper's Rest - Entrance Graveyard", WorldSafeLocs 5335), and how near to it a player
// has to come.
Position const TombEntrancePosition = { 2072.02f, 2348.32f, 66.4f };
static constexpr float TombEntranceDistance = 20.0f;

// Where Slaghammer falls on the two guards of the door, between them, and where he stands to take the
// barrier down, turned to it. Both were marked in game against a video of the retail scenario.
Position const SlaghammerAmbushPosition = { 2023.65f, 2337.84f, 75.21f };
Position const SlaghammerBarrierPosition = { 2020.65f, 2338.02f, 75.28f, float(M_PI) };

// "The Tomb of Tyr" is done when a player has gone through the door that the barrier was in. The door's
// opening is at x 2011.5, seven yards wide around y 2337.7, and the passage behind it runs on towards
// lower x (measured on the tomb's collision). A player has entered who is this far past the door, no
// farther than this to either side of the passage's middle, and no higher than this above its floor.
static constexpr float TombDoorX = 2011.5f;
static constexpr float TombDoorY = 2337.7f;
static constexpr float TombDoorZ = 75.5f;
static constexpr float TombInsideDistance = 5.0f;
static constexpr float TombPassageHalfWidth = 8.0f;
static constexpr float TombPassageHeight = 14.0f;

// When Slaghammer answers the Shadowcaster's yell and runs at the guards, when he strikes, and when the
// Shadowcaster calls him a traitor, counted from the yell
static constexpr Milliseconds SlaghammerAnswerDelay = 3s;
static constexpr Milliseconds SlaghammerStrikeDelay = 7s;
static constexpr Milliseconds ShadowcasterTraitorDelay = 9s;

// What Slaghammer does when the guards have fallen, counted from the fall of the last: he goes to the
// barrier, introduces himself, works on the barrier, says what lies ahead, and has the barrier down
static constexpr Milliseconds SlaghammerToBarrierDelay = 1s;
static constexpr Milliseconds SlaghammerMoiraSaidDelay = 6s;
static constexpr Milliseconds SlaghammerChannelDelay = 11s;
static constexpr Milliseconds SlaghammerDeaconBelowDelay = 16s;
static constexpr Milliseconds BarrierDownDelay = 23s;

// How long the barrier takes to fade, and how Slaghammer keeps to the player afterwards
static constexpr Milliseconds BarrierFadeTime = 3s;
static constexpr float SlaghammerFollowDistance = 2.0f;
static constexpr float SlaghammerFollowAngle = float(M_PI) / 2.0f;
static constexpr float SlaghammerPlayerSearchDistance = 200.0f;

// The Slaghammer of the door leaves when the one who goes through the tomb takes his place, and does not
// come back while the scenario lasts
static constexpr Seconds SlaghammerNoRespawnTime = 168h;

// The times below are from the video of the retail scenario.
// How long after the player has come into the hall Slaghammer says what the ritualists are doing there
static constexpr Milliseconds SlaghammerWardsDelay = 3s;

// The Shadow Bolts of a Shadowcaster or a ritualist, and how often a ritualist looks whether its ward is
// still dampened
static constexpr Milliseconds ShadowBoltFirstDelay = 500ms;
static constexpr Milliseconds ShadowBoltInterval = 3s;
static constexpr Milliseconds RitualistDampenInterval = 2s;
static constexpr float RitualistWardSearchDistance = 30.0f;

// What comes out of a ward when its ritual is stopped, and how far from the middle of its pedestal
static constexpr uint32 FleshSpawnPerWard = 4;
static constexpr float FleshSpawnDistance = 4.5f;

// When the last ritual is stopped the Amassing Darkness comes onto the dais, with this much of its health
// (about a fifth, as it looks in the video). Slaghammer sees the trap, and the darkness sets about its
// Armageddon: a channel of this length that ends in the blast, which also ends the darkness.
static constexpr int32 AmassingDarknessHealthPct = 20;
static constexpr Milliseconds SlaghammerTrapDelay = 500ms;
static constexpr Milliseconds AmassingDarknessAwakenDelay = 2500ms;
static constexpr Milliseconds ArmageddonChannelTime = 25s;

// and how long after the darkness is gone Slaghammer says that the way on is open
static constexpr Milliseconds SlaghammerPathOpenDelay = 1s;

// What the Twilight's Hammer does in a fight besides striking
static constexpr Milliseconds TwilightWhirlFirstDelay = 6s;
static constexpr Milliseconds TwilightWhirlInterval = 14s;
static constexpr Milliseconds TwilightStrikeFirstDelay = 4s;
static constexpr Milliseconds TwilightStrikeInterval = 7s;
static constexpr Milliseconds EdgeOfTwilightFirstDelay = 10s;
static constexpr Milliseconds EdgeOfTwilightInterval = 20s;

// A Void Tendril's Mind Flay is a channel of six seconds
static constexpr Milliseconds MindFlayFirstDelay = 1s;
static constexpr Milliseconds MindFlayInterval = 7s;

// How long after the cutscene Slaghammer says that he stays behind
static constexpr Milliseconds SlaghammerStayBehindDelay = 2s;

// What the Deacon and his Faceless guards cast
static constexpr Milliseconds VoidBoltFirstDelay = 1s;
static constexpr Milliseconds VoidBoltInterval = 3500ms;
static constexpr Milliseconds ShadowCrashFirstDelay = 5s;
static constexpr Milliseconds ShadowCrashInterval = 12s;

// The Deacon calls up his Shadow Images and Void Tendrils when he has this much of his health left, and
// the blade fails him at this much. Then he and the blade speak (the conversation's lines begin at 0,
// 7.5, 16.25 and 23.25 seconds): at his last plea the blade turns its beam on him, he dissolves in it and
// is dead as the blade begins to speak to the player. What is left of him is gone this long after.
static constexpr int32 FarthingHallucinationsHealthPct = 80;
static constexpr int32 FarthingFailingHealthPct = 1;
static constexpr Milliseconds XalatathBeamDelay = 16250ms;
static constexpr Milliseconds FarthingDissolveDelay = 19s;
static constexpr Milliseconds FarthingDeathDelay = 22500ms;
static constexpr Milliseconds FarthingCorpseTime = 1s;

// The blade speaks of Zakajz this long after it has been taken, when the cutscene of the player holding it
// up (nine seconds) is over
static constexpr Milliseconds XalatathOnZakajzDelay = 12s;

// Zakajz dissolves when he has been drained, and is gone this long after; he does not come back while the
// scenario lasts
static constexpr Milliseconds ZakajzDissolveTime = 3s;
static constexpr Seconds ZakajzNoRespawnTime = 168h;

// What Slaghammer does then, counted from the end of Zakajz: he comes running, says that it is done, walks
// to where he uses his portal stone, works on it, has the portal open, and warns of the blade
static constexpr Milliseconds SlaghammerComesDelay = 3s;
static constexpr Milliseconds SlaghammerYeDidItDelay = 10s;
static constexpr Milliseconds SlaghammerToPortalStoneDelay = 17s;
static constexpr Milliseconds SlaghammerPortalStoneDelay = 19500ms;
static constexpr Milliseconds TwilightPortalDelay = 21500ms;
static constexpr Milliseconds SlaghammerKeepAnEyeDelay = 26s;

static constexpr uint32 CheckInterval = 1000;

class BladeInTwilightScenario final : public TirisfalArtifactScenario
{
public:
    explicit BladeInTwilightScenario(InstanceMap* map) : TirisfalArtifactScenario(map), _currentStep(nullptr), _checkTimer(0),
        _barrierBeingTakenDown(false), _amassingDarknessDefeated(false), _voidTendrilsSpokenOf(false), _bossRevealStarted(false),
        _prisonChamberReached(false), _farthingsGuardsSlain(0), _farthingDead(false), _bladeTaken(false),
        _zakajzDrained(false) { }

    // The core has no script hook for scenario step changes, so the current step is polled.
    void Update(uint32 diff) override
    {
        _scheduler.Update(diff);

        InstanceScenario const* scenario = instance->GetInstanceScenario();
        if (!scenario)
            return;

        ScenarioStepEntry const* step = scenario->GetStep();
        if (step != _currentStep)
        {
            _currentStep = step;
            if (step)
                StartStage(BladeInTwilightStages(step->OrderIndex));
        }

        if (!step)
            return;

        _checkTimer += diff;
        if (_checkTimer < CheckInterval)
            return;

        _checkTimer = 0;

        switch (step->OrderIndex)
        {
            case STAGE_RAIDING_THE_TOMB_RAIDERS:
                UpdateRaidingTheTombRaiders();
                break;
            case STAGE_THE_TOMB_OF_TYR:
                UpdateTheTombOfTyr();
                break;
            case STAGE_DARK_PASSAGE:
                UpdateDarkPassage();
                break;
            case STAGE_THE_BLADE_OF_THE_BLACK_EMPIRE:
                UpdateTheBladeOfTheBlackEmpire();
                break;
            default:
                break;
        }
    }

    void OnUnitDeath(Unit* unit) override
    {
        Creature* creature = unit->ToCreature();
        if (!creature)
            return;

        switch (creature->GetEntry())
        {
            case NPC_TWILIGHT_RITUALIST:
                RitualStopped(creature);
                return;
            case NPC_AMASSING_DARKNESS:
                _amassingDarknessDefeated = true;
                if (IsStage(STAGE_DONT_FEAR_THE_REAPER))
                    SendScenarioEvent(GAME_EVENT_AMASSING_DARKNESS_DEFEATED);
                return;
            case NPC_FACELESS_GUARDIAN:
            case NPC_FACELESS_CORRUPTER:
                if (++_farthingsGuardsSlain == FarthingsGuards)
                    if (Creature* farthing = GetLivingCreature(FarthingSpawnId))
                        farthing->AI()->DoAction(ACTION_FARTHING_GUARDS_FALLEN);
                return;
            case NPC_TWILIGHT_DEACON_FARTHING:
                FarthingDead();
                return;
            default:
                break;
        }

        ObjectGuid::LowType spawnId = creature->GetSpawnId();
        if (spawnId != DoorShadowcasterSpawnId && spawnId != DoorBladetwisterSpawnId)
            return;

        _doorGuardsSlain.insert(spawnId);
        if (IsStage(STAGE_THE_DESECRATED_TOMB) && HaveDoorGuardsFallen())
            DoorGuardsDefeated();
    }

    void SetData(uint32 type, uint32 /*data*/) override
    {
        switch (type)
        {
            case DATA_BOSS_REVEAL_SCENE_OVER:
                PrisonChamberReached();
                break;
            case DATA_FARTHING_FAILING:
                FarthingFailing();
                break;
            case DATA_ZAKAJZ_DRAINED:
                ZakajzDrained();
                break;
            default:
                break;
        }
    }

private:
    void StartStage(BladeInTwilightStages stage)
    {
        switch (stage)
        {
            case STAGE_THE_DESECRATED_TOMB:
                StartTheDesecratedTomb();
                break;
            case STAGE_RECONSECRATION:
                StartReconsecration();
                break;
            case STAGE_DONT_FEAR_THE_REAPER:
                StartDontFearTheReaper();
                break;
            case STAGE_DARK_PASSAGE:
                StartDarkPassage();
                break;
            case STAGE_DEATH_TO_THE_DEACON:
                StartDeathToTheDeacon();
                break;
            case STAGE_THE_BLADE_OF_THE_BLACK_EMPIRE:
                StartTheBladeOfTheBlackEmpire();
                break;
            case STAGE_THE_TRUE_DEATH_OF_ZAKAJZ:
                StartTheTrueDeathOfZakajz();
                break;
            default:
                break;
        }
    }

    bool IsStage(BladeInTwilightStages stage) const
    {
        return _currentStep && _currentStep->OrderIndex == stage;
    }

    void UpdateRaidingTheTombRaiders()
    {
        for (MapReference const& ref : instance->GetPlayers())
        {
            Player const* player = ref.GetSource();
            if (!player || player->IsGameMaster() || !player->IsAlive())
                continue;

            if (player->GetExactDist(TombEntrancePosition) > TombEntranceDistance)
                continue;

            SendScenarioEvent(GAME_EVENT_TOMB_ENTERED);
            return;
        }
    }

    // The door of the tomb has two guards, a Twilight Shadowcaster and a Twilight Bladetwister, and
    // Shadowlord Slaghammer is with them: a spy of Moira's, who turns on them when the player comes.
    void StartTheDesecratedTomb()
    {
        if (HaveDoorGuardsFallen())
        {
            DoorGuardsDefeated();
            return;
        }

        if (Creature* shadowcaster = GetLivingCreature(DoorShadowcasterSpawnId))
            shadowcaster->AI()->Talk(SAY_SHADOWCASTER_INTRUDER);

        _scheduler.Schedule(SlaghammerAnswerDelay, TASK_GROUP_AMBUSH, [this](TaskContext /*context*/)
        {
            Creature* slaghammer = GetLivingCreature(SlaghammerSpawnId);
            if (!slaghammer)
                return;

            slaghammer->AI()->Talk(SAY_SLAGHAMMER_EXPECTING);
            slaghammer->SetWalk(false);
            slaghammer->GetMotionMaster()->MovePoint(POINT_SLAGHAMMER_AMBUSH, SlaghammerAmbushPosition);
        });

        _scheduler.Schedule(SlaghammerStrikeDelay, TASK_GROUP_AMBUSH, [this](TaskContext /*context*/)
        {
            Creature* slaghammer = GetLivingCreature(SlaghammerSpawnId);
            Creature* guard = GetLivingCreature(DoorShadowcasterSpawnId);
            if (!guard)
                guard = GetLivingCreature(DoorBladetwisterSpawnId);

            if (!slaghammer || !guard)
                return;

            slaghammer->SetImmuneToNPC(false);
            slaghammer->SetReactState(REACT_AGGRESSIVE);
            slaghammer->AI()->AttackStart(guard);
        });

        _scheduler.Schedule(ShadowcasterTraitorDelay, TASK_GROUP_AMBUSH, [this](TaskContext /*context*/)
        {
            if (Creature* shadowcaster = GetLivingCreature(DoorShadowcasterSpawnId))
                shadowcaster->AI()->Talk(SAY_SHADOWCASTER_TRAITOR);
        });
    }

    // With the guards dead Slaghammer says who he is and takes down the barrier in the door, which is what
    // the stage is done with. After that he keeps to the player and answers questions.
    void DoorGuardsDefeated()
    {
        if (_barrierBeingTakenDown)
            return;

        _barrierBeingTakenDown = true;
        _scheduler.CancelGroup(TASK_GROUP_AMBUSH);

        _scheduler.Schedule(SlaghammerToBarrierDelay, TASK_GROUP_BARRIER, [this](TaskContext /*context*/)
        {
            Creature* slaghammer = GetLivingCreature(SlaghammerSpawnId);
            if (!slaghammer)
                return;

            slaghammer->SetReactState(REACT_PASSIVE);
            slaghammer->SetImmuneToNPC(true);
            slaghammer->SetWalk(false);
            slaghammer->GetMotionMaster()->MovePoint(POINT_SLAGHAMMER_BARRIER, SlaghammerBarrierPosition, true, SlaghammerBarrierPosition.GetOrientation());
        });

        _scheduler.Schedule(SlaghammerMoiraSaidDelay, TASK_GROUP_BARRIER, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetLivingCreature(SlaghammerSpawnId))
                slaghammer->AI()->Talk(SAY_SLAGHAMMER_MOIRA_SAID);
        });

        // TODO: what he casts at the barrier on retail is not known; he holds a spell meanwhile
        _scheduler.Schedule(SlaghammerChannelDelay, TASK_GROUP_BARRIER, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetLivingCreature(SlaghammerSpawnId))
                slaghammer->SetEmoteState(EMOTE_STATE_SPELL_CHANNEL_OMNI);
        });

        _scheduler.Schedule(SlaghammerDeaconBelowDelay, TASK_GROUP_BARRIER, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetLivingCreature(SlaghammerSpawnId))
                slaghammer->AI()->Talk(SAY_SLAGHAMMER_DEACON_BELOW);
        });

        _scheduler.Schedule(BarrierDownDelay, TASK_GROUP_BARRIER, [this](TaskContext /*context*/)
        {
            BarrierDown();
        });
    }

    void BarrierDown()
    {
        if (GameObject* barrier = instance->GetGameObjectBySpawnId(TombBarrierSpawnId))
        {
            barrier->SetGoState(GO_STATE_ACTIVE);
            barrier->DespawnOrUnsummon(BarrierFadeTime);
        }

        if (Creature* slaghammer = GetLivingCreature(SlaghammerSpawnId))
        {
            slaghammer->SetEmoteState(EMOTE_ONESHOT_NONE);
            slaghammer->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);
            if (Player* player = GetPlayerToFollow())
                slaghammer->GetMotionMaster()->MoveFollow(player, SlaghammerFollowDistance, ChaseAngle(SlaghammerFollowAngle));
        }

        SendScenarioEvent(GAME_EVENT_DOOR_GUARDS_DEFEATED);
    }

    void UpdateTheTombOfTyr()
    {
        for (MapReference const& ref : instance->GetPlayers())
        {
            Player const* player = ref.GetSource();
            if (!player || player->IsGameMaster() || !player->IsAlive())
                continue;

            if (player->GetPositionX() > TombDoorX - TombInsideDistance || std::abs(player->GetPositionY() - TombDoorY) > TombPassageHalfWidth)
                continue;

            if (player->GetPositionZ() < TombDoorZ - TombInsideDistance || player->GetPositionZ() > TombDoorZ + TombPassageHeight)
                continue;

            SendScenarioEvent(GAME_EVENT_TOMB_OF_TYR_ENTERED);
            return;
        }
    }

    // In the hall of the tomb four Twilight Ritualists keep the wards of Tyr dampened, so that the Shadow
    // Deacon can work below. Slaghammer goes in with the player, and from here on fights at the player's
    // side.
    void StartReconsecration()
    {
        SummonSlaghammerForTheTomb();

        // Rituals that were stopped before the stage began could not be counted then
        for (std::size_t i = 0; i < _ritualsStopped.size(); ++i)
            SendScenarioEvent(GAME_EVENT_DAMPENING_RITUAL_STOPPED);

        _scheduler.Schedule(SlaghammerWardsDelay, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetSlaghammerInTheTomb())
                slaghammer->AI()->Talk(SAY_SLAGHAMMER_WARDS);
        });
    }

    // The Slaghammer who goes through the tomb is another creature than the one at the door ("Ironforge
    // Spy", no longer "Twilight's Hammer") and takes that one's place where it stands. On retail he comes
    // with an aura on the player, "Slaghammer's With Ye!" (201002), whose summon needs a SummonProperties
    // record (3747) that the client's data does not have.
    void SummonSlaghammerForTheTomb()
    {
        if (GetSlaghammerInTheTomb())
            return;

        Position position;
        if (Creature* slaghammerAtTheDoor = GetLivingCreature(SlaghammerSpawnId))
        {
            position = slaghammerAtTheDoor->GetPosition();
            slaghammerAtTheDoor->DespawnOrUnsummon(0s, SlaghammerNoRespawnTime);
        }
        else if (Player const* player = GetPlayerToFollow())
            position = player->GetPosition();
        else
            return;

        if (TempSummon* slaghammer = instance->SummonCreature(NPC_SHADOWLORD_SLAGHAMMER_SPY, position))
            _slaghammerGuid = slaghammer->GetGUID();
    }

    // A ritualist has died. Its ward is dampened no longer and shines again, and what the ritual had
    // gathered in it comes out as Flesh Spawn.
    void RitualStopped(Creature const* ritualist)
    {
        auto ritual = std::find_if(DampeningRituals.begin(), DampeningRituals.end(), [ritualist](DampeningRitual const& candidate)
        {
            return candidate.RitualistSpawnId == ritualist->GetSpawnId();
        });

        if (ritual == DampeningRituals.end() || !_ritualsStopped.insert(ritual->RitualistSpawnId).second)
            return;

        if (Creature* ward = instance->GetCreatureBySpawnId(ritual->WardSpawnId))
        {
            ward->RemoveAurasDueToSpell(SPELL_DAMPEN_THE_LIGHT);
            ward->RemoveAurasDueToSpell(SPELL_WARD_DAMPENED);
            ward->CastSpell(ward, SPELL_COSMETIC_HOLY_WARDS, true);
            ReleaseFleshSpawn(ward);
        }

        if (IsStage(STAGE_RECONSECRATION))
            SendScenarioEvent(GAME_EVENT_DAMPENING_RITUAL_STOPPED);
    }

    // They come out around the pedestal that the ward is on, onto whatever floor is there, and go for the
    // player
    void ReleaseFleshSpawn(Creature const* ward)
    {
        Player* player = GetPlayerToFollow();
        for (uint32 i = 0; i < FleshSpawnPerWard; ++i)
        {
            float angle = float(M_PI) / 4.0f + float(i) * float(M_PI) / 2.0f;
            float x = ward->GetPositionX() + FleshSpawnDistance * std::cos(angle);
            float y = ward->GetPositionY() + FleshSpawnDistance * std::sin(angle);
            Position position(x, y, ward->GetMapHeight(x, y, ward->GetPositionZ()), angle);

            TempSummon* fleshSpawn = instance->SummonCreature(NPC_FLESH_SPAWN, position);
            if (fleshSpawn && player && fleshSpawn->IsValidAttackTarget(player))
                fleshSpawn->AI()->AttackStart(player);
        }
    }

    // With the last ritual stopped, what the rituals fed comes onto the dais: the Amassing Darkness, which
    // sets about its Armageddon. The stage is done when it is gone, by the player's hand or by its own
    // blast.
    void StartDontFearTheReaper()
    {
        TempSummon* darkness = !_amassingDarknessDefeated ? instance->SummonCreature(NPC_AMASSING_DARKNESS, AmassingDarknessPosition) : nullptr;
        if (!darkness)
        {
            SendScenarioEvent(GAME_EVENT_AMASSING_DARKNESS_DEFEATED);
            return;
        }

        _amassingDarknessGuid = darkness->GetGUID();
        darkness->AI()->Talk(EMOTE_AMASSING_DARKNESS_RITUAL_BROKEN);

        _scheduler.Schedule(SlaghammerTrapDelay, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetSlaghammerInTheTomb())
                slaghammer->AI()->Talk(SAY_SLAGHAMMER_TRAP);
        });

        _scheduler.Schedule(AmassingDarknessAwakenDelay, [this](TaskContext /*context*/)
        {
            Creature* amassingDarkness = instance->GetCreature(_amassingDarknessGuid);
            if (amassingDarkness && amassingDarkness->IsAlive())
                amassingDarkness->AI()->DoAction(ACTION_AMASSING_DARKNESS_AWAKEN);
        });
    }

    // The seal in the hall's floor is broken and the stairs under it lead down to the prison. As Slaghammer
    // says so, Borgoth the Master Reaver and a Twilight Bladetwister come up them.
    void StartDarkPassage()
    {
        _scheduler.Schedule(SlaghammerPathOpenDelay, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetSlaghammerInTheTomb())
                slaghammer->AI()->Talk(SAY_SLAGHAMMER_PATH_OPEN);
        });

        Player* player = GetPlayerToFollow();
        auto comeUpTheStairs = [this, player](uint32 entry, Position const& position)
        {
            TempSummon* summon = instance->SummonCreature(entry, position);
            if (summon && player && summon->IsValidAttackTarget(player))
                summon->AI()->AttackStart(player);
        };

        comeUpTheStairs(NPC_BORGOTH_THE_MASTER_REAVER, BorgothPosition);
        comeUpTheStairs(NPC_TWILIGHT_BLADETWISTER, BorgothBladetwisterPosition);
    }

    // On the way down: Slaghammer speaks of the Void Tendrils when the player comes to them, and before
    // the door of the prison chamber a cutscene shows the Deacon at his work in it. The stage is done
    // when the cutscene is over (its script tells the instance).
    void UpdateDarkPassage()
    {
        for (MapReference const& ref : instance->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || player->IsGameMaster() || !player->IsAlive())
                continue;

            if (!_voidTendrilsSpokenOf && player->GetExactDist(VoidTendrilsPosition) < VoidTendrilsWarningDistance)
            {
                _voidTendrilsSpokenOf = true;
                if (Creature* slaghammer = GetSlaghammerInTheTomb())
                    slaghammer->AI()->Talk(SAY_SLAGHAMMER_MIND_GAMES);
            }

            if (_bossRevealStarted || player->GetExactDist(BossRevealPosition) > BossRevealDistance)
                continue;

            _bossRevealStarted = true;
            player->CastSpell(player, SPELL_BOSS_REVEAL_SCENE, true);
            _scheduler.Schedule(BossRevealTimeout, [this](TaskContext /*context*/)
            {
                PrisonChamberReached();
            });
        }
    }

    void PrisonChamberReached()
    {
        if (_prisonChamberReached || !IsStage(STAGE_DARK_PASSAGE))
            return;

        _prisonChamberReached = true;
        SendScenarioEvent(GAME_EVENT_PRISON_CHAMBER_REACHED);
    }

    // The Deacon is trying to bring Zakajz back to life with the blade. Slaghammer hears more of the
    // Twilight's Hammer coming and stays behind to hold them off; the Deacon is the player's. The stage is
    // done when he is dead.
    void StartDeathToTheDeacon()
    {
        if (_farthingDead)
        {
            SendScenarioEvent(GAME_EVENT_TWILIGHT_DEACON_KILLED);
            return;
        }

        _scheduler.Schedule(SlaghammerStayBehindDelay, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetSlaghammerInTheTomb())
            {
                slaghammer->AI()->Talk(SAY_SLAGHAMMER_STAY_BEHIND);
                slaghammer->AI()->DoAction(ACTION_SLAGHAMMER_STAY_BEHIND);
            }
        });
    }

    // The Deacon is all but dead and the blade does not defend him (his script tells the instance). It
    // has left his hand and floats in the chamber; the two speak, heard by the player, and at the
    // Deacon's last plea the blade turns its beam on him.
    void FarthingFailing()
    {
        SummonXalatath();

        for (MapReference const& ref : instance->GetPlayers())
            if (Player* player = ref.GetSource())
                player->CastSpell(player, SPELL_CONVERSATION_DEATH_OF_THE_DEACON, true);

        _scheduler.Schedule(XalatathBeamDelay, [this](TaskContext /*context*/)
        {
            if (Creature* xalatath = instance->GetCreature(_xalatathGuid))
                xalatath->CastSpell(nullptr, SPELL_XALATATH_BEAM, true);
        });
    }

    // With the Deacon dead his work on Zakajz ends, and the dome over Zakajz goes
    void FarthingDead()
    {
        _farthingDead = true;

        if (Creature* zakajz = instance->GetCreatureBySpawnId(ZakajzSpawnId))
            zakajz->RemoveAurasDueToSpell(SPELL_ZAKAJZ_REBIRTH_STATE);

        if (IsStage(STAGE_DEATH_TO_THE_DEACON))
            SendScenarioEvent(GAME_EVENT_TWILIGHT_DEACON_KILLED);
    }

    // The blade as it floats in the prison chamber, a creature that can be neither fought nor chosen
    Creature* SummonXalatath()
    {
        if (Creature* xalatath = instance->GetCreature(_xalatathGuid))
            return xalatath;

        TempSummon* xalatath = instance->SummonCreature(NPC_XALATATH, XalatathPosition);
        if (!xalatath)
            return nullptr;

        _xalatathGuid = xalatath->GetGUID();
        xalatath->SetReactState(REACT_PASSIVE);
        xalatath->SetImmuneToAll(true);
        xalatath->SetUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
        xalatath->SetDisableGravity(true);
        return xalatath;
    }

    // The blade offers itself to the player. It is taken by an object around it, which opens like a chest
    // with the blade in it; the stage is done when a player has the blade. (The object has a condition of
    // its own, PlayerCondition 43717, which the client checks: only a player who does not have the blade
    // yet can use it.)
    // TODO: the circle of blue light that the blade floats over in the video
    void StartTheBladeOfTheBlackEmpire()
    {
        if (_bladeTaken)
        {
            SendScenarioEvent(GAME_EVENT_BLADE_TAKEN);
            return;
        }

        if (Creature* xalatath = SummonXalatath())
            xalatath->SummonGameObject(GO_XALATATH, XalatathObjectPosition, QuaternionData::fromEulerAnglesZYX(XalatathObjectPosition.GetOrientation(), 0.0f, 0.0f), 0s);
    }

    void UpdateTheBladeOfTheBlackEmpire()
    {
        for (MapReference const& ref : instance->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || player->IsGameMaster() || !player->HasItemCount(ITEM_XALATATH))
                continue;

            BladeTaken(player);
            return;
        }
    }

    // The blade is the player's: it is in the player's hand at once (the core does not know the effect of
    // the retail spell that sees to that, "force equip item"), and a cutscene shows the player holding it
    // up. What floated in the chamber is gone.
    void BladeTaken(Player* player)
    {
        if (_bladeTaken)
            return;

        _bladeTaken = true;

        Item* blade = player->GetItemByEntry(ITEM_XALATATH);
        if (blade && !blade->IsEquipped())
            player->SwapItem(blade->GetPos(), (INVENTORY_SLOT_BAG_0 << 8) | EQUIPMENT_SLOT_MAINHAND);

        player->CastSpell(player, SPELL_BLADE_OF_THE_BLACK_EMPIRE, true);

        // The object by which it was taken is the creature's and goes with it
        if (Creature* xalatath = instance->GetCreature(_xalatathGuid))
            xalatath->DespawnOrUnsummon(1ms);

        if (IsStage(STAGE_THE_BLADE_OF_THE_BLACK_EMPIRE))
            SendScenarioEvent(GAME_EVENT_BLADE_TAKEN);
    }

    // Zakajz is dead, but the Twilight's Hammer may try again or he may wake by himself, and the blade
    // wants what is left of him. It tells the player so, and the player gets "Dark Drain" on the extra
    // action button: a channel of ten seconds on Zakajz, whose script tells the instance when it has run
    // its time.
    // TODO: on retail the spell is also offered in the scenario's objectives
    void StartTheTrueDeathOfZakajz()
    {
        if (_zakajzDrained)
        {
            SendScenarioEvent(GAME_EVENT_ZAKAJZ_DRAINED);
            return;
        }

        // He can be chosen and drained now
        if (Creature* zakajz = GetLivingCreature(ZakajzSpawnId))
        {
            zakajz->RemoveUnitFlag(UNIT_FLAG_UNINTERACTIBLE);
            zakajz->SetImmuneToPC(false);
        }

        for (MapReference const& ref : instance->GetPlayers())
            if (Player* player = ref.GetSource())
                player->CastSpell(player, SPELL_DARK_DRAIN_SPELL_BAR, true);

        _scheduler.Schedule(XalatathOnZakajzDelay, [this](TaskContext /*context*/)
        {
            if (_zakajzDrained)
                return;

            for (MapReference const& ref : instance->GetPlayers())
                if (Player* player = ref.GetSource())
                    player->CastSpell(player, SPELL_CONVERSATION_DEATH_TO_ZAKAJZ, true);
        });
    }

    // Zakajz has been drained: he dissolves and is gone for good. That is what the scenario is won by and
    // what the quest asks for ("Stop the Ritual and acquire the Blade"). On retail the spell that the
    // drain ends in ("Dark Drain 2", 201905) sees to all of it; its script does it here, because the core
    // would have Zakajz cast that spell, and a game event counts for a scenario only when a player sends it.
    void ZakajzDrained()
    {
        if (_zakajzDrained)
            return;

        _zakajzDrained = true;

        Creature* zakajz = instance->GetCreatureBySpawnId(ZakajzSpawnId);

        for (MapReference const& ref : instance->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player)
                continue;

            player->RemoveAurasDueToSpell(SPELL_DARK_DRAIN_SPELL_BAR);

            // The quest has the flag "no credit for proxy creatures", with which the core counts a credit
            // only when it is told whose end it was
            player->KilledMonsterCredit(NPC_KILL_CREDIT_WON_SCENARIO, zakajz ? zakajz->GetGUID() : player->GetGUID());
        }

        if (zakajz)
        {
            zakajz->CastSpell(zakajz, SPELL_DRAINED_ZAKAJZ, true);
            zakajz->CastSpell(zakajz, SPELL_COSMETIC_DISSOLVE, true);
            zakajz->DespawnOrUnsummon(ZakajzDissolveTime, ZakajzNoRespawnTime);
        }

        if (IsStage(STAGE_THE_TRUE_DEATH_OF_ZAKAJZ))
            SendScenarioEvent(GAME_EVENT_ZAKAJZ_DRAINED);

        SlaghammerOpensThePortal();
    }

    // The Twilight's Hammer has run, and Slaghammer comes into the chamber: he has a portal stone of
    // theirs and opens the player a portal to Dalaran with it.
    void SlaghammerOpensThePortal()
    {
        _scheduler.Schedule(SlaghammerComesDelay, [this](TaskContext /*context*/)
        {
            Creature* slaghammer = BringSlaghammerToThePrison();
            if (!slaghammer)
                return;

            slaghammer->SetWalk(false);
            slaghammer->GetMotionMaster()->MovePoint(POINT_SLAGHAMMER_YE_DID_IT, SlaghammerYeDidItPosition, true, SlaghammerYeDidItPosition.GetOrientation());
        });

        _scheduler.Schedule(SlaghammerYeDidItDelay, [this](TaskContext /*context*/)
        {
            Creature* slaghammer = GetSlaghammerInTheTomb();
            if (!slaghammer)
                return;

            if (Player* player = GetPlayerToFollow())
                slaghammer->SetFacingToObject(player);

            slaghammer->AI()->Talk(SAY_SLAGHAMMER_YE_DID_IT);
        });

        _scheduler.Schedule(SlaghammerToPortalStoneDelay, [this](TaskContext /*context*/)
        {
            Creature* slaghammer = GetSlaghammerInTheTomb();
            if (!slaghammer)
                return;

            slaghammer->SetWalk(true);
            slaghammer->GetMotionMaster()->MovePoint(POINT_SLAGHAMMER_PORTAL_STONE, SlaghammerPortalStonePosition, true, SlaghammerPortalStonePosition.GetOrientation());
        });

        // TODO: what he casts with the portal stone on retail is not known; he holds a spell meanwhile
        _scheduler.Schedule(SlaghammerPortalStoneDelay, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetSlaghammerInTheTomb())
                slaghammer->SetEmoteState(EMOTE_STATE_SPELL_CHANNEL_OMNI);
        });

        _scheduler.Schedule(TwilightPortalDelay, [this](TaskContext /*context*/)
        {
            Creature* slaghammer = GetSlaghammerInTheTomb();
            if (!slaghammer)
                return;

            slaghammer->SetEmoteState(EMOTE_ONESHOT_NONE);
            slaghammer->SummonGameObject(GO_PORTAL_TO_DALARAN_CITY, TwilightPortalPosition, QuaternionData::fromEulerAnglesZYX(TwilightPortalPosition.GetOrientation(), 0.0f, 0.0f), 0s);
        });

        _scheduler.Schedule(SlaghammerKeepAnEyeDelay, [this](TaskContext /*context*/)
        {
            if (Creature* slaghammer = GetSlaghammerInTheTomb())
                slaghammer->AI()->Talk(SAY_SLAGHAMMER_KEEP_AN_EYE);
        });
    }

    Creature* BringSlaghammerToThePrison()
    {
        Creature* slaghammer = GetSlaghammerInTheTomb();
        if (slaghammer && slaghammer->GetExactDist(PrisonDoorPosition) > SlaghammerComesDistance)
        {
            slaghammer->DespawnOrUnsummon();
            slaghammer = nullptr;
        }

        if (!slaghammer)
        {
            slaghammer = instance->SummonCreature(NPC_SHADOWLORD_SLAGHAMMER_SPY, PrisonDoorPosition);
            if (!slaghammer)
                return nullptr;

            _slaghammerGuid = slaghammer->GetGUID();
        }

        // He goes where the scenario's script sends him, not after the player
        slaghammer->AI()->DoAction(ACTION_SLAGHAMMER_STAY_BEHIND);
        return slaghammer;
    }

    Creature* GetSlaghammerInTheTomb() const
    {
        Creature* slaghammer = instance->GetCreature(_slaghammerGuid);
        return slaghammer && slaghammer->IsAlive() ? slaghammer : nullptr;
    }

    bool HaveDoorGuardsFallen() const
    {
        return _doorGuardsSlain.count(DoorShadowcasterSpawnId) && _doorGuardsSlain.count(DoorBladetwisterSpawnId);
    }

    Creature* GetLivingCreature(ObjectGuid::LowType spawnId) const
    {
        Creature* creature = instance->GetCreatureBySpawnId(spawnId);
        return creature && creature->IsAlive() ? creature : nullptr;
    }

    // The player the scenario is for; one in GM mode only if there is no other
    Player* GetPlayerToFollow() const
    {
        Player* anyPlayer = nullptr;
        for (MapReference const& ref : instance->GetPlayers())
        {
            Player* player = ref.GetSource();
            if (!player || !player->IsAlive())
                continue;

            if (!player->IsGameMaster())
                return player;

            anyPlayer = player;
        }

        return anyPlayer;
    }

    TaskScheduler _scheduler;
    ScenarioStepEntry const* _currentStep;
    uint32 _checkTimer;
    std::set<ObjectGuid::LowType> _doorGuardsSlain;
    bool _barrierBeingTakenDown;
    ObjectGuid _slaghammerGuid;
    std::set<ObjectGuid::LowType> _ritualsStopped;
    ObjectGuid _amassingDarknessGuid;
    bool _amassingDarknessDefeated;
    bool _voidTendrilsSpokenOf;
    bool _bossRevealStarted;
    bool _prisonChamberReached;
    uint32 _farthingsGuardsSlain;
    bool _farthingDead;
    ObjectGuid _xalatathGuid;
    bool _bladeTaken;
    bool _zakajzDrained;
};

std::unique_ptr<TirisfalArtifactScenario> CreateBladeInTwilightScenario(InstanceMap* map)
{
    return std::make_unique<BladeInTwilightScenario>(map);
}

// 101430 - Shadowlord Slaghammer, as he stands with the guards of the tomb's door. The scenario's script
// tells him what to do; here is only what keeps him from spoiling it.
struct npc_shadowlord_slaghammer : public ScriptedAI
{
    npc_shadowlord_slaghammer(Creature* creature) : ScriptedAI(creature) { }

    // The guards take him for one of theirs until he strikes
    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToNPC(true);
    }

    // He does not fall to the guards
    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (damage >= me->GetHealth())
            damage = me->GetHealth() - 1;
    }

    // and stays where the fight ended, instead of going back to where he came from
    void EnterEvadeMode(EvadeReason why) override
    {
        _EnterEvadeMode(why);
    }
};

// 101899 - Shadowlord Slaghammer, as he goes through the tomb with the player: he keeps to the player's
// side, fights what the player fights, and does not fall. Before the prison chamber he stays behind.
struct npc_shadowlord_slaghammer_spy : public ScriptedAI
{
    npc_shadowlord_slaghammer_spy(Creature* creature) : ScriptedAI(creature), _checkTimer(0), _staysBehind(false) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_DEFENSIVE);
        FollowPlayer();
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_SLAGHAMMER_STAY_BEHIND)
            return;

        _staysBehind = true;
        me->SetHomePosition(me->GetPosition());
        if (!me->IsEngaged())
            me->GetMotionMaster()->MoveIdle();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (damage >= me->GetHealth())
            damage = me->GetHealth() - 1;
    }

    // After a fight he goes back to the player, not to where he was, unless he has stayed behind
    void EnterEvadeMode(EvadeReason why) override
    {
        if (!_EnterEvadeMode(why))
            return;

        if (_staysBehind)
            me->GetMotionMaster()->MoveTargetedHome();
        else
            FollowPlayer();
    }

    void UpdateAI(uint32 diff) override
    {
        if (UpdateVictim() || _staysBehind)
            return;

        _checkTimer += diff;
        if (_checkTimer < CheckInterval)
            return;

        _checkTimer = 0;

        // He is led about, and the core would have him give up fights that are far from where he was
        me->SetHomePosition(me->GetPosition());

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid);
        if (!player || me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        {
            FollowPlayer();
            return;
        }

        Unit* enemy = player->getAttackerForHelper();
        if (enemy && me->IsValidAttackTarget(enemy))
            AttackStart(enemy);
    }

private:
    void FollowPlayer()
    {
        Player* player = me->SelectNearestPlayer(SlaghammerPlayerSearchDistance);
        if (!player)
            return;

        _playerGuid = player->GetGUID();
        me->GetMotionMaster()->MoveFollow(player, SlaghammerFollowDistance, ChaseAngle(SlaghammerFollowAngle));
    }

    ObjectGuid _playerGuid;
    uint32 _checkTimer;
    bool _staysBehind;
};

// 101875 - Twilight Ritualist. One stands at each of the four wards in the hall of the tomb and keeps it
// dampened. It does not leave its place for a fight, and throws Shadow Bolts from there.
struct npc_twilight_ritualist : public ScriptedAI
{
    npc_twilight_ritualist(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    void JustAppeared() override
    {
        _scheduler.CancelAll();

        // Its ward may come into the world after it, so it keeps looking
        _scheduler.Schedule(RitualistDampenInterval, [this](TaskContext context)
        {
            DampenTheLight();
            context.Repeat(RitualistDampenInterval);
        });
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _scheduler.Schedule(ShadowBoltFirstDelay, TASK_GROUP_COMBAT, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SHADOW_BOLT);
            context.Repeat(ShadowBoltInterval);
        });
    }

    void Reset() override
    {
        _scheduler.CancelGroup(TASK_GROUP_COMBAT);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
        UpdateVictim();
    }

private:
    // The beam from the ritualist into its ward is an aura on the ward
    void DampenTheLight()
    {
        if (!me->IsAlive() || me->HasUnitState(UNIT_STATE_CASTING))
            return;

        Creature* ward = me->FindNearestCreature(NPC_WARD_OF_TYR, RitualistWardSearchDistance);
        if (!ward || ward->HasAura(SPELL_DAMPEN_THE_LIGHT, me->GetGUID()))
            return;

        DoCastAOE(SPELL_DAMPEN_THE_LIGHT);
    }

    TaskScheduler _scheduler;
};

// 102693 - Amassing Darkness. The scenario's script brings it onto the dais in the hall of the tomb when
// the last ritual is stopped, and wakes it a moment later; then it channels Armageddon, which ends in a
// blast that also ends the darkness. It comes with a part of its health only, and does not gain any.
struct npc_amassing_darkness : public ScriptedAI
{
    npc_amassing_darkness(Creature* creature) : ScriptedAI(creature), _awake(false)
    {
        SetCombatMovement(false);
    }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetImmuneToAll(true);
        me->SetRegenerateHealth(false);
        me->SetHealth(me->CountPctFromMaxHealth(AmassingDarknessHealthPct));
    }

    void DoAction(int32 action) override
    {
        if (action != ACTION_AMASSING_DARKNESS_AWAKEN || _awake)
            return;

        _awake = true;

        // TODO: what it looks like on retail besides its model is not known
        DoCastSelf(SPELL_AMASSING_DARKNESS_AURA, true);
        me->SetImmuneToAll(false);
        me->SetReactState(REACT_AGGRESSIVE);
        Talk(SAY_AMASSING_DARKNESS_ELIMINATE);
        DoZoneInCombat();
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        DoCastSelf(SPELL_ARMAGEDDON);
        _scheduler.Schedule(ArmageddonChannelTime, [this](TaskContext /*context*/)
        {
            DoCastSelf(SPELL_ARMAGEDDON_BLAST, true);
        });
    }

    // Once awake it stays awake, and begins again when the player comes back
    void EnterEvadeMode(EvadeReason why) override
    {
        _scheduler.CancelAll();
        me->InterruptNonMeleeSpells(true);
        if (_EnterEvadeMode(why) && _awake)
            DoCastSelf(SPELL_AMASSING_DARKNESS_AURA, true);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    bool _awake;
};

// 101122 - Twilight Shadowcaster
struct npc_twilight_shadowcaster : public ScriptedAI
{
    npc_twilight_shadowcaster(Creature* creature) : ScriptedAI(creature) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _scheduler.Schedule(ShadowBoltFirstDelay, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SHADOW_BOLT);
            context.Repeat(ShadowBoltInterval);
        });
    }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 101127 - Twilight Bladetwister
struct npc_twilight_bladetwister : public ScriptedAI
{
    npc_twilight_bladetwister(Creature* creature) : ScriptedAI(creature) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _scheduler.Schedule(TwilightWhirlFirstDelay, [this](TaskContext context)
        {
            if (me->IsWithinMeleeRange(me->GetVictim()))
                DoCastSelf(SPELL_TWILIGHT_WHIRL);

            context.Repeat(TwilightWhirlInterval);
        });
    }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 101897 - Borgoth the Master Reaver. He comes up the stairs from the prison, with a Twilight Bladetwister,
// when the way down has opened.
struct npc_borgoth_the_master_reaver : public ScriptedAI
{
    npc_borgoth_the_master_reaver(Creature* creature) : ScriptedAI(creature) { }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _scheduler.Schedule(TwilightStrikeFirstDelay, [this](TaskContext context)
        {
            DoCastVictim(SPELL_TWILIGHT_STRIKE);
            context.Repeat(TwilightStrikeInterval);
        });

        _scheduler.Schedule(EdgeOfTwilightFirstDelay, [this](TaskContext context)
        {
            DoCastSelf(SPELL_EDGE_OF_TWILIGHT);
            context.Repeat(EdgeOfTwilightInterval);
        });
    }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 102927 - Void Tendril, of those at the round chamber on the way down to the prison and of those the
// Deacon calls up in his fight. It stays where it is and flays the mind of whoever comes near. Slaghammer
// calls them the Deacon's mind games and says they can be dispelled: a dispel that reaches one ends it.
struct npc_darkwalk_void_tendril : public ScriptedAI
{
    npc_darkwalk_void_tendril(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _scheduler.Schedule(MindFlayFirstDelay, [this](TaskContext context)
        {
            DoCastVictim(SPELL_MIND_FLAY);
            context.Repeat(MindFlayInterval);
        });
    }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void SpellHit(WorldObject* caster, SpellInfo const* spellInfo) override
    {
        if (!me->IsAlive() || !spellInfo->HasEffect(SPELL_EFFECT_DISPEL))
            return;

        Unit* dispeller = caster->ToUnit();
        Unit::Kill(dispeller ? dispeller : me, me);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// The two Faceless that guard the Deacon fight as one
static void CallTheOtherFacelessGuard(Creature* guard, Unit* enemy)
{
    uint32 otherEntry = guard->GetEntry() == NPC_FACELESS_GUARDIAN ? NPC_FACELESS_CORRUPTER : NPC_FACELESS_GUARDIAN;
    Creature* other = guard->FindNearestCreature(otherEntry, FacelessGuardCallDistance);
    if (other && !other->IsEngaged() && other->IsValidAttackTarget(enemy))
        other->AI()->AttackStart(enemy);
}

// 102015 - Faceless Guardian, one of the Deacon's two guards in the prison chamber
struct npc_faceless_guardian : public ScriptedAI
{
    npc_faceless_guardian(Creature* creature) : ScriptedAI(creature) { }

    void JustEngagedWith(Unit* who) override
    {
        CallTheOtherFacelessGuard(me, who);

        _scheduler.Schedule(ShadowCrashFirstDelay, [this](TaskContext context)
        {
            DoCastVictim(SPELL_SHADOW_CRASH);
            context.Repeat(ShadowCrashInterval);
        });
    }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 101118 - Faceless Corrupter, the other of the Deacon's two guards
struct npc_faceless_corrupter : public ScriptedAI
{
    npc_faceless_corrupter(Creature* creature) : ScriptedAI(creature) { }

    void JustEngagedWith(Unit* who) override
    {
        CallTheOtherFacelessGuard(me, who);

        _scheduler.Schedule(VoidBoltFirstDelay, [this](TaskContext context)
        {
            DoCastVictim(SPELL_VOID_BOLT);
            context.Repeat(VoidBoltInterval);
        });
    }

    void Reset() override
    {
        _scheduler.CancelAll();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
};

// 101148 - Twilight Deacon Farthing. He stands before the dome over Zakajz in the prison chamber and works
// on bringing Zakajz back to life with the blade, Xal'atath, which has told him that it would defend him.
// While his two guards live he pays the player no mind. In his fight he calls up Shadow Images of himself
// and Void Tendrils; when he is all but dead the blade fails him, and it is the blade that ends him.
struct npc_twilight_deacon_farthing : public ScriptedAI
{
    npc_twilight_deacon_farthing(Creature* creature) : ScriptedAI(creature), _summons(creature), _guarded(true), _hallucinated(false),
        _failing(false) { }

    void JustAppeared() override
    {
        DoCastSelf(SPELL_CHANNELING_WITH_BLADE, true);
    }

    void JustReachedHome() override
    {
        DoCastSelf(SPELL_CHANNELING_WITH_BLADE, true);
    }

    void Reset() override
    {
        _scheduler.CancelAll();
        _summons.DespawnAll();
        _hallucinated = false;

        if (_guarded)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetImmuneToPC(true);
        }
    }

    // His guards have fallen (the scenario's script tells him). From now on he fights who attacks him.
    void DoAction(int32 action) override
    {
        if (action != ACTION_FARTHING_GUARDS_FALLEN || !_guarded)
            return;

        _guarded = false;
        me->SetImmuneToPC(false);
        me->SetReactState(REACT_DEFENSIVE);
        Talk(SAY_FARTHING_DEFEND_ME);
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        me->RemoveAurasDueToSpell(SPELL_CHANNELING_WITH_BLADE);
        Talk(SAY_FARTHING_STOP_ME);

        _scheduler.Schedule(VoidBoltFirstDelay, [this](TaskContext context)
        {
            DoCastVictim(SPELL_VOID_BOLT);
            context.Repeat(VoidBoltInterval);
        });
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        if (_failing)
        {
            damage = 0;
            return;
        }

        // He does not die by the player's hand: he is left with the last of his health, and the rest
        // happens when the blow has been dealt
        if (me->HealthBelowPctDamaged(FarthingFailingHealthPct, damage))
        {
            uint64 lastHealth = me->CountPctFromMaxHealth(FarthingFailingHealthPct);
            damage = me->GetHealth() > lastHealth ? uint32(me->GetHealth() - lastHealth) : 0;

            _failing = true;
            _scheduler.CancelAll();
            _scheduler.Schedule(1ms, [this](TaskContext /*context*/)
            {
                Fail();
            });
            return;
        }

        if (!_hallucinated && me->HealthBelowPctDamaged(FarthingHallucinationsHealthPct, damage))
        {
            _hallucinated = true;
            Hallucinate();
        }
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);

        Unit* victim = me->GetVictim();
        if (victim && summon->IsValidAttackTarget(victim))
            summon->AI()->AttackStart(victim);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _summons.Despawn(summon);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (!_failing)
            ScriptedAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(FarthingCorpseTime);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_failing && !UpdateVictim())
            return;

        _scheduler.Update(diff);
    }

private:
    // "You cannot trust your own mind!": two Shadow Images of him, which his spell brings, and Void
    // Tendrils. On retail the tendrils come by a game event that his spell "Hallucinations" (203382) sends.
    void Hallucinate()
    {
        Talk(SAY_FARTHING_MIND);
        DoCastSelf(SPELL_SHADOW_IMAGES, true);

        for (Position const& tendril : FarthingVoidTendrilPositions)
        {
            Position position = tendril;
            position.m_positionZ = me->GetMapHeight(position.GetPositionX(), position.GetPositionY(), position.GetPositionZ());
            me->SummonCreature(NPC_VOID_TENDRIL, position);
        }
    }

    // The blade does not defend him. It leaves his hand, he backs away from the dome, and the two speak
    // (the scenario's script sees to the blade and to what is heard) until the blade's beam dissolves him.
    void Fail()
    {
        _summons.DespawnAll();

        me->InterruptNonMeleeSpells(true);
        me->SetImmuneToAll(true);
        me->SetReactState(REACT_PASSIVE);
        me->CombatStop(true);
        me->SetVirtualItem(0, 0);
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(POINT_FARTHING_LAST_STAND, FarthingLastStandPosition, true, FarthingLastStandPosition.GetOrientation());

        if (InstanceScript* instance = me->GetInstanceScript())
            instance->SetData(DATA_FARTHING_FAILING, 1);

        _scheduler.Schedule(FarthingDissolveDelay, [this](TaskContext /*context*/)
        {
            DoCastSelf(SPELL_COSMETIC_DISSOLVE_FARTHING, true);
        });

        _scheduler.Schedule(FarthingDeathDelay, [this](TaskContext /*context*/)
        {
            me->SetImmuneToAll(false);
            me->KillSelf();
        });
    }

    TaskScheduler _scheduler;
    SummonList _summons;
    bool _guarded;
    bool _hallucinated;
    bool _failing;
};

// 102039 - Zakajz the Corruptor, whom Tyr gave his life to kill: he lies dead in the prison chamber, under
// the dome of the Deacon's ritual. He does nothing and cannot be hurt; in the end the player drains what
// is left of him.
struct npc_zakajz_the_corruptor : public ScriptedAI
{
    npc_zakajz_the_corruptor(Creature* creature) : ScriptedAI(creature) { }

    void JustAppeared() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetAIAnimKitId(ANIM_KIT_DEAD);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage, DamageEffectType /*damageType*/, SpellInfo const* /*spellInfo*/) override
    {
        damage = 0;
    }

    void UpdateAI(uint32 /*diff*/) override { }
};

// 201904 - Dark Drain, the channel by which the player drains Zakajz. Its aura on Zakajz lasts ten seconds
// and at their end sets off "Dark Drain 2" (201905); the scenario's script is told instead (see
// BladeInTwilightScenario::ZakajzDrained). A channel that is broken off does nothing.
class spell_blade_in_twilight_dark_drain : public AuraScript
{
    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        PreventDefaultAction();
    }

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/) const
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        if (InstanceScript* instance = GetTarget()->GetInstanceScript())
            instance->SetData(DATA_ZAKAJZ_DRAINED, 1);
    }

    void Register() override
    {
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_blade_in_twilight_dark_drain::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_blade_in_twilight_dark_drain::AfterRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
    }
};

// 1182 - the cutscene before the prison chamber, which "Boss Reveal Scene: Shadow Priest" (201321) plays:
// the Deacon at his work on Zakajz. The scenario goes on when it is over, however it ends.
class scene_blade_in_twilight_boss_reveal : public SceneScript
{
public:
    scene_blade_in_twilight_boss_reveal() : SceneScript("scene_blade_in_twilight_boss_reveal") { }

    void OnSceneComplete(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        Over(player);
    }

    void OnSceneCancel(Player* player, uint32 /*sceneInstanceID*/, SceneTemplate const* /*sceneTemplate*/) override
    {
        Over(player);
    }

private:
    static void Over(Player* player)
    {
        if (player->GetMapId() != MAP_TIRISFAL_ARTIFACT_SCENARIOS)
            return;

        if (InstanceScript* instance = player->GetInstanceScript())
            instance->SetData(DATA_BOSS_REVEAL_SCENE_OVER, 1);
    }
};

// A priest on "Blade in Twilight" who comes to the Twilight's Hammer's camp at Tyr's Fall is put into the
// scenario where they are. The scenario's map is laid over Eastern Kingdoms (Map.CosmeticParentMapID) and
// has its coordinates, so the client changes to it without a loading screen. What starts that on retail is
// not in the client's data; here it is an area trigger of the server's own around the camp on Eastern
// Kingdoms.
struct areatrigger_blade_in_twilight_entrance : AreaTriggerAI
{
    areatrigger_blade_in_twilight_entrance(AreaTrigger* areatrigger) : AreaTriggerAI(areatrigger) { }

    void OnUnitEnter(Unit* unit) override
    {
        Player* player = unit->ToPlayer();
        if (!player || player->GetQuestStatus(QUEST_BLADE_IN_TWILIGHT) != QUEST_STATUS_INCOMPLETE)
            return;

        // The quest has the flag "no credit for proxy creatures", with which the core counts a credit only
        // when it is told whose end it was. Nobody's is here, so the player is named.
        player->KilledMonsterCredit(NPC_KILL_CREDIT_FLY_TO_SCENARIO, player->GetGUID());
        player->TeleportTo(WorldLocation(MAP_TIRISFAL_ARTIFACT_SCENARIOS, *player), TELE_TO_SEAMLESS);
    }
};

void AddSC_scenario_blade_in_twilight()
{
    RegisterCreatureAI(npc_shadowlord_slaghammer);
    RegisterCreatureAI(npc_shadowlord_slaghammer_spy);
    RegisterCreatureAI(npc_twilight_ritualist);
    RegisterCreatureAI(npc_amassing_darkness);
    RegisterCreatureAI(npc_twilight_shadowcaster);
    RegisterCreatureAI(npc_twilight_bladetwister);
    RegisterCreatureAI(npc_borgoth_the_master_reaver);
    RegisterCreatureAI(npc_darkwalk_void_tendril);
    RegisterCreatureAI(npc_faceless_guardian);
    RegisterCreatureAI(npc_faceless_corrupter);
    RegisterCreatureAI(npc_twilight_deacon_farthing);
    RegisterCreatureAI(npc_zakajz_the_corruptor);
    new scene_blade_in_twilight_boss_reveal();
    RegisterSpellScript(spell_blade_in_twilight_dark_drain);
    RegisterAreaTriggerAI(areatrigger_blade_in_twilight_entrance);
}
