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
#include "CellImpl.h"
#include "Containers.h"
#include "DB2Stores.h"
#include "GameEventSender.h"
#include "GameObject.h"
#include "GridNotifiersImpl.h"
#include "InstanceScenario.h"
#include "InstanceScript.h"
#include "Log.h"
#include "Map.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "PassiveAI.h"
#include "Player.h"
#include "Random.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "TaskScheduler.h"
#include "TemporarySummon.h"
#include "broken_shore.h"

ObjectData const creatureData[] =
{
    { NPC_KING_VARIAN_WRYNN,        DATA_KING_VARIAN_WRYNN        },
    { NPC_HIGHLORD_TIRION_FORDRING, DATA_HIGHLORD_TIRION_FORDRING },
    { NPC_GULDAN,                   DATA_GULDAN                   },
    { NPC_LADY_JAINA_PROUDMOORE,    DATA_LADY_JAINA_PROUDMOORE    },
    { NPC_GENN_GREYMANE,            DATA_GENN_GREYMANE            },
    { 0,                            0                             }  // END
};

// How close a player has to come to King Varian Wrynn to have found him
static constexpr float VarianFoundDistance = 30.0f;

// The same for Highlord Tirion Fordring, who hangs about 27 yards out from the ledge players arrive on
static constexpr float TirionFoundDistance = 40.0f;

// How close to Gul'dan a player dies
static constexpr float GuldanDeathDistance = 5.0f;

// How far into a stage Jaina's bridge of ice appears, and how long after that it starts to form
static constexpr Seconds BridgeSpawnDelay = 3s;
static constexpr Milliseconds BridgeFormDelay = 500ms;

// How far from a Spire of Woe its Anchoring Crystals are at most, and the other way round. They circle it
// about 15 yards out, and the spires stand more than 50 yards apart.
static constexpr float SpireCrystalDistance = 30.0f;

// How long an Anchoring Crystal takes to go round its spire, which is a guess, and how many points its
// circle is made of. The core leaves out the point a crystal starts at, which shows the less the more there are.
static constexpr float CrystalLapTime = 20.0f;
static constexpr uint8 CrystalCircleSteps = 32;

// The core ends a circle after one lap unless it is told how long the circling is to last
static constexpr Milliseconds CrystalCirclingTime = 12h;

// How long the arrows over the Anchoring Crystals stay if none of the crystals is destroyed before that. A
// video of the retail scenario shows them from when Jaina speaks of the crystals, for about this long.
static constexpr Seconds CrystalArrowTime = 60s;

// A Spire of Woe fires a beam at the ground for this long, with a pause of this length between two beams;
// both are guesses from a video of the retail scenario. A beam's target is there a moment before the beam,
// because a client shows nothing at a creature it does not know yet.
static constexpr Milliseconds SpireBeamTime = 3s;
static constexpr Milliseconds SpireBeamPauseMin = 6s;
static constexpr Milliseconds SpireBeamPauseMax = 10s;
static constexpr Milliseconds SpireBeamAimTime = 500ms;

// How far from its spire a beam comes down, and how wide the arc towards the landing is in which it does
// when it has no one to fire at
static constexpr float SpireBeamDistanceMin = 12.0f;
static constexpr float SpireBeamDistanceMax = 40.0f;
static constexpr float SpireBeamArc = 2.0f * float(M_PI) / 3.0f;

// How far the creature at the top of a Spire of Woe is from the spire at most
static constexpr float SpireTopDistance = 40.0f;

// Where players are taken when the Black City has been razed: the edge of the city, facing the gap to the crevasse
Position const BlackCityRazedPosition = { 1410.2959f, 2162.2405f, 21.252392f, 4.92f };

// The scheduled tasks that can be called off
enum BrokenShoreTaskGroups
{
    TASK_GROUP_BEACH                    = 1, // what the leaders say and do on the beach
    TASK_GROUP_CRYSTALS                 = 2, // what they make of the Anchoring Crystals, which is late once a spire has fallen
    TASK_GROUP_CRYSTAL_ARROWS           = 3  // the arrows over the Anchoring Crystals, which go when the first of them is destroyed
};

// "The Battle for Broken Shore". The scenario itself is attached to the map by the `scenarios` table
// and advances when its criteria receive the game events listed in broken_shore.h.
// The stages are being filled in one at a time; those that are still stubs say what they need.
class instance_broken_shore_scenario : public InstanceMapScript
{
public:
    instance_broken_shore_scenario() : InstanceMapScript(BrokenShoreScriptName, MAP_BROKEN_SHORE_SCENARIO) { }

