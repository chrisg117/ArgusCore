# Class hall mission extras: the team's full effects and troop vitality

Date: 2026-10-16
Branch: `engine/class-hall-mission-extras`, stacked on `engine/class-hall-follower-growth`
Status: draft, awaiting review
Builds on: `2026-10-14-class-hall-missions-design.md`,
`2026-10-15-class-hall-follower-growth-design.md`

## Goal

A class hall mission does what the 7.3.5 client shows before the player
presses Start: the success chance counts every ability the team brings,
the cost and duration are changed by the mission's threats and the
team's abilities, and troops wear out, one vitality per mission, dying
outright on a Lethal mission unless the threat is countered.

## Decisions

Agreed with the owner on 2026-10-16:

- **The rest of the mission work is split in two.** This piece (3)
  covers everything the client shows before Start: the missing chance
  effects, cost and duration, troop vitality. Piece 4 covers what
  happens after: the overmax bonus roll, items brought back from
  missions, the Head Wound and Broken Gear threats, single-mission items,
  reviving expired troops.
- **Troops wear out and can be healed in this piece; revive waits.** The
  revive abilities need a record of expired troops; they go to piece 4.
- **One team-effects pass** (approach 1): the success chance function
  becomes one pass over the team that returns the chance, the cost, the
  duration and whether the troops die. The rule stays in one place, the
  way the client works it out, so it can be checked against the client's
  own numbers.
- **Pain Suppression and Combustion's extra vitality loss are left out.**
  Their chances are not in the client data and no public source gives
  them; they are not invented here (see "Open points").
- **Retail values, no server settings.**

## What the client data says

