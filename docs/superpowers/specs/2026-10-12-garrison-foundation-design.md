# Garrison foundation: more than one garrison per character

Date: 2026-10-12
Branch: `engine/garrison-foundation`, cut from `main`
Status: design approved in conversation, awaiting review of this document

## Goal

A character can hold a Warlords garrison and a Legion class hall at the same
time. Both are saved, both load at login, and the client is told about both.

This is the first of five engine pieces needed for class halls. It adds no
class hall feature a player can see. It makes the later pieces possible:
hall upgrades (talents), follower progression, troop shipments and recruits,
and missions.

## Why this is needed

The garrison code in ArgusCore was written for Warlords only:

- `Garrison::GetType()` always returns the Warlords type.
- `Player` holds exactly one garrison.
- `character_garrison` has one row per character.
- Saving a garrison first deletes every garrison row the character owns.

The client data already describes the class hall, and the core's own spell
effect would already try to create one:

- Spell 185506 "Create Garrison: Artifact" creates garrison site 161.
- Site 161 has one usable level (site level 560) on the Broken Isles map
  (1220) and has no plots.
- `GarrType.db2` says type 3 (class hall) uses maps 1220 and 1669, and type 2
  (Warlords) uses map 1116.

Cast today, 185506 would replace the character's Warlords garrison with an
object that reports itself as a Warlords garrison.

## Scope

In scope:

- Working out a garrison's type from client data.
- `Garrison` knowing its own type.
- Saving and loading garrisons per type.
- `Player` holding one garrison per type.
- The garrison info reply listing every garrison.
- Achievement checks that ask for a specific garrison type finding it.

Out of scope, each handled later:

- Adding followers to the right garrison (followers spec).
- Talents, shipments, recruits, missions (their own specs).
- Which quest casts 185506 (content commit on the priest hall branch).
- GM commands or other test helpers (`chore(local)` commits on `dev`).

## Rules this work follows

From the upstream `ROADMAP.md`:

- Never rewrite large systems at once.
- Never change behaviour unless intended.
- Tiny incremental commits, each booted and tested in game.

From our branch model:

- Engine commits contain core code and schema SQL only. No NPC flags, gossip
  or scripts.
- Shared files (`Player.cpp`, `CharacterHandler.cpp`) get small hook changes.
- No warnings, no debug code, no TODO comments.
- This document lives on `dev`, never on the engine branch.

## Design

### 1. Type resolution

Load `GarrType.db2` as a normal DB2 store: entry struct, load info, hotfix
statement, `LOAD_DB2` line and the matching hotfixes table. The metadata for
it already exists in `DB2Metadata.h`.

Add one lookup to `GarrisonMgr`: given a site id, return its garrison type.

1. Take the site's level 1 entry from `GarrSiteLevel`.
2. Find the `GarrType` row whose map list contains that entry's map, or that
   map's parent map.
3. If none matches, the site has no type.

Results from the data we have:

| Site | Level 1 map | Parent map | Type |
|------|-------------|------------|------|
| 2, 71 (Warlords) | garrison instance map | 1116 | 2 |
| 161 (class hall) | 1220 | none | 3 |
| 163 (purpose unknown) | 1463 | not in any list | none |

Nothing uses the lookup yet after this commit.

### 2. `Garrison` stores its type

`Garrison` gains a type member. `Create` sets it from the lookup above.
`LoadFromDB` sets it from the type the row was loaded for. `GetType()`
returns the member.

`Create` fails when the site has no level 1 entry or no type. On failure it
sends `GarrisonCreateResult` with `GARRISON_ERROR_INVALID_SITE_ID` or
`GARRISON_ERROR_INVALID_GARRISON_TYPE`. Both codes already exist in
`Garrison.h`. Today `Create` returns without telling the client.

### 3. Persistence

Add a `garrisonType` column, default 2, to four tables:

| Table | Primary key before | Primary key after |
|-------|--------------------|-------------------|
| `character_garrison` | `guid` | `guid, garrisonType` |
| `character_garrison_blueprints` | `guid, buildingId` | `guid, garrisonType, buildingId` |
| `character_garrison_buildings` | `guid, plotInstanceId` | `guid, garrisonType, plotInstanceId` |
| `character_garrison_followers` | `dbId` | unchanged |

`character_garrison_follower_abilities` is not changed. Its rows belong to a
follower by `dbId`, and the follower row carries the type.

All four tables are empty on our server, and the default keeps any existing
Warlords rows on another server valid.

Statements:

- Select, insert and the save-time delete statements gain a type parameter.
- The delete-by-owner statements stay as they are and are used only when a
  character is deleted.
- The daily activation reset statement already updates every row and is not
  changed.

Behaviour:

- `Garrison::SaveToDB` deletes and re-inserts only rows of its own type.
- `Garrison::Delete` removes only rows of its own type.
- The static `Garrison::DeleteFromDB(guid)` still removes everything the
  character owns.

Sections 2 and 3 land as one commit. The type member and the type column
have no use without each other.

### 4. `Player` holds garrisons by type

`Player::_garrison` becomes a small container keyed by garrison type.