    struct instance_broken_shore_scenario_InstanceMapScript : public InstanceScript
    {
        instance_broken_shore_scenario_InstanceMapScript(InstanceMap* map) : InstanceScript(map), _currentStep(nullptr), _checkTimer(0),
            _spiresDestroyed(0), _allianceForcesCharging(false)
        {
            SetHeaders(DataHeader);
            LoadObjectData(creatureData, nullptr);

            // Where players go when they release after dying; without one the core falls back to the
            // default graveyard in Westfall.
            // TODO: the Horde's locations.
            SetEntranceLocation(WORLD_SAFE_LOC_ALLIANCE_BEACH);
        }

        // The core has no script hook for scenario step changes, so the current step is polled.
        void Update(uint32 diff) override
        {
            _scheduler.Update(diff);

            ResurrectReleasedPlayers();

            InstanceScenario const* scenario = instance->GetInstanceScenario();
            if (!scenario)
                return;

            ScenarioStepEntry const* step = scenario->GetStep();
            if (step != _currentStep)
            {
                if (_currentStep)
                    EndStage(BrokenShoreStages(_currentStep->OrderIndex));

                _currentStep = step;
                if (step)
                    StartStage(BrokenShoreStages(step->OrderIndex));
                else if (scenario->IsComplete())
                    CompleteScenario();
            }

            if (!step)
                return;

            switch (step->OrderIndex)
            {
                case STAGE_THE_BROKEN_SHORE:
                    UpdateTheBrokenShore(diff);
                    break;
                case STAGE_FIND_VARIAN:
                    UpdateFindVarian(diff);
                    break;
                case STAGE_THE_HIGHLORD:
                    UpdateTheHighlord(diff);
                    break;
                case STAGE_STOP_GULDAN:
                    UpdateStopGuldan(diff);
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
                case NPC_FELSTALKER_DREADHOUND:
                case NPC_FELGUARD_LEGIONNAIRE_1:
                case NPC_FELGUARD_LEGIONNAIRE_2:
                case NPC_FELGUARD_LEGIONNAIRE_3:
                    SendScenarioEvent(GAME_EVENT_BEACH_DEMON_SLAIN);
                    break;
                case NPC_FEL_LORD_KURDUZ:
                case NPC_FEL_LORD_RAKKAN:
                case NPC_FEL_LORD_ZARDAK:
                    SendScenarioEvent(GAME_EVENT_FEL_LORD_SLAIN);
                    break;
                case NPC_ANCHORING_CRYSTAL:
                    CrystalDestroyed(creature);
                    break;
                case NPC_DREAD_COMMANDER_ARGANOTH:
                    SendScenarioEvent(GAME_EVENT_ARGANOTH_SLAIN);
                    break;
                case NPC_SHIELDED_ANCHOR:
                    SendScenarioEvent(GAME_EVENT_ANCHOR_SHATTERED);
                    break;
                // The demons of the Black City, by how much health they have
                case NPC_GRINNING_SHADOWSTALKER:
                case NPC_FELFIRE_IMP:
                case NPC_FIERY_TRICKSTER:
                case NPC_SHADOWFLAME_IMP:
                    SendScenarioEvent(GAME_EVENT_BLACK_CITY_1);
                    break;
                case NPC_FELGUARD_INVADER:
                case NPC_LIVING_FELBLAZE:
                case NPC_BURNING_TERRORHOUND:
                case NPC_WRATHGUARD_DREADBLADE:
                    SendScenarioEvent(GAME_EVENT_BLACK_CITY_2);
                    break;
                case NPC_BURNING_SENTRY:
                case NPC_SOULBOUND_DESTRUCTOR:
                case NPC_DARK_WORSHIPPER:
                case NPC_SHADOWSWORN_HARBINGER:
                    SendScenarioEvent(GAME_EVENT_BLACK_CITY_3);
                    break;
                case NPC_INFERNAL_SIEGEBREAKER:
                case NPC_MOTHER_VIRILA:
                case NPC_MOARG_PAINBRINGER:
                case NPC_FEL_LORD_DUKAZ:
                case NPC_FEL_LORD_ZARNOZ:
                case NPC_FEL_LORD_RAKAZ:
                case NPC_MALIFICUS:
                    SendScenarioEvent(GAME_EVENT_BLACK_CITY_4);
                    break;
                case NPC_KROSUS:
                    SendScenarioEvent(GAME_EVENT_KROSUS_SLAIN);
                    break;
                case NPC_MOARG_SPINEBREAKER:
                    SendScenarioEvent(GAME_EVENT_GULDAN_CONFRONTED);
                    break;
                default:
                    break;
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_ALLIANCE_FORCES_CHARGING)
                return _allianceForcesCharging ? 1 : 0;

            return 0;
        }

