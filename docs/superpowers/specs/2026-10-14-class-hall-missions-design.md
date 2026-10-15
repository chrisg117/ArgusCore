# Class hall missions: the mission table

Date: 2026-10-14
Branch: `engine/class-hall-missions`, stacked on `engine/class-hall-shipments`
Status: built and tested in game on 2026-10-14; landed in `dev`
Builds on: `2026-10-12-class-hall-foundation-design.md`,
`2026-10-13-class-hall-talents-design.md`,
`2026-10-13-class-hall-followers-design.md`,
`2026-10-14-class-hall-shipments-design.md`

## Goal

A character with a class hall can open the mission table, see the missions
on offer, send champions and troops on one, pay its cost in Order
Resources, and when it ends learn whether it succeeded and collect its
rewards. The chance of success the server rolls against is the one the
client showed.

This is the first of three mission pieces, agreed on 2026-10-14:

1. **Missions** (this piece): offering, the table, starting, the success
   roll, rewards.
2. **Follower growth**: follower experience, levels, quality, item level
   and equipment.
3. **Mission extras**: troops losing vitality and dying, bonus ("overmax")
   rewards, mission bonus areas, abilities that change a mission's cost or
   time, and the remaining features.

## Decisions

These are proposed by this draft and are for the owner to confirm:

- **A separate `ClassHallMissions` class in its own files, owned by the
  hall,** with its own table, as for talents, followers and shipments.
- **Which missions are offered follows the client data's categories.**
  Missions with no offer time (the campaign and quest missions) are offered
  whenever their condition is met. The other missions rotate: the table
  keeps a fixed number on offer, drawn at random from those whose condition
  is met, each leaving after its own offer time. The number is not in any
  source I may use; see "Open points".
- **A condition the client data does not have counts as not met.** 304 of
  the 752 class hall missions name a player condition that is not in the
  client's `PlayerCondition.db2` (the same gap the troop kinds have). Such
  a mission is offered only once content defines its condition.
- **Rewards are content.** The client data has no mission rewards; the
  server sends them. The engine adds a world table and gives what it lists;
  the rows come from Wowhead's mission pages, on the content branch.
- **The success chance is the client's own rule, found by measuring the
  client.** No source I may use states it. The client works it out by
  itself for the team on the table, so the rule is recovered from the
  client's numbers for chosen teams and then checked against more teams.
- **The cost is taken by the server**, from the mission's own currency and
  amount, not by a spell: missions have no order spell.
- **Retail values, no server settings.**

## What the client data says

`GarrMission.db2` has 1395 missions, 752 of them class hall missions for
class hall followers (`GarrTypeID` 3, `GarrFollowerTypeID` 4). Fields used
here: `ID`, `GarrTypeID`, `GarrFollowerTypeID`, `GarrMissionTypeID`,
`MaxFollowers`, `MissionCost`, `MissionCostCurrencyTypesID`,
`MissionDuration`, `TravelDuration`, `OfferDuration`, `PlayerConditionID`,
`TargetLevel`, `TargetItemLevel`, `BaseCompletionChance`, `Flags`,
`OvermaxRewardPackID`.

- **Cost**: 726 cost Order Resources (currency 1220), 27 Garrison
  Resources, one nothing.
- **Size**: 638 take three followers, 62 two, 54 one.
- **Categories** (`GarrMissionType`), by count:

| Type | Name | Missions | With a condition | No offer time |
|------|------|----------|------------------|---------------|
| 71 | Generic Missions | 312 | 79 | 7 |
| 70 | Campaign Missions | 192 | 185 | 192 |
| 54 | Generic (7.0) | 69 | 69 | 69 |
| 72 | Treasure Missions | 56 | 4 | 1 |
| 76 | Treasure Missions - Raid | 36 | 36 | 0 |
| 74 | Quest Missions | 17 | 17 | 12 |
| 75, 73, others | Dungeon treasure, special reward, older types | 70 | | |

