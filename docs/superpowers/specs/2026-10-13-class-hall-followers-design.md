# Class hall followers: owning champions and troops

Date: 2026-10-13
Branch: `engine/class-hall-followers`, stacked on `engine/class-hall-talents`
Status: awaiting review
Builds on: `2026-10-12-class-hall-foundation-design.md`,
`2026-10-13-class-hall-talents-design.md`

## Goal

A character with a class hall can gain followers (champions and troops),
keep them across logins, see them in the client's follower list, set a
champion active or inactive, and dismiss a follower.

This is the third of five engine pieces for class halls. Troop orders and
missions come after it and build on it: a finished troop order adds a troop
through this piece, and missions send these followers out.

## Decisions

Made with the owner on 2026-10-13:

- **Scope is owning followers.** Experience, levels, quality upgrades, gear
  and equipment, and combat allies are left out. Progression belongs with
  missions, where experience comes from.
- **A separate `ClassHallFollowers` class in its own files, owned by the
  hall,** with its own table. This is the decision made for talents,
  applied again. The Warlords `Garrison` class, its follower code and its
  tables are not changed.
- **Retail values, no server settings.** The limits, and the cost and daily
  limit of reactivating a champion, are the client's and retail's.

## What the client data says

`GarrFollower.db2` has 446 followers of garrison type 3 (class hall), all
of follower type 4. Fields used here: `ID`, `GarrTypeID`,
`GarrFollowerTypeID`, `ChrClassID`, `Quality`, `FollowerLevel`,
`ItemLevelWeapon`, `ItemLevelArmor`, `Vitality`, `Flags`, and the class
spec per faction (`AllianceGarrClassSpecID`, `HordeGarrClassSpecID`).

- **Champions** have `Vitality` 0. Level, quality and item level are fixed
  in the data (priest champions: level 100 to 110, item level 760, or 850
  for the one added in 7.2).