        void OnGameObjectCreate(GameObject* go) override
        {
            InstanceScript::OnGameObjectCreate(go);

            switch (go->GetEntry())
            {
                // A fallen spire that is loaded again, as happens when players leave the scenario and come
                // back, would stand
                case GO_SPIRE_OF_WOE:
                    _spireGUIDs.push_back(go->GetGUID());
                    if (_fallenSpires.contains(go->GetSpawnId()))
                        go->SetGoState(GO_STATE_DESTROYED);
                    break;
                // A bridge of ice is spawned as a closed door and opened a moment after it appears. The
                // model's animation for a door that opens is the ice forming, and a client only plays it
                // for a door it already has; an open door then shows the finished bridge.
                case GO_ICE_BRIDGE:
                    _scheduler.Schedule(BridgeFormDelay, [this, guid = go->GetGUID()](TaskContext /*context*/)
                    {
                        if (GameObject* bridge = instance->GetGameObject(guid))
                            bridge->SetGoState(GO_STATE_ACTIVE);
                    });
                    break;
                default:
                    break;
            }
        }

        // The core only passes a game event on to the scenario when a player is its source,
        // and never for a player in GM mode.
        void SendScenarioEvent(uint32 gameEventId)
        {
            for (MapReference const& ref : instance->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || player->IsGameMaster())
                    continue;

                GameEvents::Trigger(gameEventId, player, nullptr);
                return;
            }
        }

