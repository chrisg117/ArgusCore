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

#ifndef DEF_BROKEN_SHORE_H_
#define DEF_BROKEN_SHORE_H_

#include "CreatureAIImpl.h"

#define DataHeader "BrokenShoreScenario"
#define BrokenShoreScriptName "instance_broken_shore_scenario"

enum BrokenShoreMisc
{
    MAP_BROKEN_SHORE_SCENARIO           = 1460,

    // Scenario.db2, both named "The Battle for Broken Shore"
    SCENARIO_BROKEN_SHORE_ALLIANCE      = 786,
    SCENARIO_BROKEN_SHORE_HORDE         = 1189,

    QUEST_THE_BATTLE_FOR_BROKEN_SHORE   = 42740
};

// ScenarioStep.OrderIndex. Both factions' scenarios have nine steps in this order;
// the Horde names differ for steps 3 ("Find The Others") and 8 ("Hold The Ridge").
enum BrokenShoreStages : uint8
{
    STAGE_THE_BROKEN_SHORE              = 0,
    STAGE_STORM_THE_BEACH               = 1,
    STAGE_DEFEAT_THE_COMMANDER          = 2,
    STAGE_FIND_VARIAN                   = 3,
    STAGE_DESTROY_THE_PORTAL            = 4,
    STAGE_RAZE_THE_BLACK_CITY           = 5,
    STAGE_THE_HIGHLORD                  = 6,
    STAGE_KROSUS                        = 7,
    STAGE_STOP_GULDAN                   = 8
};

// Game events counted by the Alliance scenario's criteria (CriteriaType::AnyoneTriggerGameEventScenario).
// The amount each step needs is noted where it is more than one.
enum BrokenShoreGameEvents
{
    GAME_EVENT_ARRIVED_AT_BROKEN_SHORE  = 44060,
    GAME_EVENT_BEACH_DEMON_SLAIN        = 44095, // 33
    GAME_EVENT_FEL_LORD_SLAIN           = 52643, // 3
    GAME_EVENT_SPIRE_OF_WOE_DESTROYED   = 44077, // 3
    GAME_EVENT_ARGANOTH_SLAIN           = 45131,
    GAME_EVENT_VARIAN_FOUND             = 45228,
    GAME_EVENT_ANCHOR_SHATTERED         = 45288, // 4
    // "Raze the Black City" is a progress bar ("Black City razed") that fills at 300 points.
    // Its four criteria have no description in the client; each event is worth the points noted
    // and is sent for a demon slain in the city, the stronger the demon the more points.
    GAME_EVENT_BLACK_CITY_1             = 44384, // 1 point
    GAME_EVENT_BLACK_CITY_2             = 53062, // 2 points
    GAME_EVENT_BLACK_CITY_3             = 53063, // 5 points
    GAME_EVENT_BLACK_CITY_4             = 53064, // 10 points
    GAME_EVENT_TIRION_FOUND             = 50027,
    GAME_EVENT_KROSUS_SLAIN             = 44669,
    GAME_EVENT_GULDAN_CONFRONTED        = 44826
};

enum BrokenShoreCreatureIds
{
    // The Alliance's leaders, who fight alongside players
    NPC_LADY_JAINA_PROUDMOORE           = 90714,
    NPC_GENN_GREYMANE                   = 90717,

    // Gelbin Mekkatorque and the forces that are with King Varian Wrynn, along with more of those that land
    NPC_GELBIN_MEKKATORQUE              = 90716,
    NPC_ALLIANCE_PRIEST                 = 92074,
    NPC_GNOMEREGAN_TINKERER             = 92122,
    NPC_STORMWIND_GUARD                 = 92123,

    // Raze the Black City: what players can drive and fire through the city
    NPC_UNATTENDED_CANNON               = 100959,

    // Raze the Black City: caged before the city's gate and in the city, and dead on the fence at the gate
    NPC_ARGENT_DAWNBRINGER              = 110615,