- **Troops** have a `Vitality` above 0 (2 or 3 for the priest's). A troop
  kind is a class spec (`GarrClassSpec.db2`) with a `FollowerClassLimit`;
  several followers share one kind and differ only in looks.
- Flag 0x1 is "unique" (the core's `GARRISON_FOLLOWER_FLAG_UNIQUE`). The
  priest's champions have it (`Flags` 3) and its troops do not (`Flags`
  2), so a character can own two troops with the same follower id. The
  other flag bits are not used by this piece.

The priest's followers:

| Kind | Followers | Limit |
|------|-----------|-------|
| Champions | 856, 857, 870 to 875, 1002 | 5 active |
| Acolytes (class spec 91) | 677, 678, 921 to 926 | 3 |
| Zealots (class spec 92) | 679, 920, 927 to 932, 943 | 2 |
| Hooded Priests (class spec 155) | 1039 to 1042 | 2 |

`GarrFollowerType.db2` has one row per follower type. Row 4 (class hall):
garrison type 3, at most 5 active followers, item level cap 950.

`GarrFollowerXAbility.db2` lists a follower's abilities per faction. A
priest champion has six rows per faction: its spec, two abilities and
three "Empty Equipment Slot" abilities (flag 0x1, a trait). A troop has
one. `GarrFollowerQuality.db2` says how many of them a follower has at
each quality: for follower type 4, quality 1 has 2 abilities and no
equipment slot, quality 2 has 3 and none, quality 3 has 3 and 1, quality 4
has 3 and 2, qualities 5 and 6 have 3 and 3. The spec counts as an ability.

`GarrClassSpecPlayerCond.db2` has up to two rows per troop kind, each with
a player condition and an order: for Acolytes, "Group of Acolytes"
(condition 42849, order 0) and "Netherlight Paragons" (41906, order 1).

A champion is granted by a spell: 219756 "Follower: Calia Menethil" has one
effect of type 220 (add garrison follower) whose value is the follower id,
856. The quest "Champion: Calia Menethil" (43271) casts it as its reward.
"Rise, Champions" (43270) then counts the recruit through criteria of type
176 (recruit garrison follower), whose asset is the follower id.

## What the core already has

- `GarrFollower.db2`, `GarrClassSpec.db2`, `GarrFollowerXAbility.db2` and
  `GarrAbility.db2` are loaded. `GarrFollowerType.db2`,
  `GarrFollowerQuality.db2` and `GarrClassSpecPlayerCond.db2` are not;
  their layouts are in `DB2Metadata.h`. `GarrisonMgr` indexes abilities of
  Warlords followers only.
- The follower entry of the garrison info reply (id, follower, quality,
  level, item levels, experience, durability, building, mission, abilities,
  zone support spell, status, custom name), and the list of followers in
  each garrison's entry. The class hall sends the list empty.
- `SMSG_GARRISON_ADD_FOLLOWER_RESULT` (0x2902) and
  `SMSG_GARRISON_REMOVE_FOLLOWER_RESULT` (0x2903), with packet classes.
- `GarrisonMgr`: the follower id generator (started from the highest id in
  `character_garrison_followers`) and the ability lookups.
- The spell effect 220 handler, which adds a follower to the Warlords
  garrison only.
- Criteria type 176 is matched against the follower id.
- Defined and unhandled: `CMSG_GARRISON_SET_FOLLOWER_INACTIVE` (0x32C5),
  `CMSG_GARRISON_REMOVE_FOLLOWER` (0x32F8),
  `CMSG_GARRISON_REQUEST_CLASS_SPEC_CATEGORY_INFO` (0x32D5),
  `SMSG_GARRISON_FOLLOWER_CHANGED_STATUS` (0x2915),
  `SMSG_GARRISON_NUM_FOLLOWER_ACTIVATIONS_REMAINING` (0x2917) and
  `SMSG_GARRISON_FOLLOWER_CHANGED_DURABILITY` (0x2904).
- The character dump does not include Warlords followers (the core marks
  it as not done): their ids are global and the dump cannot renumber
  them.
- The follower status flags: favorite 0x01, exhausted 0x02, inactive 0x04,
  troop 0x08, no experience gain 0x10.

Missing: the follower type store, storage, the routing of the grant spell,
the three request handlers and their replies.

## Scope

In scope:

- Loading `GarrFollowerType.db2`, `GarrFollowerQuality.db2` and (with the
  troop kinds) `GarrClassSpecPlayerCond.db2`.
- `ClassHallFollowers`, its table, and its place in the hall's load, save,
  delete and info reply.
- Gaining a follower from spell effect 220, with the rules below, and the
  recruit credit.
- Setting a champion active or inactive.
- Dismissing a follower.
- Answering the client's request for troop limits.

Out of scope:

- Experience, levels, quality, item level changes, equipment and ability
  changes.
- Combat allies and zone support spells.
- Missions, so vitality never drops in this piece.
- Troop orders and recruiting, the real source of troops. Troops are
  tested with rows planted by hand.
- Renaming, favorites, and assigning to buildings.
- Followers in the character dump. Their ids are global, and the dump has
  no renumbering for follower ids (it leaves the Warlords followers out
  for the same reason). A dumped character loads with its hall and talents
  and without followers.
- Anything for Warlords followers. The shared requests are answered for
  class hall followers only; a request about a Warlords follower is left
  as it is today (ignored).
- Spawning the hall's NPCs and the champion quests (content, on
  `feature/priest-class-hall`).
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

Load `GarrFollowerType.db2` and `GarrFollowerQuality.db2` like any other
store, each with its hotfix table. The first gives the limit of active
followers for the class hall, the second how many abilities and equipment
slots a follower shows at its quality. `GarrClassSpecPlayerCond.db2` is
loaded by the troop kinds commit, which is its only user.

### 2. `ClassHallFollowers`