        // A player who releases is taken to the graveyard of the stage, which is the entrance location,
        // as a ghost. The scenario has no running back to a corpse: the player comes back to life there.
        void ResurrectReleasedPlayers()
        {
            for (MapReference const& ref : instance->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || !player->HasPlayerFlag(PLAYER_FLAGS_GHOST) || player->IsBeingTeleported())
                    continue;

                player->ResurrectPlayer(1.0f);
                player->SpawnCorpseBones();
            }
        }

        // The stages that wait for players to get somewhere look for them once a second
        bool IsTimeToCheckPlayers(uint32 diff)
        {
            if (_checkTimer > diff)
            {
                _checkTimer -= diff;
                return false;
            }

            _checkTimer = 1 * IN_MILLISECONDS;
            return true;
        }

        // Sends the game event once a living player who is not in GM mode is close enough to the creature
        void SendScenarioEventWhenFound(uint32 diff, uint32 creatureDataType, float distance, uint32 gameEventId)
        {
            if (!IsTimeToCheckPlayers(diff))
                return;

            Creature* creature = GetCreature(creatureDataType);
            if (!creature)
                return;

            for (MapReference const& ref : instance->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (!player || player->IsGameMaster() || !player->IsAlive() || !player->IsWithinDist(creature, distance))
                    continue;

                GameEvents::Trigger(gameEventId, player, nullptr);
                return;
            }
        }

        // "For the Alliance!" is given at the start of every stage from the beach on to players who do not
        // have it: one stack, which lasts an hour and is lost on death. The spell could stack, and on retail
        // someone casts it; a video of the retail scenario shows a single stack, and not who that is.
        void GiveForTheAlliance()
        {
            instance->DoOnPlayers([](Player* player)
            {
                if (player->IsAlive() && !player->HasAura(SPELL_FOR_THE_ALLIANCE))
                    player->AddAura(SPELL_FOR_THE_ALLIANCE, player);
            });
        }

        // Has one of the creatures the script keeps track of say a line after a delay
        void TalkLater(Seconds delay, uint32 taskGroup, uint32 creatureDataType, uint8 textGroup)
        {
            _scheduler.Schedule(delay, taskGroup, [this, creatureDataType, textGroup](TaskContext /*context*/)
            {
                if (Creature* creature = GetCreature(creatureDataType))
                    creature->AI()->Talk(textGroup);
            });
        }

        // The Anchoring Crystals that circle a Spire of Woe. A gameobject measures a distance from the edges
        // of its model's box, which lets a search around a spire reach the crystals of the next one, so
        // every crystal found is measured again from the spire's centre.
        std::vector<Creature*> GetCrystals(GameObject const* spire) const
        {
            std::vector<Creature*> crystals;
            spire->GetCreatureListWithEntryInGrid(crystals, NPC_ANCHORING_CRYSTAL, SpireCrystalDistance);
            std::erase_if(crystals, [spire](Creature const* crystal) { return spire->GetExactDist2d(crystal) > SpireCrystalDistance; });
            return crystals;
        }

        // Puts an arrow over every Anchoring Crystal, or takes the arrows away again
        void ShowCrystalArrows(bool show)
        {
            for (ObjectGuid const& guid : _spireGUIDs)
            {
                GameObject* spire = instance->GetGameObject(guid);
                if (!spire)
                    continue;

                for (Creature* crystal : GetCrystals(spire))
                {
                    if (!show)
                        crystal->RemoveAurasDueToSpell(SPELL_GREEN_CAT_MARK_STATE);
                    else if (crystal->IsAlive())
                        crystal->AddAura(SPELL_GREEN_CAT_MARK_STATE, crystal);
                }
            }
        }

        // A Spire of Woe is held in place by the Anchoring Crystals that circle it and falls with the last
        // of them: its model has an animation of that, which a client plays for a destroyed gameobject.
        // A crystal without a spire counts as one, so that the stage can still be finished.
        void CrystalDestroyed(Creature* crystal)
        {
            _scheduler.CancelGroup(TASK_GROUP_CRYSTAL_ARROWS);
            ShowCrystalArrows(false);

            if (GameObject* spire = crystal->FindNearestGameObject(GO_SPIRE_OF_WOE, SpireCrystalDistance))
            {
                for (Creature const* other : GetCrystals(spire))
                    if (other != crystal && other->IsAlive())
                        return;

                DestroySpire(spire);
            }

            SendScenarioEvent(GAME_EVENT_SPIRE_OF_WOE_DESTROYED);
            SpireDestroyed();
        }

        void DestroySpire(GameObject* spire)
        {
            _fallenSpires.insert(spire->GetSpawnId());
            spire->SetGoState(GO_STATE_DESTROYED);
        }

        // What the leaders say as the Spires of Woe fall
        void SpireDestroyed()
        {
            _scheduler.CancelGroup(TASK_GROUP_CRYSTALS);

            switch (++_spiresDestroyed)
            {
                case 1:
                    TalkLater(1s, TASK_GROUP_BEACH, DATA_LADY_JAINA_PROUDMOORE, SAY_JAINA_IT_WORKED);
                    TalkLater(3s, TASK_GROUP_BEACH, DATA_GENN_GREYMANE, SAY_GENN_ONE_DOWN);
                    break;
                case 2:
                    TalkLater(1s, TASK_GROUP_BEACH, DATA_GENN_GREYMANE, SAY_GENN_ONE_MORE);
                    break;
                default:
                    break;
            }
        }

        // TODO: Jaina, who makes the bridges; for now they form by themselves.
        void SpawnBridge(uint32 spawnGroupId)
        {
            _scheduler.Schedule(BridgeSpawnDelay, [this, spawnGroupId](TaskContext /*context*/)
            {
                instance->SpawnGroupSpawn(spawnGroupId, true);
            });
        }

        void StartStage(BrokenShoreStages stage)
        {
            TC_LOG_DEBUG("scripts", "Broken Shore scenario: stage {} started (instance {})", uint32(stage), instance->GetInstanceId());

            if (stage >= STAGE_STORM_THE_BEACH)
                GiveForTheAlliance();

            switch (stage)
            {
                case STAGE_THE_BROKEN_SHORE:
                    StartTheBrokenShore();
                    break;
                case STAGE_STORM_THE_BEACH:
                    StartStormTheBeach();
                    break;
                case STAGE_DEFEAT_THE_COMMANDER:
                    StartDefeatTheCommander();
                    break;
                case STAGE_FIND_VARIAN:
                    StartFindVarian();
                    break;
                case STAGE_DESTROY_THE_PORTAL:
                    StartDestroyThePortal();
                    break;
                case STAGE_RAZE_THE_BLACK_CITY:
                    StartRazeTheBlackCity();
                    break;
                case STAGE_THE_HIGHLORD:
                    StartTheHighlord();
                    break;
                case STAGE_KROSUS:
                    StartKrosus();
                    break;
                case STAGE_STOP_GULDAN:
                    StartStopGuldan();
                    break;
                default:
                    break;
            }
        }

        void EndStage(BrokenShoreStages stage)
        {
            switch (stage)
            {
                // The Alliance's forces stay where they are. TODO: what they do in the stages that follow.
                case STAGE_STORM_THE_BEACH:
                    instance->SpawnGroupDespawn(SPAWN_GROUP_STORM_THE_BEACH, true);
                    _scheduler.CancelGroup(TASK_GROUP_BEACH);
                    _scheduler.CancelGroup(TASK_GROUP_CRYSTALS);
                    _scheduler.CancelGroup(TASK_GROUP_CRYSTAL_ARROWS);
                    _allianceForcesCharging = false;

                    // A spire that still stands, which takes a GM skipping the stage, falls with the rest
                    for (ObjectGuid const& guid : _spireGUIDs)
                        if (GameObject* spire = instance->GetGameObject(guid))
                            if (spire->GetGoState() == GO_STATE_READY)
                                DestroySpire(spire);
                    break;
                // His death ends the stage, so only respawning is stopped and his corpse stays
                case STAGE_DEFEAT_THE_COMMANDER:
                    instance->SetSpawnGroupInactive(SPAWN_GROUP_DEFEAT_THE_COMMANDER);
                    break;
                case STAGE_DESTROY_THE_PORTAL:
                    instance->SpawnGroupDespawn(SPAWN_GROUP_DESTROY_THE_PORTAL, true);
                    break;
                case STAGE_RAZE_THE_BLACK_CITY:
                    instance->SpawnGroupDespawn(SPAWN_GROUP_RAZE_THE_BLACK_CITY, true);
                    break;
                // As with the commander, his death ends the stage and his corpse stays
                case STAGE_KROSUS:
                    instance->SetSpawnGroupInactive(SPAWN_GROUP_KROSUS);
                    break;
                // The same goes for the Mo'arg Spinebreaker, and Gul'dan stays as well
                case STAGE_STOP_GULDAN:
                    instance->SetSpawnGroupInactive(SPAWN_GROUP_STOP_GULDAN);
                    break;
                default:
                    break;
            }
        }

        // "Travel to the Broken Shore." Needs GAME_EVENT_ARRIVED_AT_BROKEN_SHORE.
        // TODO: the faction ship and its arrival; players currently land straight on the beach.
        void StartTheBrokenShore() { }

        // Until there is a ship to arrive on, a player being on the map counts as having arrived
        void UpdateTheBrokenShore(uint32 diff)
        {
            if (IsTimeToCheckPlayers(diff))
                SendScenarioEvent(GAME_EVENT_ARRIVED_AT_BROKEN_SHORE);
        }

        // "Destroy all demons and structures on the beach." Needs 33 GAME_EVENT_BEACH_DEMON_SLAIN,
        // 3 GAME_EVENT_FEL_LORD_SLAIN and 3 GAME_EVENT_SPIRE_OF_WOE_DESTROYED, sent from OnUnitDeath.
        // Jaina and Genn greet the players and then charge the spires with the forces they landed with.
        // When they say what is a guess at retail's pacing.
        // TODO: the Alliance's cannons, and the demons' abilities.
        // The beach demons respawn in place of the reinforcements that should keep arriving.
        void StartStormTheBeach()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_SPIRES_OF_WOE, true);
            instance->SpawnGroupSpawn(SPAWN_GROUP_STORM_THE_BEACH, true);
            instance->SpawnGroupSpawn(SPAWN_GROUP_ALLIANCE_LEADERS, true);
            instance->SpawnGroupSpawn(SPAWN_GROUP_ALLIANCE_LANDING_FORCE, true);

            TalkLater(2s, TASK_GROUP_BEACH, DATA_LADY_JAINA_PROUDMOORE, SAY_JAINA_REINFORCEMENTS);
            TalkLater(5s, TASK_GROUP_BEACH, DATA_GENN_GREYMANE, SAY_GENN_JUST_IN_TIME);
            TalkLater(11s, TASK_GROUP_BEACH, DATA_LADY_JAINA_PROUDMOORE, SAY_JAINA_NOW_OR_NEVER);
            TalkLater(14s, TASK_GROUP_BEACH, DATA_GENN_GREYMANE, SAY_GENN_CHARGE);
            _scheduler.Schedule(22s, TASK_GROUP_BEACH, [this](TaskContext /*context*/)
            {
                _allianceForcesCharging = true;
            });

            // By then they are among the spires
            TalkLater(36s, TASK_GROUP_CRYSTALS, DATA_LADY_JAINA_PROUDMOORE, SAY_JAINA_CRYSTALS);
            TalkLater(43s, TASK_GROUP_CRYSTALS, DATA_GENN_GREYMANE, SAY_GENN_HOPE_YOU_ARE_RIGHT);

            // As Jaina speaks of the crystals, arrows point them out for a while
            _scheduler.Schedule(36s, TASK_GROUP_CRYSTAL_ARROWS, [this](TaskContext /*context*/)
            {
                ShowCrystalArrows(true);
            });
            _scheduler.Schedule(36s + CrystalArrowTime, TASK_GROUP_CRYSTAL_ARROWS, [this](TaskContext /*context*/)
            {
                ShowCrystalArrows(false);
            });
        }

        // "Slay Dread Commander Arganoth." Needs GAME_EVENT_ARGANOTH_SLAIN, sent from OnUnitDeath.
        // TODO: his arrival, lines and abilities, and Fel Commander Azgalor for the Horde.
        void StartDefeatTheCommander()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_DEFEAT_THE_COMMANDER, true);
        }

        // "Locate King Varian Wrynn." Needs GAME_EVENT_VARIAN_FOUND, sent when a player reaches him.
        // He stays where he is afterwards, because the next stage opens with him.
        // TODO: Genn and Jaina leading the way over the hill, the cutscene at its crest, the bombardment
        // on the way down, the forces fighting at Varian's side and the Horde's "Find The Others".
        void StartFindVarian()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_FIND_VARIAN, true);

            // Players who release from here on go to the graveyard at Varian's camp
            SetEntranceLocation(WORLD_SAFE_LOC_ALLIANCE_PORTAL);
        }

        void UpdateFindVarian(uint32 diff)
        {
            SendScenarioEventWhenFound(diff, DATA_KING_VARIAN_WRYNN, VarianFoundDistance, GAME_EVENT_VARIAN_FOUND);
        }

        // "Destroy the demon portal to stop reinforcements." Needs 4 GAME_EVENT_ANCHOR_SHATTERED,
        // sent from OnUnitDeath.
        // TODO: the portal itself, the shield an anchor has until its guard dies, and Varian's forces.
        void StartDestroyThePortal()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_DESTROY_THE_PORTAL, true);
        }

        // "Assault the demon city." A progress bar: 300 points from GAME_EVENT_BLACK_CITY_1 to _4,
        // worth 1, 2, 5 and 10 points each and sent from OnUnitDeath. The city's demons respawn,
        // because killing each of them once does not fill the bar. The Unattended Cannons that players
        // can fight from are spawned with them and need no scripting.
        // TODO: the city's Anchoring Crystals and Varian's forces.
        void StartRazeTheBlackCity()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_RAZE_THE_BLACK_CITY, true);

            // Players who release from here on go to the graveyard at the edge of the city
            SetEntranceLocation(WORLD_SAFE_LOC_ALLIANCE_CITY);
        }

        // "Get to Tirion." Needs GAME_EVENT_TIRION_FOUND, sent when a player reaches the ledge he hangs
        // in front of. He stays where he is until the next stage starts.
        // Players start the stage together at the edge of the city, where a bridge of ice forms across the
        // gap to the crevasse.
        // TODO: the cinematic that plays before players are moved, Gul'dan and what he says, Tirion's
        // chains, the demons on the way from the city and Varian's forces.
        void StartTheHighlord()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_THE_HIGHLORD, true);
            SpawnBridge(SPAWN_GROUP_BRIDGE_TO_THE_HIGHLORD);

            // Players who release from here on go to the graveyard at the crevasse
            SetEntranceLocation(WORLD_SAFE_LOC_ALLIANCE_CREVASSE);

            instance->DoOnPlayers([](Player* player)
            {
                player->NearTeleportTo(BlackCityRazedPosition);
            });
        }

        void UpdateTheHighlord(uint32 diff)
        {
            SendScenarioEventWhenFound(diff, DATA_HIGHLORD_TIRION_FORDRING, TirionFoundDistance, GAME_EVENT_TIRION_FOUND);
        }

        // "Kill Krosus." Needs GAME_EVENT_KROSUS_SLAIN, sent from OnUnitDeath.
        // Tirion goes when Krosus comes: Gul'dan drops him into the pool that Krosus rises from.
        // TODO: Krosus rising from the pool, his abilities, what is said and the faction leaders.
        void StartKrosus()
        {
            instance->SpawnGroupDespawn(SPAWN_GROUP_THE_HIGHLORD, true);
            instance->SpawnGroupSpawn(SPAWN_GROUP_KROSUS, true);
        }

        // "Stop Gul'dan from summoning the Legion." Needs GAME_EVENT_GULDAN_CONFRONTED, sent from
        // OnUnitDeath when the Mo'arg Spinebreaker dies. It is the last stage of the scenario.
        // A bridge of ice forms from the ledge where Krosus was fought to the path before the tomb.
        // TODO: the demons Gul'dan summons, what is said, the Alliance being surrounded and the Horde
        // on the ridge.
        void StartStopGuldan()
        {
            instance->SpawnGroupSpawn(SPAWN_GROUP_STOP_GULDAN, true);
            SpawnBridge(SPAWN_GROUP_BRIDGE_TO_GULDAN);

            // Players who release from here on go to the graveyard before the tomb
            SetEntranceLocation(WORLD_SAFE_LOC_ALLIANCE_TOMB);
        }

        // Nobody gets to Gul'dan: a player who comes that close to him dies
        void UpdateStopGuldan(uint32 diff)
        {
            if (!IsTimeToCheckPlayers(diff))
                return;

            Creature* guldan = GetCreature(DATA_GULDAN);
            if (!guldan)
                return;

            for (MapReference const& ref : instance->GetPlayers())
            {
                Player* player = ref.GetSource();
                if (player && !player->IsGameMaster() && player->IsAlive() && player->GetExactDist(guldan) <= GuldanDeathDistance)
                    Unit::Kill(guldan, player);
            }
        }

        // TODO: the finale cinematic and the return to Stormwind.
        void CompleteScenario()
        {
            instance->DoOnPlayers([](Player* player)
            {
                player->KilledMonsterCredit(NPC_FINALE_KILL_CREDIT);
            });
        }

    private:
        ScenarioStepEntry const* _currentStep;
        uint32 _checkTimer;
        TaskScheduler _scheduler;
        uint8 _spiresDestroyed;
        bool _allianceForcesCharging;
        GuidVector _spireGUIDs;
        std::unordered_set<ObjectGuid::LowType> _fallenSpires;
    };

    InstanceScript* GetInstanceScript(InstanceMap* map) const override
    {
        return new instance_broken_shore_scenario_InstanceMapScript(map);
    }
};