    // The forces that land with them
    NPC_KIRIN_TOR_BATTLE_MAGE_1         = 91353,
    NPC_KIRIN_TOR_BATTLE_MAGE_2         = 97496,
    NPC_GILNEAN_ROYAL_GUARD             = 97486,
    NPC_GILNEAN_DRUID                   = 110627,
    NPC_DARNASSUS_SENTINEL              = 114466,

    // Storm the Beach (Alliance)
    NPC_FELSTALKER_DREADHOUND           = 90686,
    NPC_FEL_LORD_KURDUZ                 = 91588,
    NPC_ANCHORING_CRYSTAL               = 91704, // circles a Spire of Woe; three of them hold it in place
    NPC_SPIRE_OF_WOE                    = 97624, // at the top of a Spire of Woe, where its beam comes from
    NPC_SPIRE_OF_WOE_BEAM_TARGET        = 97629, // of the same name; on the ground, where a beam comes down
    NPC_FEL_LORD_RAKKAN                 = 109586,
    NPC_FEL_LORD_ZARDAK                 = 109587,
    NPC_FELGUARD_LEGIONNAIRE_1          = 109591,
    NPC_FELGUARD_LEGIONNAIRE_2          = 109592,
    NPC_FELGUARD_LEGIONNAIRE_3          = 109604,

    // Defeat the Commander (Alliance)
    NPC_DREAD_COMMANDER_ARGANOTH        = 90705,
    NPC_FELBLAZE_INFERNAL               = 93060, // called down by him

    // Find Varian (Alliance)
    NPC_KING_VARIAN_WRYNN               = 90713,
    NPC_FEL_METEOR_STALKER              = 54020, // "General Purpose Bunny JMF (Look 2 - Flying, Huge AOI)"

    // Destroy the Portal (Alliance)
    NPC_EREDAR_CHAOS_GUARD              = 90525, // one at each Shielded Anchor, which it shields
    NPC_MOARG_PAINBRINGER_PORTAL        = 92564, // comes out of the portal
    NPC_SHIELDED_ANCHOR                 = 101667,

    // Raze the Black City (Alliance)
    NPC_INFERNAL_SIEGEBREAKER           = 91967,
    NPC_FELGUARD_INVADER                = 91970,
    NPC_LIVING_FELBLAZE                 = 94189,
    NPC_BURNING_SENTRY                  = 94190,
    NPC_BURNING_TERRORHOUND             = 94191,
    NPC_SOULBOUND_DESTRUCTOR            = 97510,
    NPC_MOTHER_VIRILA                   = 100621,
    NPC_MOARG_PAINBRINGER               = 102701,
    NPC_WRATHGUARD_DREADBLADE           = 102702,
    NPC_FEL_LORD_DUKAZ                  = 102703,
    NPC_FEL_LORD_ZARNOZ                 = 102704,
    NPC_FEL_LORD_RAKAZ                  = 102705,
    NPC_GRINNING_SHADOWSTALKER          = 102706,
    NPC_FELFIRE_IMP                     = 103896,
    NPC_FIERY_TRICKSTER                 = 103897,
    NPC_SHADOWFLAME_IMP                 = 103899,
    NPC_MALIFICUS                       = 110614,
    NPC_DARK_WORSHIPPER                 = 110616,
    NPC_SHADOWSWORN_HARBINGER           = 110617,

    // The Highlord (Alliance). The Horde's leaders watch from a ledge of their own across the pool.
    NPC_HIGHLORD_TIRION_FORDRING        = 91951,
    NPC_LADY_SYLVANAS_WINDRUNNER        = 90709,
    NPC_THRALL                          = 90711,
    NPC_BAINE_BLOODHOOF                 = 90710,
    NPC_VOLJIN                          = 90708,

    // Krosus (Alliance)
    NPC_KROSUS                          = 90544,