- **Offer time**: 302 have none (0); the others stay on offer for 6 hours
  to 10 days (`OfferDuration`, seconds).
- **Conditions**: 292 have none, 156 name a condition in the client data
  (none of them by class), 304 name one that is not there.
- **Encounters and threats**: a mission has up to three encounters
  (`GarrMissionXEncounter`), each with threats (`GarrEncounterXMechanic` ->
  `GarrMechanic`, which names a threat type and an ability). 539 encounter
  rows name an encounter set instead of an encounter, and 1112 threat rows
  name a threat set: those are chosen at random. How the client learns the
  chosen ones is not known (the mission's packet entry has no field for
  them); see "Open points".
- **Rewards**: not in the client data. 585 missions name an overmax reward
  pack (piece 3).
- **Base chance**: 669 missions have `BaseCompletionChance` 0; the rest 50
  to 100.
- **Follower death**: `FollowerDeathChance` is 0 for every class hall
  mission.

Wowhead's mission pages (`wowhead.com/mission=<id>`) list a mission's
encounters as seen, cost, champion experience, and the rewards seen, with
item ids.

## What the core already has

- `GarrMission.db2` is loaded (`sGarrMissionStore`); `GarrAbility.db2` and
  `GarrFollowerXAbility.db2` too. The encounter, threat and ability-effect
  tables are not.
- The packet structures `GarrisonMission` and `GarrisonMissionReward`
  (written for 6.x) and their place in the garrison info reply: missions,
  rewards, overmax rewards, "can start" bits, archived missions. The class
  hall's part of the reply sends none of them yet.
- `GarrisonFollower::CurrentMissionID`, sent with every follower, always 0.
- The garrison errors (`GARRISON_ERROR_MISSION_NOT_OFFERED`,
  `_FOLLOWER_ALREADY_ON_MISSION`, `_MISSION_SIZE_INVALID` and others).
- From the followers piece: the follower list, a follower's abilities, the
  active and inactive state, and the mission NPC's reply layout
  (`SMSG_GARRISON_OPEN_MISSION_NPC` = packed NPC guid + `int32` follower
  type), found in game but not yet sent by anything.

Missing: every mission request handler, the offering, storage, the success
chance and the rewards.

## Packets

From WowPacketParser (`V6_0_2_19033` and `V7_0_3_22248` garrison parsers):

| Packet | Layout |
|--------|--------|
| `CMSG_OPEN_MISSION_NPC` | packed NPC guid, `int32` follower type |
| `SMSG_GARRISON_OPEN_MISSION_NPC` | packed NPC guid, `int32` follower type |
| `SMSG_GARRISON_ADD_MISSION_RESULT` | `int32` garrison type, `int32` result, `uint8` state, mission, `uint32` reward count, `uint32` overmax count, rewards, overmax rewards, bit "can start" |
| `SMSG_DELETE_EXPIRED_MISSIONS_RESULT` | `int32` garrison type, `uint32` result, `int32` count, `uint32` mission each, bit "succeeded", one more bit |
| `CMSG_GARRISON_START_MISSION` | packed NPC guid, `int32` follower count, `int32` mission, `int64` follower id each |
| `SMSG_GARRISON_START_MISSION_RESULT` | `int32` result, `uint16` missions started this session, mission, `uint32` follower count, `uint64` follower id each |
| `CMSG_GARRISON_COMPLETE_MISSION` | packed NPC guid, `int32` mission |
| `SMSG_GARRISON_COMPLETE_MISSION_RESULT` | `int32` result, mission, `int32` mission, `uint32` follower count, per follower `uint64` id and `uint32` flags, bit "succeeded" |
| `CMSG_GARRISON_MISSION_BONUS_ROLL` | packed NPC guid, `uint32` mission |
| `SMSG_GARRISON_MISSION_BONUS_ROLL_RESULT` | mission, `int32` mission, `int32` result |

A mission entry is `uint64` id, `int32` mission, offer time, `int32` offer
duration, start time, `int32` travel duration, `int32` mission duration,
`uint32` state, `int32` success chance, `uint32` flags. States: offered 0,
in progress 1, completed 2, overmax pending 3.