// 108920 - Captain Angelica
struct npc_captain_angelica_broken_shore : public ScriptedAI
{
    npc_captain_angelica_broken_shore(Creature* creature) : ScriptedAI(creature) { }

    // Her gossip menu has a single option, shown while the quest is in the log
    bool OnGossipSelect(Player* player, uint32 /*menuId*/, uint32 /*gossipListId*/) override
    {
        CloseGossipMenuFor(player);
        if (player->GetQuestStatus(QUEST_THE_BATTLE_FOR_BROKEN_SHORE) != QUEST_STATUS_INCOMPLETE)
            return true;

        // TODO: the ship ride to the Broken Shore; for now players are placed on the beach.
        player->KilledMonsterCredit(NPC_CAPTAIN_ANGELICA);
        player->TeleportTo(sWorldSafeLocsStore.AssertEntry(WORLD_SAFE_LOC_ALLIANCE_BEACH));
        return true;
    }
};

// 91704 - Anchoring Crystal
// Circles the Spire of Woe it is spawned at, at the distance and height it is spawned at, and does nothing
// else. It floats (CREATURE_STATIC_FLAG_FLOATING), which makes the circle a level one. For a creature on the
// ground the core looks up the ground under every point of the circle, finds none where that is more than
// half a yard above the height it is given, and sends the creature down through the beach.
struct npc_broken_shore_anchoring_crystal : public NullCreatureAI
{
    npc_broken_shore_anchoring_crystal(Creature* creature) : NullCreatureAI(creature), _radius(0.0f) { }