    // Stop Gul'dan (Alliance). The demons he brings before him to be fought have templates of their own,
    // made together; those of the Legion that comes after them were made with the scenario's other
    // creatures. Which of them stand there is from a video of the retail scenario, as far as it shows.
    NPC_GULDAN                          = 94276,
    NPC_FELSTALKER_DREADHOUND_TOMB      = 105199,
    NPC_FELGUARD_INVADER_TOMB           = 105203,
    NPC_MOARG_SPINEBREAKER              = 105205,
    NPC_WRATHGUARD_DREADBLADE_TOMB      = 105206,
    NPC_IMP_MOTHER_FECUNDA              = 90662,
    NPC_TICHONDRIUS_THE_DARKENER        = 90688,
    NPC_JAILER_ZERUS                    = 91672,
    NPC_BRUTALLUS                       = 91902,
    NPC_FEL_COMMANDER_AZGALOR           = 93719,

    NPC_FINALE_KILL_CREDIT              = 90918, // quest objective "Broken Shore assaulted"
    NPC_CAPTAIN_ANGELICA                = 108920 // Stormwind Harbor, quest objective "Ship taken to the Broken Shore"
};

enum BrokenShoreGameObjectIds
{
    // Storm the Beach. Which of the templates of that name Blizzard spawns is not known
    GO_SPIRE_OF_WOE                     = 240194,

    // A custom template: the object Blizzard spawns for Jaina's bridge of ice is not known
    GO_ICE_BRIDGE                       = 1460001
};

enum BrokenShoreSpells
{
    // Players are given it by the instance script, a stack more as the scenario goes on. On retail it is
    // one of Genn Greymane's abilities; that he is who casts it there is not known.
    SPELL_FOR_THE_ALLIANCE              = 185265,

    // "Cosmetic - Green Cat Mark State (5.00)": a large green arrow that points down at a unit. A video of
    // the retail scenario shows such arrows over the Anchoring Crystals; which spell they are is not known.
    SPELL_GREEN_CAT_MARK_STATE          = 151205,

    // "Before We're Overrun: Fel Beam", named after a quest of Mardum, where Spires of Woe stand too. A
    // spire channels it at its target on the ground. A video of the retail scenario shows such beams; which
    // spell they are is not known.
    SPELL_FEL_BEAM                      = 192664,

    // Dread Commander Arganoth
    SPELL_SUMMON_FELBLAZE_INFERNAL      = 183956,
    SPELL_FEL_CRACK                     = 200465,

    // Find Varian, which these spells call stage 2. A player has the stage's cutscene for as long as
    // "Stage 2 Scene" is on them. The other two are what the cutscene asks for while it plays: a look at
    // Varian's forces, and to be taken to the hilltop it looks out from. Where that look goes and where the
    // hilltop is, is in spell_target_position.
    SPELL_STAGE_2_SCENE                 = 218626,
    SPELL_STAGE_2_FAR_SIGHT             = 218636,
    SPELL_STAGE_2_TELEPORT              = 227791,

    // The cutscene that plays when the Black City has been razed, for as long as the aura is on a player
    SPELL_STAGE_3_SCENE                 = 181926,

    // "Dummy Cone of Cold": what Jaina casts as her bridge of ice forms. A video of the retail scenario
    // shows her cast a cone of frost there; which spell it is is not known.
    SPELL_DUMMY_CONE_OF_COLD            = 210120,

    // What Gul'dan holds Tirion over the pool with, which he channels at him. It was picked by eye against
    // a video of the retail scenario and is the nearest found, not a match.
    // TODO: the green rings around Tirion that the video shows.
    SPELL_GULDAN_CORRUPTION_BEAM        = 182542,

    // Krosus rises from the pool with the first, which was made with the scenario's other spells, and
    // breathes on Tirion with the second, which is a guess: a video of the retail scenario shows him
    // breathe fel fire on Tirion, and this is a breath that reaches 60 yards.
    SPELL_KROSUS_EMERGE                 = 208495,
    SPELL_FEL_BREATH                    = 217946,