New files: `src/server/game/ClassHall/ClassHallFollowers.h` and `.cpp`.

It belongs to one `ClassHall` and holds the character's followers by their
id. For each: the follower's data row and the values sent to the client
(quality, level, item levels, experience, durability, status).

It can:

- **Load** from its rows. A row whose follower does not exist in client
  data, or is not a class hall follower, is skipped with one logged error.
- **Save** its rows inside the player's save transaction.
- **Delete** all rows of a character.
- **Fill** the follower list of the hall's info entry.
- **Add** a follower, applying the rules in section 4.
- **Set** a champion active or inactive (section 5).
- **Remove** a follower (section 6).
- **Count** followers of a troop kind, and active champions.

`ClassHall` owns one and forwards load, save, delete and the info entry to
it.

A follower's abilities are not saved. They are worked out from client
data when the follower is loaded or added: of the rows
`GarrFollowerXAbility.db2` lists for it and the character's faction, in id
order, the first N that are not traits and the first M that are, where N
and M are the ability and trait counts `GarrFollowerQuality.db2` gives for
the follower's type and quality. Nothing in this piece changes them.

### 3. Storage

One new table, and one new column on `character_class_hall` (section 5).

`character_class_hall_follower`:

| Column | Type | Meaning |
|--------|------|---------|
| `dbId` | bigint unsigned | the follower's own id, unique across all characters |
| `guid` | bigint unsigned | the character |
| `followerId` | int unsigned | `GarrFollower` id |
| `quality` | int unsigned | |
| `level` | int unsigned | |
| `itemLevelWeapon` | int unsigned | |
| `itemLevelArmor` | int unsigned | |
| `xp` | int unsigned | |
| `durability` | int unsigned | a troop's remaining vitality; 0 for a champion |
| `status` | int unsigned | the follower status flags |

Primary key `dbId`, index on `guid`. There is no unique key on the
character and follower id, because a character can own two troops with the
same follower id.

Quality, level, item levels and experience are stored although nothing
changes them yet. They are part of what a follower is, they are sent to the
client, and the missions piece will change them; storing them now avoids
reshaping the table then.

Saving deletes the character's rows and inserts the current ones, inside
the player's save transaction, as the talents do. The table has no database
link to `character_class_hall`, for the same reason as the talents table.

The follower id generator in `GarrisonMgr` is shared with the Warlords
garrison, because the client identifies a follower by this id alone. It
starts from the highest id in either follower table. This is the one change
to `GarrisonMgr`.

The table is not added to the character dump list (see "Out of scope").
Character deletion removes its rows.

### 4. Gaining a follower

The spell effect 220 handler looks up the follower. If it belongs to a
class hall (`GarrTypeID` 3), the request goes to the character's class
hall; otherwise the existing path runs unchanged. This is the same routing
the foundation added to the "create garrison" effect.

Checked in this order. The first failure changes nothing and sends the "add
follower result" reply with an error.

| Check | Error |
|-------|-------|
| The character has a class hall | no garrison (1) |
| The follower exists and is a class hall follower | invalid follower (28) |
| The follower's class is the character's class, when it names one | invalid class (88) |
| A unique follower (a champion) is not already owned | follower exists (27) |
| A troop's kind is below its limit | at class spec cap (81) |

The garrison error list also has "follower inactive" (69), "follower
active" (70), "follower activation unavailable" (71) and "at follower hard
cap" (78), used in section 5.

On success a new follower gets the next id from the generator, the
quality, level and item levels of its data row, no experience, its
`Vitality` as durability, and the troop status flag if it has vitality. The
reply is sent with the follower, and the recruit criteria (type 176) is
updated with the follower id.

A champion gained while five are active: see "Open points".

### 5. Active and inactive

`CMSG_GARRISON_SET_FOLLOWER_INACTIVE` names a follower and the wanted
state.

