# Class hall mission rewards: the bonus roll, temporary abilities, reviving troops

Date: 2026-10-16
Branch: `engine/class-hall-mission-rewards`, stacked on `engine/class-hall-mission-extras`
Status: draft, awaiting review
Builds on: `2026-10-14-class-hall-missions-design.md`,
`2026-10-16-class-hall-mission-extras-design.md`

## Goal

What a class hall mission gives after it is done matches the 7.3.5
client and the rules the data holds: a team over 100% rolls for the
mission's bonus reward, a champion can be given a temporary ability that
lasts one mission, and a successful mission can bring back a troop that
expired.

## Decisions

Agreed with the owner on 2026-10-16:

- **Items brought back from missions (action 21) are left out.** Their
  record is a passive spell, not an item, and neither the items nor
  their chances are in the client data; nothing is invented.
- **Head Wound and Broken Gear (actions 30 and 31) are left out.** No
  `GarrMechanic` names their abilities, so no class hall mission can
  draw them.
- **Pain Suppression and Combustion stay out**, as in piece 3: their
  chances are not in the client data.
- **A temporary ability is used up when its mission is completed**,
  success or failure. The chance it gave is already stored at Start; the
  champion shows it while the mission runs.
- **Expired troops are remembered in their own table** (approach A), and
  a revive adds a fresh troop of the remembered kind through the normal
  path for gaining a troop.
- **Retail values, no server settings.**

## What the client data and UI say

- **The bonus chance.** `Blizzard_GarrisonMissionTemplates.lua` (7.3.5),
  for follower types that use the overmax mechanic:
  `overmaxSuccess = Clamp(successChance - 100, 0, 100)`. The mission
  page shows the first of the mission's `overmaxRewards` beside it.
- **The bonus rewards are not in the client.** `GarrMission.
  OvermaxRewardPackID` holds values such as 7334 and 7558 on class hall
  missions; the client's `RewardPack` ends at 402. They are content, as
  the normal mission rewards are (`class_hall_mission_reward`).