    // What Krosus is seen to cast in a video of the retail scenario: "Slam", "Fel Storm" and "Orb of
    // Destruction". Which of the spells of those names they are is a guess: the Slam is that of Krosus in
    // the Nighthold, whose model he has, and does nothing by itself; the Fel Storm is a rain of fel, whose
    // missiles the client has no spells for; the Orb is a ball of fel fire that stuns where it lands.
    SPELL_KROSUS_SLAM                   = 205862,
    SPELL_KROSUS_FEL_STORM              = 221080,
    SPELL_ORB_OF_DESTRUCTION            = 183498,

    // What the Burning Sentry and the Living Felblaze are seen to cast in videos of the retail scenario. Both
    // are cones of fire, and were made with the scenario's other spells.
    SPELL_FEL_ERUPTION                  = 218621,
    SPELL_MASS_INCINERATION             = 218763,

    // What a Fel Meteor on the way to Varian does where it lands. The client has the effects of two more
    // spells to go with it, but not the spells themselves, so the core does not know them: the bearer of
    // 199044 casts 199031 every three seconds, a missile that lands within ten yards of the ground below
    // its caster after two seconds' wait and a flight at ten yards a second, and triggers this one. The
    // instance script does that in their place. That these are the scenario's is not known: they were made
    // when its other spells were, and they do what a video of the retail scenario shows. Who casts them
    // there is not known either; here it is a stalker in the air that cannot be seen.
    SPELL_FEL_METEOR                    = 199036,

    // Destroy the Portal. An Eredar Chaos Guard channels it on its Shielded Anchor, which nothing harms
    // while it lasts. The spell was made with the scenario's others; that the guards are who casts it,
    // and that they fight once they are attacked, is from videos of the retail scenario.
    SPELL_CHAOS_SHIELD                  = 181545,
    SPELL_LEGION_INVASION_SPAWN         = 180433, // the Mo'arg Painbringer, as it appears in the portal; picked by eye against a video

    // "Broken Shore Complete": how the scenario ends for a player. It gives the credit for the quest's
    // objective "Broken Shore assaulted", plays the cinematic of Varian's fall, takes the player out of
    // combat and lets nothing harm them for five minutes.
    SPELL_BROKEN_SHORE_COMPLETE         = 208513,

    // The Alliance's forces
    SPELL_WRATH                         = 171773, // Gilnean Druid
    SPELL_FROSTFIRE_BOLT                = 199219  // Kirin Tor Battle-Mage
};

// Movie.db2
enum BrokenShoreMovies
{
    MOVIE_BROKEN_SHORE_ALLIANCE         = 487 // what "Broken Shore Complete" plays
};

enum BrokenShoreSpellVisuals
{
    // The missile of 199031, the spell that sends a Fel Meteor down
    SPELL_VISUAL_FEL_METEOR             = 51706
};

enum BrokenShoreSpellVisualKits
{
    // The burst of fel fire on the ground that the aura "Before We're Overrun: Spire of Woe - Area Trigger"
    // (192604) shows on its bearer
    SPELL_VISUAL_KIT_SPIRE_OF_WOE_FIRE  = 59267
};

// creature_text groups of the Alliance's leaders. Group 0 is the same for both.
enum BrokenShoreLeaderTexts
{
    SAY_LEADER_IN_A_FIGHT               = 0,

    SAY_JAINA_REINFORCEMENTS            = 1,
    SAY_JAINA_NOW_OR_NEVER              = 2,
    SAY_JAINA_CRYSTALS                  = 3,
    SAY_JAINA_IT_WORKED                 = 4,
    SAY_JAINA_FOCUS_ON_THE_COMMANDER    = 5,
    SAY_JAINA_INFERNALS                 = 6,
    SAY_JAINA_IS_EVERYONE_ALRIGHT       = 7,
    SAY_JAINA_MOURN_THEM_LATER          = 8,
    SAY_JAINA_VARIAN                    = 9,
    SAY_JAINA_CRYSTALS_ARE_THE_KEY      = 10,
    SAY_JAINA_FOCUS_ON_THE_CRYSTALS     = 11,
    SAY_JAINA_WHAT_HAPPENED_HERE        = 12,
    SAY_JAINA_SURVIVORS                 = 13,
    SAY_JAINA_IVE_GOT_THIS              = 14,
    SAY_JAINA_NO                        = 15,
    SAY_JAINA_BEASTS_FROM_THE_FELFIRE   = 16,
    SAY_JAINA_PUT_THEM_ON_ICE           = 17,
    SAY_JAINA_ANOTHER_WAVE              = 18,