- Only a champion of this hall can be changed. A troop cannot.
- Setting inactive is free and always allowed.
- Setting active is refused with "at follower hard cap" when the limit of
  active champions (5, from `GarrFollowerType`) is reached. Retail charges
  a cost for it and allows it a limited number of times per day; both are
  followed, refusing with "not enough gold" and "follower activation
  unavailable" (see "Open points" for where the numbers come from).
- Asking for the state a champion already has is refused with "follower
  inactive" or "follower active".

The follower's status flag changes and the client is told. Two replies
could carry it: 0x2915, which this core calls "follower changed status"
and WowPacketParser "follower changed flags" (no parser exists), and
0x2904, which WowPacketParser calls "update follower" (a result and the
whole follower). Whichever the client follows is used; see "Open points".

How many activations are left today is kept on the hall, in a new column
`followerActivationsRemaining` of `character_class_hall`, sent in the
hall's info entry and after each activation (0x2917, "follower activations
set": the site and the number), and set back to the daily number when the
character's daily quests reset, where the Warlords garrison resets its
own.

### 6. Removing

`CMSG_GARRISON_REMOVE_FOLLOWER` names a follower. If it is a follower of
this hall and may be dismissed, it is dropped and
`SMSG_GARRISON_REMOVE_FOLLOWER_RESULT` is sent. Which followers may be
dismissed follows what the client offers (see "Open points").

### 7. Troop limits

`CMSG_GARRISON_REQUEST_CLASS_SPEC_CATEGORY_INFO` names a follower type.
The reply (WowPacketParser: "get class spec category info result", 0x2901,
a number this core does not define yet) lists, for each troop kind, the
class spec and one `GarrClassSpecPlayerCond` row. The client works out the
counts and limits itself from its follower list and its data.

For each troop kind that followers of the character's class use, the row
sent is the one with the highest order whose player condition the
character meets. A kind with no row met is left out.

## Commit sequence

Each commit builds without new warnings, boots, and is checked in game
before the next one starts.

1. `feat(engine,sql)`: load `GarrFollowerType.db2` and
   `GarrFollowerQuality.db2`
2. `feat(engine,sql)`: store class hall followers and report them to the
   client
3. `feat(engine)`: add the garrison follower packets
4. `feat(engine)`: add a class hall follower from its spell
5. `feat(engine,sql)`: set a class hall follower active or inactive
6. `feat(engine)`: dismiss a class hall follower
7. `feat(engine,sql)`: report the class hall's troop kinds

Three long builds: commits 1 and 2 (store and statement headers), commits
3 to 6 (the opcode list and the session header), and commit 7 (a store
header again). Commits that share a build are still tested one after the
other.

Commits 6 and 7 are the least certain. If the client's expectations turn
out to differ from this design, they move to a follow-up and the commits
before them stand.

## Error handling

- Saved follower that does not exist in client data, or is not a class
  hall follower: skipped, one error logged, login continues. The next save
  drops the row.
- Saved followers for a character whose hall row is missing or bad: not
  loaded, because the hall is not loaded. The rows stay untouched.
- More saved troops of a kind than its limit, or more than five active
  champions (possible only through a hand edit): all load as saved. The
  limits are applied to requests, not to loaded data.
- A grant spell for a follower the character cannot gain: refused with the
  error from section 4. Nothing changes.
- A request naming a follower the character does not own: refused, nothing
  changes.

## Verification

No unit tests, for the same reason as the earlier pieces: every change is
database, packet or player plumbing, and the proof is in the client.

The engine branch is merged into a throwaway
`integration/class-hall-followers` branch cut from
`feature/priest-class-hall`, and the server is built from that in
`~/arguscore`.

Tools that worked for talents and are used again:

- The packet log (`PacketLogFile`), to see what the client sent and each
  reply.
- `.debug send opcode` with a hand-written `opcode.txt`, to try a reply
  number and layout without a build.
