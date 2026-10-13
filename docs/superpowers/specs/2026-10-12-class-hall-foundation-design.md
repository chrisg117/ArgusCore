# Class hall foundation: a class hall beside the garrison

Date: 2026-10-12
Branch: `engine/class-hall-foundation`, cut from `main`
Status: awaiting review
Replaces: `2026-10-12-garrison-foundation-design.md` (one `Garrison` class
holding both types), withdrawn the same day

## Goal

A character can have a Legion class hall. It is created by the retail
spell, saved, loaded at login, and reported to the client. A Warlords
garrison on the same character keeps working exactly as before.

This is the first of five engine pieces needed for class halls. It adds no
class hall feature a player can see. It gives the later pieces somewhere to
live: hall upgrades (talents), followers, troop shipments and recruits, and
missions.

## Decision: two separate systems

The class hall is a new `ClassHall` class in new files. The existing
`Garrison` class, its tables and its statements stay Warlords-only.

Reasons:

- Most of `Garrison` is plots, buildings, blueprints and an instanced map.
  A class hall has none of these.
- Missions and shipments do not exist for Warlords in this core, so there
  is no working code to share. Only basic follower handling would be
  duplicated.
- Nothing that exists is rewritten, which is what the upstream `ROADMAP.md`
  asks for: never rewrite large systems at once, never change behaviour
  unless intended.

Cost, accepted:

- Warlords gets nothing from the later pieces for free. Each later system
  goes in its own files, so a Warlords garrison could adopt it later.
- The client still treats both as garrisons and uses the same packets for
  both. The two systems therefore meet in three small places, listed under
  "Where the two systems meet".

## What the client data says

- Spell 185506 "Create Garrison: Artifact" creates garrison site 161.
- Site 161, level 1, is site level 560 on the Broken Isles map (1220). It
  has no plots.
- `GarrType.db2` has two rows. Type 2 (Warlords) lists map 1116. Type 3
  (class hall) lists maps 1220 and 1669.
- The Warlords sites (2 and 71) sit on instance maps whose parent map is
  1116.

Cast today, 185506 would replace the character's Warlords garrison with an
object that reports itself as a Warlords garrison on site 161.

## Scope

In scope:

- Working out from client data whether a site is a class hall.
- The `ClassHall` class and its one table.
- `Player` owning, loading, saving and deleting a class hall.
- The create-garrison spell effect making a class hall for a class hall
  site.
- The garrison info reply including the class hall.

Out of scope, each handled later:

- Followers, talents, shipments, recruits, missions (their own specs).
- Class hall achievement checks. The existing garrison checks stay
  Warlords-only.
- Which quest casts 185506 (content commit on the priest hall branch).
- GM commands or other test helpers (`chore(local)` commits on `dev`).

## Rules this work follows

- Engine commits contain core code and schema SQL only. No NPC flags,
  gossip or scripts.
- New code goes in new files. Shared files get small hook changes only.
- No warnings, no debug code, no TODO comments.
- Tiny commits, each booted and tested in game before the next.
- This document lives on `dev`, never on the engine branch.

## Design

### 1. Site type lookup

Load `GarrType.db2` as a normal DB2 store: entry struct, load info, hotfix
statement, `LOAD_DB2` line and the matching hotfixes table. Its metadata
already exists in `DB2Metadata.h`.

Add one lookup to `GarrisonMgr`: given a site id, return its `GarrType` row,
or nothing.

1. Take the site's level 1 entry from `GarrSiteLevel`.
2. Find the `GarrType` row whose map list contains that entry's map, or that
   map's parent map.

Results from the data:

| Site | Level 1 map | Parent map | Type |
|------|-------------|------------|------|
| 2 (Warlords, Alliance) | 1158 | 1116 | 2 |
| 71 (Warlords, Horde) | 1152 | 1116 | 2 |
| 161 (class hall) | 1220 | none | 3 |
| 163 (purpose unknown) | 1463 | none | none |

### 2. `ClassHall`

New files: `src/server/game/ClassHall/ClassHall.h` and `ClassHall.cpp`.

A `ClassHall` belongs to one player and knows its site level. In this piece
that is all it holds. It can:

- **Create** itself for a site. It takes the site's level 1 entry and sends
  the client a `GarrisonCreateResult` carrying the site level id.
- **Load** from its row.
- **Save** its row inside the player's save transaction.
- **Delete** all rows of a given character (used when a character is
  deleted).
- **Fill** one garrison entry of the info reply: type 3, site 161, site
  level 560, and empty lists for everything else.

It has no plots, buildings, blueprints, map, enter or leave.

### 3. Persistence

One new table. No existing table changes.

`character_class_hall`:

| Column | Type | Meaning |
|--------|------|---------|
| `guid` | bigint unsigned, primary key | the character |
| `siteLevelId` | int unsigned | `GarrSiteLevel` id, 560 today |