    SAY_GENN_JUST_IN_TIME               = 1,
    SAY_GENN_CHARGE                     = 2,
    SAY_GENN_HOPE_YOU_ARE_RIGHT         = 3,
    SAY_GENN_ONE_DOWN                   = 4,
    SAY_GENN_ONE_MORE                   = 5,
    SAY_GENN_ENOUGH_OF_YOUR_CHATTER     = 6,
    SAY_GENN_SINGED_BUT_ALIVE           = 7,
    SAY_GENN_AGREED                     = 8,
    SAY_GENN_AROUND_THIS_HILL           = 9,
    SAY_GENN_LETS_GO                    = 10,
    SAY_GENN_AND_YOU                    = 11
};

// creature_text groups of Dread Commander Arganoth
enum BrokenShoreArganothTexts
{
    SAY_ARGANOTH_ARRIVES                = 0,
    SAY_ARGANOTH_INFERNO                = 1,
    SAY_ARGANOTH_IN_THE_FIGHT           = 2,
    SAY_ARGANOTH_LOW_HEALTH             = 3,
    SAY_ARGANOTH_DEATH                  = 4
};

// creature_text groups of King Varian Wrynn
enum BrokenShoreVarianTexts
{
    SAY_VARIAN_GOOD_TO_SEE_YOU_SAFE     = 0,
    SAY_VARIAN_TAKE_DOWN_THIS_PORTAL    = 1,
    SAY_VARIAN_FORM_UP                  = 2,
    SAY_VARIAN_HEAL_THE_WOUNDED         = 3,
    SAY_VARIAN_WITH_ME_ALLIANCE         = 4,
    SAY_VARIAN_HALFWAY_THERE            = 5,
    SAY_VARIAN_IN_A_FIGHT               = 6,
    SAY_VARIAN_WHERE_IS_TIRION          = 7,
    SAY_VARIAN_SAVE_YOUR_STRENGTH       = 8,
    SAY_VARIAN_TIRION                   = 9,
    SAY_VARIAN_NEVER_GIVE_IN            = 10,
    SAY_VARIAN_TOGETHER_FOR_AZEROTH     = 11,
    SAY_VARIAN_BRACE_YOURSELVES         = 12,
    SAY_VARIAN_FOCUS_YOUR_FIRE          = 13,
    SAY_VARIAN_HEADING_YOUR_WAY         = 14,
    SAY_VARIAN_WEVE_GOT_HIM             = 15,
    SAY_VARIAN_ANOTHER_SHOT             = 16,
    SAY_VARIAN_BIGGER_THEY_ARE          = 17,
    SAY_VARIAN_ACROSS_THE_CHASM         = 18,
    SAY_VARIAN_SYLVANAS_THANK_YOU       = 19,
    SAY_VARIAN_AND_YOU                  = 20,
    SAY_VARIAN_NOWHERE_TO_RUN           = 21,
    SAY_VARIAN_HOLD_STEADY              = 22,
    SAY_VARIAN_WE_WILL_NEVER_FALL       = 23,
    SAY_VARIAN_CUT_YOU_DOWN             = 24
};

// creature_text groups of Highlord Tirion Fordring, Gul'dan, the Horde's leaders and Krosus
enum BrokenShoreTirionTexts
{
    SAY_TIRION_STAY_BACK                = 0,
    SAY_TIRION_THE_LIGHT_WILL           = 1,

    SAY_GULDAN_YOU_FOOL                 = 0,
    SAY_GULDAN_DESTROY_HIM              = 1,
    SAY_GULDAN_FALL_TO_ASH              = 2,
    SAY_GULDAN_DESTROY_THEM             = 3,
    SAY_GULDAN_WITNESS_THE_MIGHT        = 4,
    SAY_GULDAN_LEGION_IS_ENDLESS        = 5,
    SAY_GULDAN_DO_YOU_SEE_NOW           = 6,
    SAY_GULDAN_NOW_YOU_WILL_FALL        = 7,