`GarrAbilityEffect` rows of class hall abilities (`GarrFollowerTypeID`
4), by `AbilityAction`, for the actions this piece adds. A "threat" is
an ability named by a `GarrMechanic` (a mission's encounter mechanic).

| Action | Meaning (ability description) | Value | Examples |
|---|---|---|---|
| 6 | chance + value on missions longer than `ActionHours` | `ActionValueFlat` | Agony 15 (8 h), Azurelight Sapphire 40 (8 h) |
| 7 | chance + value on missions shorter than `ActionHours` | `ActionValueFlat` | Hasty Pocketwatch 20 (4 h), Void-Touched Arinor Blossom 40 (8 h) |
| 17 | duration × value | `ActionValueFlat` | threat Slowing 2.0; Flying Serpent Kick, Dreadsteed 0.8 |
| 24 | chance + value per point of troop vitality on the mission | `ActionValueFlat` | Soul Link, Commanding Shout 5 |
| 25 | chance + value with troops of only N vitality | flat 20, N = `CombatWeightMax` 1 | Enrage |
| 26 | chance + value with the follower `ActionRecordID` on the team | `ActionValueFlat` | Chenship (596) 20; To Me Squire! (one row per paladin champion) |
| 27 | troops on the mission are killed | | threat Lethal |
| 28 | a random troop on the mission recovers vitality | 1 | Light of Valor, Leeching Poisons |
| 33 | chance + value with troops of different kinds | `ActionValueFlat` | Royal Bloodline 15 |
| 39 | cost × value (currency `ActionRecordID`, 1220) | `ActionValueFlat` | threat Disorienting 2.0; Berserk 0.7 |
| 41 | chance + value with no other champion on the team | `ActionValueFlat` | Hero of the Storm, Titan's Wrath 15 |

Also action 28: Pain Suppression ("chance for a random troop to not take
vitality damage", `CombatWeightBase` −1, `CombatWeightMax` 0) and
Combustion ("troops have a chance to lose extra vitality", 0 and 1), whose
chances the data does not give.

Rows with `GarrAbility.Flags` 0x2 only (no 0x10), such as "Carrot on a
Stick" and "Green - Mission Cost", are Warlords-era rows carried in type 4
data; they follow the same rule.

`GarrMission.FollowerDeathChance` is 0 for all 756 class hall missions:
troops die only of vitality, or of Lethal.

`GarrFollower.Vitality` is a troop kind's starting and maximum vitality
(public guides: 2 for lesser troops, 3 for greater, 1 for special class
troops; each completed mission costs one; an uncountered Lethal threat
takes all of it).

## What the core already has

- `ClassHallMissions::GetSuccessChance` (missions piece): champions'
  power from their bias, counters against threat types, flat bonuses
  (actions 0, 2, 22, 23), and the "no bonus loot" threat (29). Measured
  against the client's own numbers, 2160 of 2160 teams.
- Start charges `GarrMission.MissionCost` and stores the chance; the end
  time is `StartTime + TravelDuration + MissionDuration` from the
  mission's own entry.
- A troop's vitality is `Follower::PacketInfo.Durability`, saved in the
  follower table and set to `GarrFollower.Vitality` when the troop is
  gained.
- `ClassHallFollowers::RemoveFollower` (dismissing a troop) and
  `SendUpdateFollower` (the whole follower, 0x2904, applied silently).
- Champions stay on a successful completed mission until its chest is
  opened (the bonus roll frees them); a failed mission frees them at
  once.

## Scope

In scope:

- The team-effects pass: chance actions 6, 7, 24, 25, 26, 33, 41; cost
  (39); duration (17); Lethal (27).
- Charging the changed cost and keeping the changed duration, stored
  with the mission.
- Troop vitality: the loss at completion, Lethal, heals with a fixed
  amount (action 28), removing a troop that reaches 0.
- The messages that tell the client.

Out of scope:

- The overmax bonus roll, items brought back from missions (21), Head
  Wound (30), Broken Gear (31), single-mission items (flag 0x40),
  reviving expired troops (35) (piece 4).
- Pain Suppression, Combustion's extra loss.
- Combat allies (37), troop limits (34), champion limit (3).
- Warlords missions and followers.
- GM commands or other test helpers.

## Rules this work follows

- Engine commits contain core code and schema SQL only.
- New code goes where the code it extends lives; shared files get small
  hook changes only.
- No warnings, no debug code, no TODO comments.
- Small commits, each booted and tested in game before the next.
- SQL files are named by the server's date, which is UTC.
- This document lives on `dev`, never on the engine branch.
- Test rows are planted by hand, never committed, and removed at the end.
  The test character is put back exactly as it was.

## Design

### The team-effects pass

`ClassHallMissions::GetSuccessChance` becomes

```cpp
struct TeamEffects
{
    uint32 SuccessChance = 0;
    uint32 Cost = 0;
    uint32 MissionDuration = 0;
    bool Lethal = false;
};

TeamEffects GetTeamEffects(Mission const& mission, std::vector<ClassHallFollowers::Follower const*> const& team) const;
```

It keeps the existing chance rule unchanged and adds, in the same walk
over the team's abilities and the mission's threats:

- **Duration** starts at `GarrMission.MissionDuration` and is multiplied
  by each action 17 value; **cost** starts at `GarrMission.MissionCost`
  and is multiplied by each action 39 value whose currency is the
  mission's.
- **Threat effects** (17, 39, 27) apply only while the threat's mechanic
  type is not countered by the team, as the existing "no bonus loot"
  threat does.
- **The chance actions** add their flat value when their condition
  holds: 6 and 7 compare the mission's duration with `ActionHours`; 24
  counts the vitality of the troops on the team; 25 needs a troop with
  exactly `CombatWeightMax` vitality; 26 needs the follower
  `ActionRecordID` on the team besides the ability's owner; 33 needs at
  least two troops of different kinds; 41 needs no other champion.
- **Lethal** is set when an uncountered threat has action 27.

What the probe settles before the code is written: whether countering a
threat removes its cost, duration and Lethal effects; whether several
multipliers multiply or add, and how the result is rounded; whether
actions 6 and 7 compare against the mission's own duration or the
changed one; how troop vitality is counted for action 24. The rules are
written to the client's numbers, as the chance was.

### Start and the stored duration

Start checks the changed cost against the player's currency, charges it,
and stores the chance and the changed duration in the mission's packet
info (`MissionDuration`). The end time and every message use the stored
duration.

`character_class_hall_mission` gets a column:

```sql
`missionDuration` int unsigned NOT NULL DEFAULT '0'
```

0 means the mission's own duration, so rows saved before this change load
unchanged and need no migration. The character dump copies the table as
a whole (`DTT_CHAR_TABLE`) and needs no change.

### Troops

At the completion roll, on a success and on a failure:

1. Each troop on the team loses 1 vitality, or all of it when the team's
   `Lethal` is set.
2. Each heal (action 28 with a fixed amount, from an ability on the team)
   gives one random troop on the team that amount back, up to its kind's
   `GarrFollower.Vitality`.
3. A troop at 0 vitality is removed when the mission lets its followers
   go: at once on a failure, when the chest is opened on a success (the
   completed-mission report needs its followers).

`ClassHallFollowers` gets:

```cpp
void SetVitality(uint64 dbId, uint32 vitality);   // sends the whole follower
void RemoveExpiredTroop(uint64 dbId);             // erases and tells the client
```

Vitality changes are sent as the whole follower (0x2904). The removal
uses the remove-follower message; the probe checks which `Destroyed`
value the client takes for a troop that expired.

## Commit sequence

1. `feat(engine,sql): store a class hall mission's duration`
2. `feat(engine): change a class hall mission's cost and duration by its team`
3. `feat(engine): count the rest of a team's abilities in the success chance`
4. `feat(engine): wear out class hall troops on missions`
5. `feat(engine): heal class hall troops on missions`

## Error handling

- Vitality never goes below 0 or above the kind's `Vitality`.
- Cost and duration are rounded as the probe shows and are never
  negative.
- A stored duration for a mission row is used as it is; 0 falls back to
  the mission's own duration.
- A follower on the team that no longer exists at completion is skipped.
- A heal with no troop on the team does nothing.

## Verification

As before: no unit tests; the proof is in the client. The engine branch
is merged into a throwaway `integration/class-hall-mission-extras`
branch cut from `feature/priest-class-hall`.

Test setup, by hand, never committed: Gairy's hall, planted champions
with the abilities under test, planted troops at chosen vitality,
planted equipment (Hasty Pocketwatch and the like), planted missions with
chosen threats, made due when needed.

Probe first: a test add-on reads the client's own chance, cost and
duration for planted teams, including countered and uncountered threats
and stacked multipliers. The rules are fitted to half the teams and
checked against the other half.

Checks, in a few short logins:

1. The table's chance, cost and duration for teams with each new action
   match what the server charges, stores and rolls.
2. A changed duration survives a relog.
3. A troop with 2 vitality has 1 after a mission.
4. A troop with 1 vitality is removed: at once on a failure, at the chest
   on a success.
5. An uncountered Lethal mission kills its troops; a countered one costs
   one vitality.
6. A heal gives a troop its vitality back, never above its maximum.
7. A relog keeps every troop's vitality.

## Open points

- **Pain Suppression and Combustion**: the chances are not known; left
  out until a source gives them.
- **The removal message for an expired troop**: found by the probe.
- **The parked mission bug** (issue #1): the client drops the mission
  complete reply, so the report does not show troop deaths as they
  happen; the removal arrives in its own message.

## What comes next

Piece 4, `engine/class-hall-mission-rewards`: the overmax bonus roll,
items brought back from missions, Head Wound and Broken Gear,
single-mission items, reviving expired troops. Its own spec, plan and
branch, stacked on this one.