The client's flow, from its UI code (`Blizzard_GarrisonMissionTemplates`):
the table sends start; when the time is up it sends complete
(`MarkMissionComplete`) and plays the encounters as the reply says; on a
success the player clicks the chest, which sends the bonus roll
(`MissionBonusRoll`), and the rewards are shown when its reply arrives.
So rewards are given on the bonus roll, not on complete.

## Scope

In scope:

- Loading the encounter, threat and ability-effect tables the success
  chance needs.
- `ClassHallMissions`, its table, a follower's current mission, and their
  place in the hall's load, save, delete and the character dump.
- Offering: which missions, when they are added and removed, and the
  packets that tell the client.
- The mission table: opening it from an NPC.
- Starting a mission: the checks, the cost, the followers, the success
  chance.
- Completing: the roll, the reply, freeing the followers.
- Rewards: the world table, giving them on the bonus roll, sending them
  with the mission.
- The missions in the garrison info reply at login.

Out of scope:

- Follower experience, levels, quality, item level, equipment (piece 2).
  `BaseFollowerXP` and experience rewards are not given yet.
- Troop vitality, deaths, overmax rewards, mission bonus areas, abilities
  that change cost or duration, items used on missions, combat allies
  (piece 3). Durations are the mission's own.
- Random choice among alternative rewards: a mission gives every reward
  row it has.
- Warlords missions. Requests from a character without a class hall are
  left as they are today.
- The mission table NPC, its spawn and flags, quests that need a mission
  started, the conditions the client data lacks, and the reward rows
  (content, on `feature/priest-class-hall`).
- GM commands or other test helpers.

## Rules this work follows

- Engine commits contain core code and schema SQL only.
- New code goes in new files. Shared files get small hook changes only.
- No warnings, no debug code, no TODO comments.
- Small commits, each booted and tested in game before the next.
- SQL files are named by the server's date, which is UTC.
- This document lives on `dev`, never on the engine branch.
- Test rows are planted by hand, never committed, and removed at the end.
  The test character is put back exactly as it was.

## Design

### 1. Stores

Load `GarrMissionXEncounter`, `GarrEncounter`, `GarrEncounterXMechanic`,
`GarrMechanic`, `GarrMechanicType` and `GarrAbilityEffect`, each with its
hotfix table, plus the encounter and threat set tables if the probe
(section 9) shows the server must choose from them. Only what the success
chance and the offering read is loaded; the list is final after the
probe.

### 2. `ClassHallMissions`

New files: `src/server/game/ClassHall/ClassHallMissions.h` and `.cpp`.

It belongs to one `ClassHall` and holds the character's missions by
mission id: offered ones and ones in progress or completed. For each: the
mission row, its id for the client, when it was offered, when it was
started, its success chance once started, and its state. A character holds
a mission at most once.

It remembers, for the current session only, which NPC's table is open. The
start, complete and bonus roll requests name the NPC.

`ClassHall` owns one, forwards load, save and delete to it, and calls it
from `ClassHall::Update` for offering.

### 3. Storage

New table `character_class_hall_mission`:

| Column | Type | Meaning |
|--------|------|---------|
| `guid` | bigint unsigned | the character |
| `missionId` | int unsigned | `GarrMission.db2` id |
| `dbId` | bigint unsigned | the mission's id for the client, unique per character |
| `offerTime` | bigint | when it was offered |
| `startTime` | bigint | when it was started, 0 while offered |
| `successChance` | int | set at start |
| `state` | tinyint unsigned | offered, in progress or completed |

Primary key `(guid, missionId)`. No foreign key to the hall row, for the
reason the earlier tables give.

`character_class_hall_follower` gets a column `missionId` (int unsigned,
default 0): the mission a follower is on. It is what the follower entry's
`CurrentMissionID` sends, so the follower list needs no lookup.

The character dump gets the new table as a per-character table. The
follower column needs nothing: it holds a mission id, not a follower id.