    SAY_SYLVANAS_YOU_WILL_PAY           = 0,
    SAY_SYLVANAS_TOGETHER               = 1,
    SAY_SYLVANAS_LETS_GO_HORDE          = 2,
    SAY_SYLVANAS_ALL_YOURS              = 3,
    SAY_SYLVANAS_JOB_DONE               = 4,
    SAY_SYLVANAS_NO_TIME_TO_GLOAT       = 5,
    SAY_SYLVANAS_TAKE_THE_RIDGE         = 6,
    SAY_SYLVANAS_GOOD_LUCK_VARIAN       = 7,

    SAY_THRALL_FORDRING                 = 0,

    SAY_KROSUS_YOU_WILL_KNOW_FEAR       = 0,

    // Brutallus, Tichondrius and Azgalor each have one line, before Gul'dan
    SAY_LEGION_LORD_RETURNS             = 0
};

// creature_text groups of the Argent Dawnbringer
enum BrokenShoreDawnbringerTexts
{
    SAY_DAWNBRINGER_FELFIRE_EVERYWHERE  = 0
};

// creature_text groups of Gelbin Mekkatorque. Group 0 is SAY_LEADER_IN_A_FIGHT.
enum BrokenShoreGelbinTexts
{
    SAY_GELBIN_IT_HAPPENED_IN_SECONDS   = 1,
    SAY_GELBIN_HOW_TO_GET_ACROSS        = 2
};

// What the instance script has Dread Commander Arganoth do
enum BrokenShoreActions
{
    ACTION_ARGANOTH_CRASH_DOWN          = 1,
    ACTION_ARGANOTH_FIGHT               = 2,

    // What a Legion Cage has the Argent Dawnbringer inside it do when a player opens it
    ACTION_DAWNBRINGER_FREED            = 3,

    // What the instance script has Krosus do when Gul'dan has sent him: go to the Alliance's ledge and fight
    ACTION_KROSUS_ADVANCE               = 4
};

// The faction of the Alliance's leaders and forces in the scenario, which a freed Argent Dawnbringer takes
enum BrokenShoreFactions
{
    FACTION_BROKEN_SHORE_ALLIANCE       = 2879
};

// Creatures the instance script keeps track of (ObjectData), what it can be asked with GetData and what it can
// be told with SetGuidData
enum BrokenShoreDataTypes
{
    DATA_KING_VARIAN_WRYNN              = 0,
    DATA_HIGHLORD_TIRION_FORDRING       = 1,
    DATA_GULDAN                         = 2,
    DATA_LADY_JAINA_PROUDMOORE          = 3,
    DATA_GENN_GREYMANE                  = 4,
    DATA_DREAD_COMMANDER_ARGANOTH       = 5,

    // Whether the Alliance's forces are to charge (1) or to stay where they are (0)
    DATA_ALLIANCE_FORCES_CHARGING       = 6,

    // Whether Dread Commander Arganoth has come down on the beach and can be fought (1) or not yet (0)
    DATA_COMMANDER_ARRIVED              = 7,

    // Told with a player's GUID: the cutscene of "Find Varian" is over for that player
    DATA_PLAYER_LEFT_FIND_VARIAN_SCENE  = 8,

    // Where the Alliance's forces gather while they charge and have no one to fight (BrokenShoreRallyPoints)
    DATA_ALLIANCE_RALLY_POINT           = 9,

    DATA_GELBIN_MEKKATORQUE             = 10,

    // Told by a Legion Cage: a player has opened it and freed the Argent Dawnbringer inside
    DATA_DAWNBRINGER_FREED              = 11,

    // Told with a player's GUID: the cutscene after "Raze the Black City" is over for that player
    DATA_PLAYER_LEFT_BLACK_CITY_SCENE   = 12,