    // The circling is started here and not when the crystal appears, because its spire may appear after it,
    // and started again when something has stopped it
    void UpdateAI(uint32 /*diff*/) override
    {
        if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE
            || me->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING))
            return;

        if (!_radius)
        {
            GameObject* spire = me->FindNearestGameObject(GO_SPIRE_OF_WOE, SpireCrystalDistance);
            if (!spire)
                return;

            _circleCentre.Relocate(spire->GetPositionX(), spire->GetPositionY(), me->GetPositionZ());
            _radius = me->GetExactDist2d(spire);
        }

        me->GetMotionMaster()->MoveCirclePath(_circleCentre.GetPositionX(), _circleCentre.GetPositionY(), _circleCentre.GetPositionZ(), _radius, false,
            CrystalCircleSteps, CrystalCirclingTime, 2.0f * float(M_PI) * _radius / CrystalLapTime);
    }

private:
    Position _circleCentre;
    float _radius;
};

// 97624 - Spire of Woe
// The creature at the top of a Spire of Woe, which itself is a gameobject. It fires the spire's beam at the
// ground under one of those who storm the beach, or towards the landing when none of them is near.
// TODO: on retail the end of a beam wanders about on the ground and leaves fire there, which burns those
// who stand in it; here it stays where it is and is for show.
struct npc_broken_shore_spire_of_woe : public NullCreatureAI
{
    npc_broken_shore_spire_of_woe(Creature* creature) : NullCreatureAI(creature) { }