Three new statements: select by guid, replace (insert or overwrite) and
delete by guid.

`character_class_hall` is also added to the character dump table list, so a
dumped and re-imported character keeps its hall.

Later pieces add their own tables (followers, talents and so on) beside this
one. None of them reuse the `character_garrison_*` tables.

### 4. `Player`

`Player` gains one member, two small functions and three hook lines beside
the garrison ones:

- `GetClassHall()` returns the class hall or nothing.
- `CreateClassHall(siteId)` creates one. If the character already has a
  class hall, it is kept as it is and the client gets
  `GarrisonCreateResult` with `GARRISON_ERROR_GARRISON_EXISTS`. A second
  cast must never wipe a hall, because later it will hold followers and
  research.
- Login load, save and character deletion each get one added line for the
  class hall.

`GetGarrison()`, `CreateGarrison()` and `DeleteGarrison()` are not changed.

The login query holder gets one more query.

### 5. Where the two systems meet

Three places, each a few lines:

**The create-garrison spell effect.** It asks the lookup for the site's
type. A class hall site goes to `CreateClassHall`. Every other site goes to
`CreateGarrison`, exactly as today. Warlords behaviour does not depend on
the lookup being right.

**The garrison info reply.** The client asks once and expects one reply
listing every garrison. The reply is built in the request handler from both
systems. For that, `Garrison::SendInfo` is split in two: a part that fills
the garrison's own entry, which stays in `Garrison`, and the part that
builds and sends the reply, which moves to the handler. This is the only
change to existing garrison code, and a character with only a Warlords
garrison gets the same reply as before.

**Character deletion.** The class hall rows are deleted beside the garrison
rows.

## Commit sequence

Each commit builds without new warnings, boots, and is checked in game
before the next one starts.

1. `feat(engine,sql)`: load `GarrType.db2` and resolve a garrison site's type
2. `feat(engine,sql)`: add the class hall and its storage
3. `feat(engine)`: create a class hall for a class hall site
4. `feat(engine)`: include the class hall in the garrison info reply

After commit 2 a class hall row is loaded and saved, but nothing can create
one in game yet. Commit 2 is tested with a row planted by hand.

SQL files follow `YYYY_MM_DD_NN_<db>.sql` under `sql/updates/<db>/master/`.
The number is chosen when the commit is made, because another session is
adding updates on the same dates.

## Error handling

- Site with no level 1 entry: nothing is created.
- Second create for a character that already has a class hall: the hall is
  kept and the client gets `GARRISON_ERROR_GARRISON_EXISTS`.
- Saved row whose site level does not exist: the hall is not loaded, one
  error is logged, and the login continues.
- Typeless site (163): handled as today, by `CreateGarrison`. That path is
  not made better or worse here.

## Verification

There are no unit tests in this piece. Every change is database, packet or
player plumbing, and the proof is in the client.

The engine branch cannot run on its own, because the Docker setup lives only
on `dev`. For each commit the engine branch is merged into a throwaway
integration branch that also holds the priest hall content, and the server
is built from that.

Checked after every commit:

- The server boots, and the DBErrors count matches the count recorded before
  the first change.
- A character with no garrison and no class hall logs in and plays as
  before.

Commit 2, on Gairy:

1. With a row `(1, 560)` planted by hand: log in, `.save`, log out. The row
   is still there.
2. With the row changed to site level `999999`: the login succeeds and
   Server.log has one error naming it. Restore the row afterwards.

Commit 3, on Gairy, starting with no row:

1. `.cast self 185506 triggered`, `.save`. The table has `(1, 560)`.
2. Cast it again. The table still has exactly that row.
3. `.cast self 156020 triggered`, `.save`. `character_garrison` has a
   Warlords row, so the old path still works.

Commit 4, on Gairy, after a relog:

1. `/dump C_Garrison.HasGarrison(3)` prints `true`.
2. `/dump C_Garrison.HasGarrison(2)` prints `true`.
3. `/dump C_Garrison.GetLandingPageGarrisonType()` prints `3`.

Gairy's Warlords garrison row is removed by hand after testing. Whether the
class hall row stays is the user's call, since the priest hall content will
create it through a quest later.

## Open points

- The minimap landing page button may need more than the hall existing. It
  is not a pass criterion here.
- Sending both garrisons in one reply is a reading of the packet's list
  layout. Commit 4's checks confirm it. If the client wants one reply per
  garrison, the handler sends two, and nothing else changes.
- The reply's "follower activations remaining" field is sent as 0 for the
  class hall. The followers spec decides its real value.

## What comes next

Each is its own spec, plan and branch, stacked on this one. Each adds its
own files and tables to the class hall:

1. `engine/class-hall-talents`: hall upgrades and research
2. `engine/class-hall-followers`: champions and troops
3. `engine/class-hall-shipments`: troop work orders and recruits
4. `engine/class-hall-missions`: missions and rewards