    DATA_LADY_SYLVANAS_WINDRUNNER       = 13,
    DATA_THRALL                         = 14,
    DATA_KROSUS                         = 15
};

enum BrokenShoreRallyPoints
{
    RALLY_POINT_NONE                    = 0,
    RALLY_POINT_ANCHORS                 = 1, // among the Shielded Anchors of "Destroy the Portal"
    RALLY_POINT_PORTAL_DESTROYED        = 2, // where they fall back to when the portal is destroyed
    RALLY_POINT_BLACK_CITY_GATE         = 3, // before the gate of the Black City, by way of BlackCityGateWay
    RALLY_POINT_BLACK_CITY              = 4, // the same place, but they go for the demons all over the city
    RALLY_POINT_BRIDGE                  = 5, // the far end of Jaina's bridge to the crevasse
    RALLY_POINT_TIRION                  = 6, // the ledge before the pool that Tirion hangs over
    RALLY_POINT_GULDAN                  = 7, // before Gul'dan, on the height above the pool
    RALLY_POINT_MAX
};

struct BrokenShoreRallyPoint
{
    float X;
    float Y;
    float Z;
    float JoinDistance;  // how far from it a fighter may be to belong to it; those left on the beach do not cross the island
    float FightDistance; // how far around itself a fighter that belongs to it looks for demons
};

// The places are from videos of the retail scenario, taken in game: where Jaina runs to when the forces
// advance, where the leaders gather when the portal is destroyed, and where they all gather before the gate
// of the Black City. That is a few yards back from where the video has them, to keep them off the cage
// there, and they do not look far for demons from it: the city's first are 50 yards on. From the gate the
// forces go for the nearest demon anywhere in the city, which is up to 250 yards across. Where they gather
// before Gul'dan is a guess: about 50 yards from him, on the way up from Jaina's second bridge.
static constexpr BrokenShoreRallyPoint AllianceRallyPoints[RALLY_POINT_MAX] =
{
    {    0.0f,    0.0f,  0.0f,   0.0f,   0.0f },
    { 1188.6f, 2454.1f, 36.2f, 150.0f,  45.0f },
    { 1162.1f, 2452.9f, 33.6f, 150.0f,  45.0f },
    { 1110.8f, 2336.7f, 20.2f, 200.0f,  30.0f },
    { 1108.1f, 2332.5f, 20.1f, 400.0f, 250.0f },
    { 1419.9f, 2082.1f, 21.0f, 150.0f,  30.0f },
    { 1495.8f, 1813.7f, 37.4f, 150.0f,  60.0f },
    { 1630.0f, 1702.0f, 77.5f, 250.0f,  60.0f }
};

// The middle of the gateway into the Black City, from the same videos. The forces that gathered before the
// gate go through it when they are sent into the city; the nearest demon is over a hill otherwise.
static constexpr BrokenShoreRallyPoint BlackCityGateway = { 1093.4f, 2325.3f, 20.2f, 0.0f, 0.0f };

// The way the forces take to the gate of the Black City from where they fell back to, from the same videos
static constexpr BrokenShoreRallyPoint BlackCityGateWay[] =
{
    { 1159.8f, 2403.9f, 31.1f, 0.0f, 0.0f },
    { 1124.1f, 2357.8f, 20.9f, 0.0f, 0.0f }
};

// The ways the forces take over Jaina's bridges of ice: the foot of the bridge, and the middle of its deck
// every ten yards along it. The deck rises in a curve; its heights are those of the model. The first bridge
// leads from the edge of the Black City to the crevasse.
static constexpr BrokenShoreRallyPoint HighlordBridgeWay[] =
{
    { 1415.5f, 2141.5f, 21.8f, 0.0f, 0.0f },
    { 1416.7f, 2130.1f, 22.2f, 0.0f, 0.0f },
    { 1417.6f, 2120.1f, 27.2f, 0.0f, 0.0f },
    { 1418.6f, 2110.2f, 31.7f, 0.0f, 0.0f },
    { 1419.6f, 2100.2f, 34.3f, 0.0f, 0.0f },
    { 1420.6f, 2090.3f, 36.4f, 0.0f, 0.0f },
    { 1421.4f, 2082.8f, 36.8f, 0.0f, 0.0f }
};