- The client's own functions for the follower list, e.g.
  `C_Garrison.GetFollowers(4)`, and the hall report window.

Test setup, done by hand and never committed:

- Gairy is given a class hall row, as in the talents run.
- Champions are granted with their reward spells (`.cast self 219756
  triggered` and the like).
- Troops are rows planted while Gairy is offline.

Checked after every commit:

- The server boots and DBErrors growth is the recorded baseline.
- A character with no hall, and a character with a hall and no followers,
  log in and play as before.

Commit 2, with rows planted by hand:

1. Log in. The follower list shows the planted champion and troop with the
   right name, level, quality and item level, and the troop with its
   vitality.
2. `.save`, log out. The rows are unchanged.
3. A row with follower id `999999`, and one with a Warlords follower id:
   login succeeds, Server.log has one error for each.
4. A throwaway character with planted follower rows is deleted. Its rows
   are gone.

Commit 4:

1. Cast Calia's spell. She appears in the list without a relog, and after
   `.save` the table has her row.
2. With quest 43270 taken, the "Recruit Calia" objective is credited.
3. Cast it again. Refused; still one row.
4. Cast a champion spell of another class. Refused.
5. With a troop kind at its limit, a further troop of that kind is
   refused. (The troop is added by a spell if the client data has one for
   it; otherwise this rule is covered by review.)
6. A Warlords follower spell still reaches the Warlords path (read in
   review; there is no Warlords garrison on the test character).

Commit 5:

1. Set a champion inactive from the list, then active. The list follows
   without a relog and the status is saved.
2. With five active champions, activating a sixth is refused.
3. The cost and the daily limit behave as retail's.

Commit 6:

1. Dismiss a troop from the list. It is gone without a relog and its row
   is gone after `.save`.
2. What the client offers for a champion is recorded, and the server
   agrees with it.

Commit 7:

1. The troop counts and limits show where the client displays them.

After testing, every planted row is removed, the integration branch is
deleted, and Gairy is put back exactly as she was.

## Open points

Each is settled in planning or by the in-game check named, and the spec is
corrected in the same sitting if the answer differs.

- **Reply layouts.** From WowPacketParser: 0x2917 is the site and the
  number of activations; 0x2904 is a result and the follower; 0x2901 is
  the follower type and a list of (class spec, class spec condition row).
  0x2915 has no parser. Which of 0x2915 and 0x2904 the client follows for
  a status change is found by sending both by hand before the commit that
  needs it.
- **The request layouts.** From WowPacketParser: set inactive is the
  follower's id and a bit (6.x layout); the troop request is the follower
  type. The dismiss request has no parser. All three are read from the
  packet log before their handlers are written, since the log records
  requests the server does not handle yet.
- **Whether the client sends these requests without a mission NPC window
  open.** The research request needed its NPC. If these do too, the tests
  open the window by sending the "open mission NPC" reply by hand.
- **A champion gained while five are active:** whether retail adds it
  inactive or refuses it. Commit 4 or 5, from what the client shows.
- **The reactivation cost and daily limit:** whether the numbers are in
  client data or fixed, and what the client displays for them. Commit 5.
- **Which followers can be dismissed.** Commit 6, from what the client
  offers.
- **Whether a newly gained follower shows without a relog** with the
  existing "add follower result" reply for a class hall. Commit 4.
- **Abilities.** Whether the list worked out from the two data files is
  what retail shows for a fresh champion (its spec and one ability, no
  equipment slot) and for a troop. Commit 2.
- **The Warlords code passes the follower's own id** to the recruit
  criteria, where the criteria compares a follower id. That looks like a
  bug in the Warlords path; it is not touched here, and the class hall
  passes the follower id.

## What comes next

Each is its own spec, plan and branch, stacked on this one:

1. `engine/class-hall-shipments`: troop work orders and recruits
2. `engine/class-hall-missions`: missions, rewards, follower experience
   and gear, vitality loss