- `GetGarrison()` keeps its signature and returns the Warlords garrison. The
  45 or so existing callers are not touched.
- `GetGarrison(GarrisonType)` is added beside it.
- `CreateGarrison(siteId)` creates the garrison and stores it under its own
  type. As today, creating a type the character already has replaces it.
- `DeleteGarrison()` keeps deleting the Warlords garrison. Nothing calls it
  today, so no type-aware version is added.

Every place that acted on the one garrison now loops over all of them:

- Login load. The login query holder gets three more queries for the class
  hall: garrison, followers, follower abilities. Class halls have no plots,
  so the blueprint and building results are passed empty, which `LoadFromDB`
  already accepts.
- Save.
- Daily follower activation reset.
- Remote info on map change and after a seamless teleport.

A saved row whose site level no longer exists is skipped and logged, as it
is today.

### 5. Garrison info lists every garrison

`GetGarrisonInfoResult` already carries a list of garrisons. Today
`Garrison::SendInfo` builds the whole reply with one entry.

Split it: each `Garrison` fills its own entry, and a `Player` function
builds one reply holding every garrison the character has. The info request
handler and the login path call the `Player` function.

A character with only a Warlords garrison gets the same reply as before.

### 6. Enter and Leave guard

`Garrison::Enter` and `Garrison::Leave` teleport between a garrison's
instance map and its parent map. A class hall's map has no parent.

Both become no-ops for a garrison whose site map has no parent map. No
current caller reaches them for a class hall, because the Draenor area
triggers use `GetGarrison()`. The guard keeps a later caller from
teleporting a player to an invalid map.

### 7. Criteria look up the requested type

Seventeen modifier-tree checks in `CriteriaHandler.cpp` take the player's
garrison and then compare its type with the type the achievement data asks
for. They change to ask the player for the garrison of that type.

Warlords achievements behave as before. Checks that ask for a class hall
could never pass until now and can pass after this commit. That is the one
intended behaviour change in this spec.

The other garrison criteria checks do not name a type and stay on
`GetGarrison()`.

## Commit sequence

Each commit builds without warnings, boots, and is checked in game before
the next one starts.

1. `feat(engine,sql)`: load `GarrType.db2` and add the site type lookup
2. `feat(engine,sql)`: garrison type column; `Garrison` stores its type
3. `feat(engine)`: `Player` holds garrisons by type
4. `feat(engine)`: garrison info lists every garrison
5. `fix(engine)`: Enter and Leave only for garrisons with a parent map
6. `fix(engine)`: criteria look up the requested garrison type

After commit 2 a class hall can be created and is saved under type 3, but it
still replaces the Warlords garrison in memory, as it would today, and it is
not loaded again at login. Commit 3 fixes both.

SQL files follow `YYYY_MM_DD_NN_<db>.sql` under `sql/updates/<db>/master/`.
The number is chosen when the commit is made, because another session is
adding world updates on the same dates.

## Error handling

- Unknown site: creation fails with `GARRISON_ERROR_INVALID_SITE_ID`.
- Site with no resolvable type: creation fails with
  `GARRISON_ERROR_INVALID_GARRISON_TYPE`.
- Saved row with a missing site level: skipped at login and logged.
- Enter or Leave on a garrison without a parent map: nothing happens.

## Verification

There are no unit tests in this piece. Every change is database, packet or
player plumbing, and the proof is in the client.

The engine branch cannot run on its own, because the Docker setup lives only
on `dev`. Each commit is merged into `dev` (or the priest hall branch) to
build and test.

Checked after every commit:

- The server boots and the DBErrors count stays at the 13,422 baseline.
- A character with no garrison logs in and plays as before.

Checked from commit 3 onward, on Gairy:

1. `.cast 185506`
2. `/dump C_Garrison.HasGarrison(3)` prints `true`.
3. `/dump C_Garrison.GetLandingPageGarrisonType()` prints `3`.
4. Relog. Steps 2 and 3 still hold, and `character_garrison` has a row with
   `garrisonType = 3` and `siteLevelId = 560`.
5. `.cast 156020` to add a Warlords garrison. `HasGarrison(2)` and
   `HasGarrison(3)` both print `true`.
6. Relog. Both still print `true`, and `character_garrison` has two rows.

The database checks in steps 4 and 6 must pass from commit 3. The `/dump`
checks after a relog depend on the info reply and must pass from commit 4.

Gairy's garrison rows are removed by hand after testing if the priest hall
content work needs a clean character.

## Open points

- The minimap landing page button may need more than the garrison existing.
  It is not a pass criterion here.
- Site 163 and its spell 192191 have no known use. The lookup gives the site
  no type, so that spell fails cleanly.
- Sending every garrison in one reply is a reading of the packet's list
  layout. Verification step 5 confirms it. If the client wants one reply per
  garrison, commit 4 changes to send them separately and nothing else moves.

## What comes next

Each is its own spec, plan and branch, stacked on this one:

1. `engine/garrison-talents`: hall upgrades and research
2. `engine/garrison-followers`: follower routing by type, levels, gear
3. `engine/garrison-shipments`: troop work orders and recruits
4. `engine/garrison-missions`: missions and rewards
