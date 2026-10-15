# Class hall follower growth: experience, quality, item level, equipment

Date: 2026-10-15
Branch: `engine/class-hall-follower-growth`, stacked on `engine/class-hall-missions`
Status: draft, awaiting review
Builds on: `2026-10-13-class-hall-followers-design.md`,
`2026-10-14-class-hall-missions-design.md`

## Goal

Class hall champions grow the way they did in Legion: experience from
successful missions and from tomes raises their level from 100 to 110 and
then their quality; armament items raise their item level; equipment items
fill the equipment slots their quality unlocks. Troops do not grow.

## Decisions

Agreed with the owner on 2026-10-15:

- **One piece covers both ways champions grow**: from missions
  (experience, levels, quality) and from items used on them (armaments,
  tomes, equipment).
- **The growth lives in `ClassHallFollowers`**, which already owns the
  followers' state, storage and messages. Missions call it on a success;
  the item spell effects call it on a cast. No new class, no change to the
  Warlords `Garrison` class.
- **A failed mission gives no experience.** No source found states the
  retail rule; this is a deliberate choice, kept in one place so it is one
  line to change if a source turns up (see "Open points").
- **The experience bonus of a team is the client's own rule, found by
  measuring the client**, as the success chance was: the client works out
  `xpBonus` for the team on the table.
- **Retail values, no server settings.**

## What the client data says

Read from the 7.3.5 client files (class hall follower type 4 only):

- `GarrFollowerLevelXP` (rows 30-40): experience to the next level by
  level, 200 at level 100 rising to 3000 at level 109; level 110 has none.
  The highest level with a row and no next step is the cap: 110.
- `GarrFollowerQuality` (rows 32-35, 45, 46): experience to the next
  quality at level 110, and how many abilities and traits each quality
  has:

  | Quality | To next | Abilities | Traits (equipment slots) |
  |---|---|---|---|
  | 1 | 8000 | 2 | 0 |
  | 2 | 20000 | 3 | 0 |
  | 3 | 100000 | 3 | 1 |
  | 4 | 200000 | 3 | 2 |
  | 5 | none | 3 | 3 |
  | 6 | none | 3 | 3 |

  Quality 6 has no experience step: no class hall item reaches it through
  this piece (see "Scope").
- `GarrFollowerXAbility`: a champion's list holds its specialization
  ability (flags 0x230), its abilities, and three placeholder traits with
  the empty-slot flag (0x401): abilities 414, 415 and 855. These are the
  equipment slots; the quality's trait count says how many are unlocked.
  Troops have one ability and no traits.
- `GarrItemLevelUpgradeData` (fields Operation, MinItemLevel,
  MaxItemLevel, FollowerTypeID; rows 12-24 for type 4):
  - Operations 2 and 3, rows 12-15: add the spell's amount, up to the
    row's MaxItemLevel (850 for rows 12-13, 900 for rows 14-15). Used by
    the armor sets (+5, +10, +15).
  - Operations 0 and 1, rows 16-24: raise to the spell's amount (850, 880,
    900, 925, 950), the row's MaxItemLevel. Used by the 7.3 catch-up sets.
  - Each armament spell has two effects, one per operation of a pair: one
    for the weapon item level, one for the armor item level (which is
    which is settled by the probe; both move together).
- Spell effects used by class hall items:
  - 80 `MODIFY_FOLLOWER_ITEM_LEVEL`: 24 spells; MiscValue = the upgrade
    row, BasePoints = the amount.
  - 231 `INCREASE_FOLLOWER_EXPERIENCE`: 2 spells (4000 and 6000);
    MiscValue2 = follower type 4.
  - 244 `LEARN_FOLLOWER_ABILITY`: 136 spells; MiscValue = the ability,
    MiscValue2 = follower type 4. 125 teach a trait (flags 0x3): equipment.
    7 teach a single-mission ability (flags 0x40, BasePoints 1): piece 3.
    4 have no item.
  - 229 (quality), 233 (reroll abilities) and 287 (set level): no class
    hall item uses them.
- `GarrMission.BaseFollowerXP`: the experience a mission gives.