### 4. Offering

Run at login and from the hall's update, at most once a minute:

1. **Remove** offered missions whose offer time has run out. No packet is
   sent: 7.3.5's opcode list has no "delete expired missions" message, so
   the client is expected to drop an offer at its own end time (checked in
   game). Missions in progress or completed never expire.
2. **Add** every mission with no offer time whose condition is met and that
   the character does not hold, has not finished, and could be offered
   (rules below).
3. **Fill** the rotation: while fewer than the rotation's size of rotating
   missions are on offer, add one at random from those eligible.

Each added mission is sent with `SMSG_GARRISON_ADD_MISSION_RESULT` (state
offered, its rewards, "can start" true).

A mission is eligible when: it is a class hall mission for the hall's
follower type; its condition is met (a condition missing from the client
data counts as not met); and the character does not hold it now.
Recruitment, zone support and invasion missions (types 11, 51, 52, 57)
are not offered: they are other mechanisms (zone support is piece 3).

"Has not finished": a campaign or quest mission, once completed, is not
offered again. This needs a record of finished one-time missions; the
client's archived-mission list in the info reply is that record. Whether
it is kept in a table or follows from quest state is settled with the
content's first campaign mission; until then no-offer-time missions are
offered again after completion.

### 5. Opening the table

`CMSG_OPEN_MISSION_NPC` is answered when the NPC has
`UNIT_NPC_FLAG_2_GARRISON_MISSION_NPC`, is in reach, and the character has
a class hall: the NPC is remembered and `SMSG_GARRISON_OPEN_MISSION_NPC` is
sent with follower type 4. The client sends nothing when the table
closes, so the remembered NPC simply stays until another is opened; the
reach check on every request covers walking away.

### 6. Starting

`CMSG_GARRISON_START_MISSION` names the NPC, the mission and the
followers. It is accepted when all of these hold; the first failure is the
answer, in `SMSG_GARRISON_START_MISSION_RESULT`:

| Check | Error |
|-------|-------|
| The NPC is the remembered one and in reach | ignored |
| The mission is held and offered | mission not offered (37) |
| The follower count is between 1 and `MaxFollowers` (exact rule from the probe) | mission size invalid (39) |
| Each follower is owned, of the hall's type, active, and on no mission | invalid follower (28), follower already on mission (29) |
| The character can pay the cost | not enough currency (the client's error) |

Then: the cost is taken; the success chance is worked out (section 7) and
stored; the mission becomes in progress with the start time now; each
follower's `missionId` is set; the reply carries result 0, the mission and
the follower ids.

### 7. The success chance

Worked out once, at start, from the mission and the team, by the client's
rule. What the rule takes into account is known from the client UI:
threats countered by the followers' abilities, the followers' level and
item level against the mission's targets, troops, and abilities that raise
the chance. The arithmetic is not. It is found in two steps before this
code is written:

1. **Measure.** A small test add-on in the client (never committed) puts
   chosen teams on chosen missions with the client's own functions
   (`C_Garrison.AddFollowerToMission`, `GetMissionSuccessChance`,
   `GetMissionUncounteredMechanics`) and writes every result to its saved
   variables, which are read from the client folder on this PC. One
   command in game runs all of it.
2. **Check.** The rule written from those numbers is run on the server for
   a second set of teams the add-on also measured; every one must match.

The chance may go above 100; it is stored and sent as it is, and the roll
treats anything at or above 100 as certain. What lies above 100 belongs to
overmax rewards (piece 3).

### 8. Completing and rewards

`CMSG_GARRISON_COMPLETE_MISSION`: for a held mission in progress whose
travel and mission time have passed, the server rolls against the stored
chance. The followers' `missionId` goes back to 0. The reply carries the
mission, its followers, and "succeeded".

- On a failure the mission is removed.
- On a success it becomes completed and waits for the bonus roll.

`CMSG_GARRISON_MISSION_BONUS_ROLL`: for a completed mission, the rewards
are given, the mission is removed, and
`SMSG_GARRISON_MISSION_BONUS_ROLL_RESULT` is sent with result 0.

New world table `class_hall_mission_reward`:

| Column | Type | Meaning |
|--------|------|---------|
| `MissionID` | int unsigned | `GarrMission.db2` id |
| `Idx` | tinyint unsigned | order of the rewards |
| `ItemID` | int unsigned | an item, or 0 |
| `ItemQuantity` | int unsigned | |
| `CurrencyID` | int unsigned | a currency, or 0 |
| `CurrencyQuantity` | int unsigned | |

Money is currency 0 with the amount in copper, as the client shows it. An
item that does not fit in the bags is mailed. The rows are validated at
load (unknown mission, item or currency: the row is skipped with an
error). The engine branch adds the table, empty.

### 9. The probe

Before the packets and the success chance are written, one login with
hand-sent packets and the test add-on settles:

- that `SMSG_GARRISON_ADD_MISSION_RESULT` in the 6.x layout puts a mission
  on the client's table, and that the open reply opens the table;
- how the client learns a mission's random encounters and threats: two
  offers of one set-based mission with different ids, compared;
- whether the client requires every follower slot filled;
- the success chances for the measured teams.

Gairy gets planted champions and troops of several kinds and qualities for
it.

## Commit sequence

1. `feat(engine,sql)`: load the mission encounter and threat tables
2. `feat(engine,sql)`: store class hall missions
3. `feat(engine)`: add the mission packets
4. `feat(engine,sql)`: offer class hall missions
5. `feat(engine)`: open the mission table
6. `feat(engine)`: work out a mission's success chance
7. `feat(engine)`: start a class hall mission
8. `feat(engine)`: complete a class hall mission
9. `feat(engine,sql)`: reward a class hall mission

The probe sits between commits 2 and 3. Commit 6 may move or split
depending on what the probe shows about random encounters.

## Error handling

- Saved mission whose id is not a class hall mission: skipped, one error,
  login continues; the next save drops it.
- Saved follower `missionId` naming a mission the character does not hold
  in progress: set back to 0 at load, one error.
- A mission in progress whose followers were dismissed meanwhile (cannot
  happen through the client; possible by hand): completes with the
  followers it still has.
- Requests naming another NPC, one out of reach, a mission not held, or a
  mission in the wrong state: ignored or answered with the matching error.
- A complete request before the time is up: answered with an error
  result, the mission left in progress (the client then shows it as not
  ready).

## Verification

As before: no unit tests; the proof is in the client. The engine branch is
merged into a throwaway `integration/class-hall-missions` branch cut from
`feature/priest-class-hall`.

Test setup, by hand, never committed: Gairy's hall row, planted
followers, Order Resources; a creature with the mission NPC flag spawned
next to her; a few planted reward rows; missions' times shortened by
editing their start time while she is offline.

Checks, in a few short logins with a handful of steps each:

1. Login: the table opens and shows offered missions with their rewards;
   a relog shows the same missions.
2. Starting a mission takes its cost, marks the followers as on a mission,
   and shows the same success chance the table showed.
3. Starting with a follower already on a mission, too many followers, or
   without enough Order Resources is refused and takes nothing.
4. After the time is up, completing plays the result; on a success the
   chest gives the planted rewards; on a failure nothing is given and the
   followers are free.
5. An expired offer leaves the table.
6. A character with no hall logs in and plays as before.

## What the tests showed

Tested in game on 2026-10-14 with a priest, hand-planted champions and
troops, a test add-on that read the client's own numbers, and the packet
log.

- **The table is the adventure map.** The client asks with
  `CMSG_OPEN_MISSION_NPC` (it sends follower type 1). The 7.3.5 interface
  ignores `GARRISON_MISSION_NPC_OPENED` for class hall followers; the
  table opens on `SMSG_ADVENTURE_MAP_OPEN_NPC` (0x2835, the NPC guid
  only). Its Missions and Followers tabs show only once the character
  has a class hall follower.
- **The info reply carries each mission's reward count right before its
  rewards**, not all counts first; with the old layout the client read
  garbage after the reward lists and showed no followers.
- **The champion limit must be sent.** Without a `FollowerSoftCaps`
  entry for follower type 4 the client shows "Champions: N/0" and starts
  no mission. It is `GarrFollowerType.MaxFollowers` (5).
- **Random encounters and threats: the client draws them itself**, and
  the server makes the same draw. The client uses `std::minstd_rand`
  (48271, 2^31 - 1) seeded with the mission's db id, one value per set,
  index = value modulo the set's size, in this order: the mission's
  encounter rows by `OrderIndex`, an encounter set if the row names one,
  then the chosen encounter's threat rows by id, a threat set where a row
  names one. Set members are taken in id order. 67 of 67 draws matched.
- **The success rule**, measured from the client's own numbers (2160
  teams on 10 missions, two logins; fitted on half, checked on the other
  half, all matched):
  - A champion's bias: (level - mission level) / 5, plus (item level -
    mission item level) / 30 when it is not above the mission level,
    kept within -1 and 1 (`GarrFollowerType` level and item level
    range). Troops have none. A champion at -1 adds nothing, not even
    its traits.
  - Power: 100 (1 + bias) below 0, 100 + 50 bias above.
  - Counters (ability effect action 0, target 0): weight base (1 + bias)
    below 0, base + (max - base) bias above; per threat type the counter
    weight counts up to the sum of that type's threat factors.
  - Chance = 100 (power + countered) / (sum of factors + 100), plus flat
    bonuses: action 2 always, action 23 when the mission has a threat of
    the type, action 22 when a follower of the class spec is on the team.
  - An uncountered threat with the no-bonus-loot effect (action 29) keeps
    the chance at 100. The result is rounded half to even.
  - Item level is taken as (weapon + armor) / 2; every follower probed
    had both equal, so piece 2 should re-check it with unequal values.