// The second leads from the ledge before the pool to the slope below the height Gul'dan stands on
static constexpr BrokenShoreRallyPoint GuldanBridgeWay[] =
{
    { 1528.6f, 1808.7f, 37.3f, 0.0f, 0.0f },
    { 1532.8f, 1797.5f, 37.9f, 0.0f, 0.0f },
    { 1536.4f, 1788.1f, 42.9f, 0.0f, 0.0f },
    { 1539.9f, 1778.8f, 47.4f, 0.0f, 0.0f },
    { 1543.4f, 1769.4f, 50.1f, 0.0f, 0.0f },
    { 1546.9f, 1760.1f, 52.1f, 0.0f, 0.0f },
    { 1549.6f, 1753.0f, 52.5f, 0.0f, 0.0f }
};

// spawn_group_template. Each stage's spawns are one group, spawned when the stage starts.
enum BrokenShoreSpawnGroups
{
    SPAWN_GROUP_STORM_THE_BEACH         = 14601,
    SPAWN_GROUP_DEFEAT_THE_COMMANDER    = 14602,
    SPAWN_GROUP_FIND_VARIAN             = 14603,
    SPAWN_GROUP_DESTROY_THE_PORTAL      = 14604,
    SPAWN_GROUP_RAZE_THE_BLACK_CITY     = 14605,
    SPAWN_GROUP_THE_HIGHLORD            = 14606,
    SPAWN_GROUP_KROSUS                  = 14607,
    SPAWN_GROUP_STOP_GULDAN             = 14608,

    // The bridges that Jaina makes are groups of their own, because they stay when their stage ends
    SPAWN_GROUP_BRIDGE_TO_THE_HIGHLORD  = 14609,
    SPAWN_GROUP_BRIDGE_TO_GULDAN        = 14610,

    // So are the Alliance's leaders and the forces that land with them, who go on from stage to stage
    SPAWN_GROUP_ALLIANCE_LEADERS        = 14611,
    SPAWN_GROUP_ALLIANCE_LANDING_FORCE  = 14612,

    // And the Spires of Woe, whose wreckage stays
    SPAWN_GROUP_SPIRES_OF_WOE           = 14613,

    // The Horde's leaders, who are there from "The Highlord" on
    SPAWN_GROUP_HORDE_LEADERS           = 14614
};

// WorldSafeLocs.db2. The "Start" locations are relative to the faction's ship, not to the map.
enum BrokenShoreWorldSafeLocs
{
    WORLD_SAFE_LOC_ALLIANCE_START       = 4981,
    WORLD_SAFE_LOC_ALLIANCE_BEACH       = 5021,
    WORLD_SAFE_LOC_ALLIANCE_PORTAL      = 5024,
    WORLD_SAFE_LOC_ALLIANCE_CITY        = 5025,
    WORLD_SAFE_LOC_ALLIANCE_CREVASSE    = 5026,
    WORLD_SAFE_LOC_ALLIANCE_TOMB        = 5027,

    WORLD_SAFE_LOC_HORDE_START          = 5456,
    WORLD_SAFE_LOC_HORDE_BEACH          = 5682,
    WORLD_SAFE_LOC_HORDE_PORTAL         = 5683,
    WORLD_SAFE_LOC_HORDE_CITY           = 5684,
    WORLD_SAFE_LOC_HORDE_CREVASSE       = 5685,
    WORLD_SAFE_LOC_HORDE_TOMB           = 5686
};

template <class AI, class T>
inline AI* GetBrokenShoreAI(T* obj)
{
    return GetInstanceAI<AI>(obj, BrokenShoreScriptName);
}

#define RegisterBrokenShoreCreatureAI(ai_name) RegisterCreatureAIWithFactory(ai_name, GetBrokenShoreAI)

#endif