    void JustAppeared() override
    {
        _scheduler.Schedule(randtime(SpireBeamPauseMin, SpireBeamPauseMax), [this](TaskContext context)
        {
            GameObject const* spire = GetSpire();
            if (spire && spire->GetGoState() == GO_STATE_READY)
                Fire(spire);

            context.Repeat(SpireBeamAimTime + SpireBeamTime + randtime(SpireBeamPauseMin, SpireBeamPauseMax));
        });

        // A spire that has fallen fires no more
        _scheduler.Schedule(500ms, [this](TaskContext context)
        {
            GameObject const* spire = GetSpire();
            if (!spire || spire->GetGoState() == GO_STATE_READY)
            {
                context.Repeat();
                return;
            }

            if (Creature* target = ObjectAccessor::GetCreature(*me, _targetGUID))
                target->DespawnOrUnsummon();

            me->DespawnOrUnsummon();
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    GameObject* GetSpire() const
    {
        return me->FindNearestGameObject(GO_SPIRE_OF_WOE, SpireTopDistance);
    }

    void Fire(GameObject const* spire)
    {
        TempSummon* target = me->SummonCreature(NPC_SPIRE_OF_WOE_BEAM_TARGET, GetGround(spire), TEMPSUMMON_TIMED_DESPAWN, SpireBeamAimTime + SpireBeamTime);
        if (!target)
            return;

        _targetGUID = target->GetGUID();
        _scheduler.Schedule(SpireBeamAimTime, [this](TaskContext /*context*/)
        {
            Creature* target = ObjectAccessor::GetCreature(*me, _targetGUID);
            if (!target)
                return;

            target->SendPlaySpellVisualKit(SPELL_VISUAL_KIT_SPIRE_OF_WOE_FIRE, 0, uint32(SpireBeamTime.count()));
            DoCastAOE(SPELL_FEL_BEAM);
        });
    }

    // Where the next beam comes down
    Position GetGround(GameObject const* spire) const
    {
        Map::PlayerList const& players = me->GetMap()->GetPlayers();
        if (!players.empty())
        {
            // Those who storm the beach are the players and whoever is on their side
            std::vector<Unit*> units;
            Trinity::AnyFriendlyUnitInObjectRangeCheck check(me, players.begin()->GetSource(), SpireTopDistance + SpireBeamDistanceMax);
            Trinity::UnitListSearcher<Trinity::AnyFriendlyUnitInObjectRangeCheck> searcher(me, units, check);
            Cell::VisitAllObjects(me, searcher, SpireTopDistance + SpireBeamDistanceMax);
            std::erase_if(units, [spire](Unit const* unit)
            {
                float distance = spire->GetExactDist2d(unit);
                return distance < SpireBeamDistanceMin || distance > SpireBeamDistanceMax;
            });

            if (!units.empty())
                return Trinity::Containers::SelectRandomContainerElement(units)->GetPosition();
        }

        WorldSafeLocsEntry const* landing = sWorldSafeLocsStore.AssertEntry(WORLD_SAFE_LOC_ALLIANCE_BEACH);
        float angle = spire->GetAbsoluteAngle(landing->Loc.X, landing->Loc.Y) + frand(-SpireBeamArc / 2.0f, SpireBeamArc / 2.0f);
        float distance = frand(SpireBeamDistanceMin, SpireBeamDistanceMax);
        float x = spire->GetPositionX() + distance * std::cos(angle);
        float y = spire->GetPositionY() + distance * std::sin(angle);
        return { x, y, me->GetMapHeight(x, y, me->GetPositionZ()) };
    }

    TaskScheduler _scheduler;
    ObjectGuid _targetGUID;
};

// 192664 - Before We're Overrun: Fel Beam
// The spell takes every creature around its caster that its conditions let through. A Spire of Woe only
// fires at the target it has summoned, and not at that of the next spire.
class spell_broken_shore_fel_beam : public SpellScript
{
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        Unit const* caster = GetCaster();
        if (caster->GetEntry() != NPC_SPIRE_OF_WOE)
            return;

        targets.remove_if([caster](WorldObject const* target)
        {
            Creature const* creature = target->ToCreature();
            return !creature || !creature->IsSummon() || creature->ToTempSummon()->GetSummonerGUID() != caster->GetGUID();
        });
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_broken_shore_fel_beam::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
    }
};

void AddSC_scenario_broken_shore()
{
    new instance_broken_shore_scenario();
    RegisterCreatureAI(npc_captain_angelica_broken_shore);
    RegisterBrokenShoreCreatureAI(npc_broken_shore_anchoring_crystal);
    RegisterBrokenShoreCreatureAI(npc_broken_shore_spire_of_woe);
    RegisterSpellScript(spell_broken_shore_fel_beam);
}