- **Whether every slot must be filled: no.** The client starts a mission
  with any team that holds at least one champion.
- **Starting, refusals and rewards behaved as designed**: the shown
  chance equalled the server's on every start; Order Resources were
  taken once; a busy follower, an early complete request and the
  dismissal of a troop on a mission were refused; an expired offer left
  the table at the next login; items, Order Resources and money were
  given by the chest. A second chest and a start without the cost were
  refused by the client itself, before any request.
- **A failed mission goes back on offer.** The client takes every
  completed mission for a successful one (it shows the success screen
  and a chest for it), so a failed mission is not left completed.
- **A successful mission keeps its followers until its chest is
  opened.** The client's report on a completed mission shows them; with
  none it stops with a Lua error.

Still open:

- **The client drops every complete reply.** No
  `SMSG_GARRISON_COMPLETE_MISSION_RESULT` the server sends is taken: no
  `GARRISON_MISSION_COMPLETE_RESPONSE` event, no mission list update.
  Tried: WowPacketParser's 7.x layout, five other layouts, both
  connections, the neighbouring unnamed opcodes, follower experience
  messages and `SMSG_GARRISON_OPEN_MISSION_NPC` alongside. The client
  binary is encrypted, so its check cannot be read. In play, the first
  view of a finished mission shows no fight and its chest stays locked;
  closing and reopening the table shows the mission completed with its
  chest. A retail 7.x capture of any class hall mission being completed
  would settle the layout. Fallbacks if none turns up: complete missions
  on the server when they come due and send the info again, or an
  add-on.
- **The rotation's size** stays 10 (`ROTATING_MISSIONS_ON_OFFER`), a
  deliberate choice.
- **The archived-mission list**: what keeps finished one-time missions
  from coming back, with the first campaign mission's content.
- **Threats that double cost or duration, and troop deaths**: piece 3.

## What comes next

`engine/class-hall-follower-growth`: follower experience from missions,
levels, quality, item level and equipment. Its own spec, plan and branch,
stacked on this one.