## What the core already has

- `ClassHallFollowers` stores quality, level, both item levels and
  experience in `character_class_hall_follower`, and derives abilities
  from quality with `GetAbilities` (abilities, then empty-slot traits up
  to the quality's counts).
- The spell effects 80, 231 and 244 are `EffectNULL`.
- `GarrFollowerLevelXP` and `GarrItemLevelUpgradeData` are not loaded.
- `ClassHallMissions::CompleteMission` rolls the outcome and knows the
  team.
- The opcode names disagree with WowPacketParser's 7.3.5 list for two
  messages: 0x2914 is `SMSG_GARRISON_FOLLOWER_CHANGED_ABILITIES` in the
  core and `..._CHANGED_QUALITY` in WowPacketParser; 0x2904 is
  `..._CHANGED_DURABILITY` in the core and `SMSG_GARRISON_UPDATE_FOLLOWER`
  in WowPacketParser. 0x2912 (`..._CHANGED_XP`: earned, source, follower
  before, follower after) and 0x2913 (`..._CHANGED_ITEM_LEVEL`) agree.
  The XP message's layout was confirmed in the missions run: the client
  fired `GARRISON_FOLLOWER_XP_CHANGED` for it.

## Scope

In scope:

- Loading `GarrFollowerLevelXP` and `GarrItemLevelUpgradeData`.
- Experience: from successful missions (base plus the team's bonus) and
  from tomes; levels to 110; then quality steps to 5.
- Item level from armaments, at level 110.
- Equipment: putting a trait in an unlocked slot, empty or filled.
- The equipment table, its load, save, delete and the character dump.
- The messages that tell the client.

Out of scope:

- Single-mission abilities (effect 244 with flag 0x40), items used on
  missions, troop vitality (piece 3).
- Effects 229, 233, 287; quality 6.
- Warlords followers.
- The items themselves, their drops and vendors (content).
- GM commands or other test helpers.

## Rules this work follows

- Engine commits contain core code and schema SQL only.
- New code goes in new files where it is new; shared files get small hook
  changes only (spell effect handlers, the mission completion).
- No warnings, no debug code, no TODO comments.
- Small commits, each booted and tested in game before the next.
- SQL files are named by the server's date, which is UTC.
- This document lives on `dev`, never on the engine branch.
- Test rows are planted by hand, never committed, and removed at the end.
  The test character is put back exactly as it was.

## Design

### 1. Stores

`GarrFollowerLevelXP` and `GarrItemLevelUpgradeData` are loaded like the
other garrison tables (structure, load info, store, hotfix statements and
tables), in the column order their metas give.

### 2. Experience

`ClassHallFollowers::AddExperience(dbId, amount, source)`:

- Only champions; a troop gains nothing.
- Below the cap, the experience fills the current level's
  `GarrFollowerLevelXP` step; each level up carries the remainder, so one
  gain can give several levels.
- At level 110, the experience fills the quality's `XpToNextQuality`;
  each step raises the quality, carries the remainder, and re-derives the
  abilities with `GetAbilities` (which may unlock an ability or a slot).
- At a quality with no next step, experience stays 0 and the rest is
  dropped.
- The client is told with `SMSG_GARRISON_FOLLOWER_CHANGED_XP` (earned,
  source, the follower before and after); a quality change also sends the
  quality message if the probe shows the client needs it.

From missions: in `CompleteMission`, on a success only, after the roll and
before the reply, each champion on the mission gains
`GarrMission.BaseFollowerXP` plus the team's bonus. The bonus follows the
rule the probe recovers from the client's `xpBonus`.

From tomes (effect 231): the spell's amount, to the targeted champion.

### 3. Item level

Effect 80, on the targeted champion at level 110, for each effect of the
spell, by its upgrade row's operation:

- Add (2, 3): the value rises by the amount, up to MaxItemLevel.
- Raise (0, 1): the value rises to the amount; it is never lowered.

If neither value would change (cap reached, or already at or above the
target), or the champion is below 110, the cast is refused before the
item is used. The client is told with
`SMSG_GARRISON_FOLLOWER_CHANGED_ITEM_LEVEL`.

### 4. Equipment

Effect 244 for a trait ability (flag 0x1, follower type 4), on the
targeted champion:

- The client names the slot to fill (settled by the probe); only unlocked
  slots count (the quality's trait count).
- The trait goes in that slot, replacing what is there.
- Refused before the item is used: a locked slot, a trait the champion
  already has, an ability that is not a trait.
- `GetAbilities` sends, for each unlocked slot, the equipped trait or the
  empty-slot placeholder of that slot.
- The client is told with the abilities message the probe identifies.

### 5. Targeting

The client puts an item cast into a follower targeting mode and then names
the follower in the cast (and the slot for equipment); where in the cast
packet is settled by the probe's packet log. For all three effects the
server checks before the item is used: the target is one of the caster's
class hall followers, of the spell's follower type, a champion, active,
and not on a mission. Otherwise the cast fails with a bad-target result.

### 6. Storage

New table `character_class_hall_follower_equipment` (`guid`, `dbId`,
`slot`, `abilityId`; primary key `dbId`, `slot`), one row per filled slot,
loaded and saved with the followers, deleted with the character and with
the follower, and added to the character dump with the follower dbIds
remapped through the same map as `character_class_hall_follower`.

### 7. The probe

One login before the first build, with the packet log on:

- Use an armament, a tome and an equipment item on a champion (planted
  items): the casts show where the follower and the slot are named.
- Hand-send 0x2914 and 0x2904 with a changed follower: which one the
  client takes for quality and for abilities.
- The probe add-on reads `xpBonus` for chosen teams on the table, as the
  success chance probe did, to recover the bonus rule.

## Commit sequence

1. `feat(engine,sql)`: load the follower level and item level upgrade tables
2. `feat(engine)`: add the follower change messages
3. `feat(engine)`: give class hall followers experience
4. `feat(engine)`: give experience for a successful mission
5. `feat(engine)`: use items on class hall champions (targeting, tomes, armaments)
6. `feat(engine,sql)`: equip class hall champions

The probe sits before commit 1. Commit 2's message layouts and commit 5's
targeting depend on it.

## Error handling

- Saved equipment row for a follower that is not held, a slot above 2, or
  an ability that is not a trait: skipped, one error; the next save drops
  it.
- Saved equipment in a slot the follower's quality has not unlocked (only
  by hand): kept, not sent, until the quality unlocks it.
- An item cast with no valid target, or one that would change nothing:
  fails before the item is used.
- Experience for a follower that is not held, or a troop: ignored.

## Verification

As before: no unit tests; the proof is in the client. The engine branch is
merged into a throwaway `integration/class-hall-follower-growth` branch cut
from `feature/priest-class-hall`.

Test setup, by hand, never committed: Gairy's hall, planted champions at
chosen levels and qualities, planted items, planted missions made due.

Checks, in a few short logins:

1. A successful mission gives each champion the experience the table
   showed; a failed one gives none; troops gain nothing.
2. Enough experience levels a champion up, several levels at once.
3. At 110, experience raises the quality, and the new quality shows its
   unlocked ability or slot.
4. Each armament kind: an add, a raise, a refusal at the cap, a refusal
   below 110.
5. A tome gives its experience.
6. Equipment fills an empty slot, replaces a filled one, and is refused
   for a locked slot or a duplicate.
7. A relog keeps level, quality, item level, experience and equipment.

## Open points

- **Experience from a failed mission**: none, by choice; no source found.
- **The team's experience bonus**: found by the probe.
- **Where the cast names the follower and the slot**: found by the probe.
- **Which message tells the client of a quality or abilities change**:
  found by the probe.
- **Which effect of an armament is the weapon and which the armor**:
  found by the probe; both move together, so only the stored columns
  differ.
- **The parked mission bug**: the client drops the mission complete reply,
  so the report's experience bars do not animate; the experience itself
  arrives in its own message.

## What comes next

`engine/class-hall-mission-extras`: troop vitality and deaths, threats
that double cost or duration, single-mission abilities, items used on
missions, the overmax bonus roll. Its own spec, plan and branch, stacked
on this one.