- **The client hears of a won bonus** through `overmaxSucceeded`: an
  argument of the complete response it drops (issue #1) and a field of
  the mission info it builds. Which part of the mission message sets the
  field is not known; the probe finds it.
- **Temporary abilities.** Seven class hall abilities carry
  `GARRISON_ABILITY_FLAG_SINGLE_MISSION_DURATION` (0x40), each taught by
  an item through `SPELL_EFFECT_LEARN_FOLLOWER_ABILITY` (244):

  | Item | Ability |
  |---|---|
  | 139419 | 662 Golden Banana |
  | 139428 | 663 Master Plan |
  | 140156 | 748 Blessing of the Order |
  | 140749 | 765 Horn of Winter |
  | 140760 | 776 Libram of Truth |
  | 143605 | 808 Strange Ball of Energy |
  | 143852 | 821 Lucky Rabbit's Foot |

  Each is "Increases success chance of a single mission by N%. Will be
  consumed on use." The client asks "Are you sure you want to grant %s
  this temporary ability?" and decides by itself whether a champion can
  take one (`C_Garrison.CanSpellTargetFollowerIDWithAddAbility`).
- **Revive (action 35).** Rebirth, Absolution, Arise My Champion,
  Reincarnation, Resurrection: "${$a}% chance on mission success to
  recover an expired Troop with ${$m} Vitality". `ActionValueFlat` is 50
  for all five; `CombatWeightMax` is 1 (the vitality).

## What the core already has

- `class_hall_mission_reward` (world): a mission's rewards, given at the
  chest (`ClassHallMissions::GiveRewards`). `BuildInfo` sends an empty
  overmax reward list per mission; the add-mission message has no
  overmax rewards.
- `ClassHallMissions::BonusRoll`: the chest. It gives the rewards, frees
  the team and removes its expired troops.
- `ClassHallMissions::CompleteMission`: the success roll and the troops'
  vitality loss (piece 3).
- `ClassHallFollowers`: equipment through effect 244 (`CheckItemCast`
  refuses single-mission abilities on purpose today), `GetAbilities`,
  `RemoveExpiredTroop`, `AddFollower` with `CanAddFollower` (class, unique
  and troop-kind limits).
- The character dump copies `character_class_hall_follower` with its
  dbIds remapped, and other class hall tables as plain character tables.

## Scope

In scope:

- The bonus roll and the bonus rewards, with the client told of them.
- Temporary abilities: granted by their items, counted, kept, used up.
- Expired troops remembered; revived by action 35 on a success.

Out of scope:

- Items brought back (21), Head Wound (30), Broken Gear (31), Pain
  Suppression and Combustion's chances.
- The first-mission-of-the-day ability flag (0x100).
- Content: no bonus reward rows; those belong on the content branch.
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

### The bonus roll

`class_hall_mission_reward` gets a column:

```sql
`Overmax` tinyint unsigned NOT NULL DEFAULT '0'   -- 1: a bonus reward
```

and its key becomes (`MissionID`, `Overmax`, `Idx`). `GarrisonMgr` loads
both kinds; `GetClassHallMissionRewards` and a new
`GetClassHallMissionOvermaxRewards` return them.

The bonus rewards are sent wherever the normal ones are: the info
message's overmax list and the add-mission message.

When a mission succeeds (at completion), the server rolls
`clamp(SuccessChance - 100, 0, 100)` and keeps the result with the
mission until its chest is opened, in a column of the mission row:

```sql
`bonusWon` tinyint unsigned NOT NULL DEFAULT '0'   -- character_class_hall_mission
```

A mission without bonus rewards rolls nothing. At the chest, after the
normal rewards, a won bonus gives the bonus rewards the same way (items
to the bags or the mail, currencies, money).

The roll is made at completion, not at the chest, so that the client can
be told before it shows the chest: through the part of the mission
message that sets `overmaxSucceeded`, as the probe shows. If no part of
it does, the client is not told: the rewards still arrive, only the
bonus chest's animation is missing (noted on issue #1).

### Temporary abilities

`character_class_hall_follower` gets a column:

```sql
`temporaryAbilityId` int unsigned NOT NULL DEFAULT '0'   -- 0: none
```

- `CheckItemCast`: an effect 244 ability with the single-mission flag is
  accepted on an active champion not on a mission that does not already
  hold that ability; whether one holding a different temporary ability
  is refused follows the client, as the probe shows.
- The spell effect stores it on the champion and sends the whole
  follower.
- `GetAbilities` puts it after the champion's own abilities and
  equipment, so the team-effects pass counts it with no change.
- At completion, success or failure, each champion on the team loses its
  temporary ability, and the whole follower is sent.
- Loading drops a value that is not a single-mission ability, with one
  error.

If the probe shows a champion can hold several at once, the column
becomes a table (`guid`, `dbId`, `abilityId`) and the rules stay the
same.

### Reviving troops

```sql
CREATE TABLE `character_class_hall_expired_troop` (
  `guid` bigint unsigned NOT NULL,
  `followerId` int unsigned NOT NULL,     -- GarrFollower
  `expireTime` bigint NOT NULL,
  PRIMARY KEY (`guid`, `expireTime`, `followerId`)
)
```

- `RemoveExpiredTroop` records the troop's kind and the time.
- At the chest of a successful mission, after its expired troops are
  removed, each action 35 effect on the team rolls `ActionValueFlat`; on
  a hit the most recently expired kind comes back as a new troop with
  `CombatWeightMax` vitality, through `AddFollower` (new dbId, the
  add-follower message, `CanAddFollower`'s checks). Refused, it does
  nothing and the record stays; taken, the record goes.
- Only the most recent `MAX_EXPIRED_TROOPS` records are kept (the retail
  number is not known; a chosen value, 10, named as such).
- The table is saved, loaded and deleted with the followers, and dumped
  as a plain character table.

## Commit sequence

1. `feat(engine,sql): load and send class hall mission bonus rewards`
2. `feat(engine,sql): roll for a class hall mission's bonus rewards`
3. `feat(engine,sql): give class hall champions temporary abilities`
4. `feat(engine,sql): remember expired class hall troops`
5. `feat(engine): revive expired class hall troops`

## Error handling

- A bonus reward of an unknown item or currency is skipped at load with
  an error, as the normal rewards are.
- A champion that no longer exists at completion or at the chest is
  skipped.
- A revive with no record does nothing; with the troop kind at its limit
  or no longer valid for the class, it does nothing and keeps the record.
- An expired-troop record of an unknown kind is dropped at load with one
  error.

## Verification

As before: no unit tests; the proof is in the client. The engine branch
is merged into a throwaway `integration/class-hall-mission-rewards`
branch cut from `feature/priest-class-hall`.

Probe first, no code:

1. Planted completed missions with different `MissionState` and `Flags`
   values: which, if any, makes the client report `overmaxSucceeded`.
2. A champion holding one temporary ability (planted): does the client
   let a second, different one be used on it.

Checks, in a few short logins (priest troop kinds, since `CanAddFollower`
refuses other classes' troops):

1. A mission with bonus rewards shows them beside the bonus line.
2. At 150% the bonus is rolled (shown and given over several missions);
   at 100% it never is.
3. A temporary ability item is accepted, shows on the champion, counts in
   the chance, survives a relog, and is gone after the mission, success
   and failure.
4. An expired troop is recorded; a revive on a success brings back a
   troop of that kind at 1 vitality; a revive with the kind at its limit
   brings back nothing and keeps the record.
5. A character dump and load keeps the expired troops and a temporary
   ability.

## Open points

- **How the client learns the bonus succeeded**: found by the probe.
- **Several temporary abilities on one champion**: found by the probe.
- **How many expired troops retail remembers**: not known; 10 chosen.
- **Items brought back, Pain Suppression, Combustion**: left out until a
  source gives their items or chances.

## What comes next

Priest class hall content on `feature/priest-class-hall`: recruiters,
research, the mission table, troops, and the content rows the engine
reads (mission rewards and bonus rewards).
