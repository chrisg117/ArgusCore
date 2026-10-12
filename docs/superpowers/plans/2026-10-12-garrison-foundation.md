# Garrison Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A character can hold a Warlords garrison and a Legion class hall at the same time, with both saved, loaded and reported to the client.

**Architecture:** The existing `Garrison` class gains a type member, resolved from `GarrType.db2`. The character tables gain a `garrisonType` column so each garrison saves and deletes only its own rows. `Player` holds one garrison per type; `GetGarrison()` keeps returning the Warlords one so existing callers are untouched.

**Tech Stack:** C++ (ArgusCore, TrinityCore-derived, Legion 7.3.5 build 26972), MySQL 8.0, Docker Compose build on host `172.30.1.185`, WoW 7.3.5 client for in-game checks.

**Spec:** `docs/superpowers/specs/2026-10-12-garrison-foundation-design.md` (on `dev`)

## Global Constraints

- Engine commits contain core code and schema SQL only. No NPC flags, gossip, scripts, GM commands, debug code or TODO comments.
- One commit per task commit listed, with the exact message given. Every commit ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Engine commits are made only in the worktree `~/arguscore-engine` on branch `engine/garrison-foundation`, cut from `main`.
- All image builds run from `~/arguscore` and nowhere else. The build cache trusts file times, so building a second directory would leave stale objects behind.
- Never pass `--no-cache` to a build. Never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while any character has `online = 1`.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` under `sql/updates/<db>/master/`. Use the commit date and the lowest `NN` not used for that date and database on `main`, `dev` or `feature/priest-class-hall`.
- Do not use LegionCore or any other leak-derived core as a reference.
- Do not push any branch and do not open upstream issues or pull requests. Pushing is the user's call in Task 6.
- No new compiler warnings in touched files.
- `GetGarrison()` with no argument must keep returning the Warlords garrison.
- Steps marked **[user]** are done by the user in the game client. Stop and wait for the result.

## Review Focus

1. Warlords rows written before the schema change must come out as type 2. Pinned in Task 2, Step 2 and Step 9.
2. A follower with no ability rows must not break saving. The existing delete statement joins on abilities and would leave such a follower behind, so the re-insert would fail. Pinned in Task 2, Step 4 (statement) and Task 3, Step 8 (test).
3. Casting the create spell for a type the character already holds must leave exactly one row for that type and no SQL error. Pinned in Task 3, Step 9.
4. A saved row whose site level does not exist must be skipped and logged, and the login must succeed. Pinned in Task 2, Step 5 (code) and Task 3, Step 7 (test).
5. A map change with a class hall held must not teleport or disconnect the character. Pinned in Task 3, Step 10.

## File map

| File | Change | Task |
|------|--------|------|
| `sql/updates/hotfixes/master/<date>_<NN>_hotfixes.sql` | create: `garr_type` table | 1 |
| `src/server/game/DataStores/DB2Structure.h` | add `GarrTypeEntry` | 1 |
| `src/server/game/DataStores/DB2LoadInfo.h` | add `GarrTypeLoadInfo` | 1 |
| `src/server/game/DataStores/DB2Stores.h`, `.cpp` | add `sGarrTypeStore` | 1 |
| `src/server/database/Database/Implementation/HotfixDatabase.h`, `.cpp` | add `HOTFIX_SEL_GARR_TYPE` | 1 |
| `src/server/game/Garrison/GarrisonMgr.h`, `.cpp` | add `GetGarrTypeEntryForSite` | 1 |
| `sql/updates/characters/master/<date>_<NN>_characters.sql` | create: `garrisonType` column | 2 |
| `src/server/database/Database/Implementation/CharacterDatabase.h`, `.cpp` | typed statements | 2 |
| `src/server/game/Garrison/Garrison.h`, `.cpp` | type member, typed save, create errors; later `BuildInfo`, `Leave` guard | 2, 4, 5 |
| `src/server/game/Handlers/CharacterHandler.cpp` | type parameter on login queries; later three class hall queries | 2, 3 |
| `src/server/game/Entities/Player/Player.h`, `.cpp` | garrisons by type; later `SendGarrisonInfo` | 2, 3, 4 |
| `src/server/game/Handlers/MovementHandler.cpp` | call `SendGarrisonRemoteInfo` | 3 |
| `src/server/game/Handlers/GarrisonHandler.cpp` | call `SendGarrisonInfo` | 4 |
| `src/server/game/Achievements/CriteriaHandler.cpp` | 16 type lookups | 5 |

Paths are relative to the repository root. Line numbers below are from `main` at `cace9934`.

---

### Task 0: Workspace and baseline

**Files:**
- Create (host, outside the repo): `~/garrison-foundation/build.sh`, `~/garrison-foundation/boot_check.sh`

**Interfaces:**
- Produces: `build.sh` (merge the engine branch into the integration branch and build the image), `boot_check.sh` (restart the world server and report the boot). Every later task runs these two.

- [ ] **Step 1: Check the preconditions**

Run on the host:

```bash
cd ~/arguscore && git status -sb && pgrep -x docker-buildx; docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT name, online FROM characters.characters" 2>/dev/null
```

Expected: branch `feature/priest-class-hall` with no modified or untracked files, no `docker-buildx` process, every character `online = 0`.

If the tree is not clean or a build is running, stop. Another session is using this tree; the user decides when it is free.

- [ ] **Step 2: Create the two branches**

```bash
cd ~/arguscore
git worktree add ~/arguscore-engine -b engine/garrison-foundation main
git switch -c integration/garrison-foundation feature/priest-class-hall
```

`integration/garrison-foundation` is a throwaway branch: the priest hall content plus the engine commits, so the server can be built and tested. It is deleted in Task 6.

- [ ] **Step 3: Write `~/garrison-foundation/build.sh`**

```sh
#!/bin/sh
# Merges the engine branch into the integration branch and builds the image.
set -e
d=~/garrison-foundation
cd ~/arguscore
[ "$(git branch --show-current)" = integration/garrison-foundation ] || { echo "wrong branch"; exit 1; }
if pgrep -x docker-buildx >/dev/null; then echo "another build is running"; exit 1; fi
git merge --no-edit engine/garrison-foundation
if ! BUILDKIT_PROGRESS=plain docker compose build worldserver > $d/build.log 2>&1; then
    tail -40 $d/build.log; echo "BUILD FAILED"; exit 1
fi
echo "== warnings in touched files =="
grep "warning:" $d/build.log | grep -E "Garrison|DB2|HotfixDatabase|CharacterDatabase|Player\.|CharacterHandler|MovementHandler|CriteriaHandler" || echo "none"
echo "BUILD OK"
```

Run it under `nohup` with output to `~/garrison-foundation/build.out` and poll that file for `BUILD OK` or `BUILD FAILED`. Tasks 1 to 3 change widely included headers, so expect most of the core to recompile.

- [ ] **Step 4: Write `~/garrison-foundation/boot_check.sh`**

```sh
#!/bin/sh
# Restarts the world server on the current image and reports the boot.
d=~/garrison-foundation
logs=~/arguscore-server/logs
cd ~/arguscore
online=$(docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT COUNT(*) FROM characters.characters WHERE online = 1" 2>/dev/null)
[ "$online" = 0 ] || { echo "a character is online"; exit 1; }
before=$(wc -l < $logs/DBErrors.log)
since=$(date -u +%Y-%m-%dT%H:%M:%SZ)
docker compose up -d --force-recreate worldserver 2>&1 | tail -1
i=0
while [ $i -lt 100 ]; do
    if docker compose logs --since "$since" worldserver 2>/dev/null | grep -q "daemon) ready"; then echo "READY after ~$((i * 3)) s"; break; fi
    i=$((i + 1)); sleep 3
done
[ $i -lt 100 ] || echo "NOT READY after 300 s"
echo "== updates applied =="
docker compose logs --since "$since" worldserver 2>/dev/null | grep -i "Applying update" | sed "s/^[^|]*| //"
growth=$(( $(wc -l < $logs/DBErrors.log) - before ))
grep -o "\[\(ERROR\|WARN\)\].*" $logs/Server.log | sort -u > $d/srv_now.txt
if [ ! -f $d/dberr_base.txt ]; then echo $growth > $d/dberr_base.txt; cp $d/srv_now.txt $d/srv_base.txt; echo "BASELINE recorded: $growth"; exit 0; fi
echo "== DBErrors growth: $growth (baseline $(cat $d/dberr_base.txt)) =="
echo "== Server.log ERROR/WARN lines not in the baseline =="
diff $d/srv_base.txt $d/srv_now.txt && echo "none"
```

- [ ] **Step 5: Record the baseline**

Run: `sh ~/garrison-foundation/boot_check.sh`
Expected: `READY`, then `BASELINE recorded: 13422` (or a number close to it; whatever it prints is the baseline for this plan).

No commit in this task.

---

### Task 1: Load `GarrType.db2` and resolve a site's type

**Files:**
- Create: `sql/updates/hotfixes/master/<date>_<NN>_hotfixes.sql`
- Modify: `src/server/game/DataStores/DB2Structure.h` (after `GarrTalentTreeEntry`, line 1563)
- Modify: `src/server/game/DataStores/DB2LoadInfo.h` (after `GarrTalentTreeLoadInfo`, line 1795)
- Modify: `src/server/game/DataStores/DB2Stores.h:110`, `DB2Stores.cpp:128`, `DB2Stores.cpp:643`
- Modify: `src/server/database/Database/Implementation/HotfixDatabase.h:335`, `HotfixDatabase.cpp:535`
- Modify: `src/server/game/Garrison/GarrisonMgr.h`, `GarrisonMgr.cpp:88`

**Interfaces:**
- Produces: `GarrTypeEntry` (fields below), `sGarrTypeStore`, and `GarrTypeEntry const* GarrisonMgr::GetGarrTypeEntryForSite(uint32 garrSiteId) const`, which returns `nullptr` when the site has no type. `GarrTypeEntry::ID` is the garrison type (2 or 3).

- [ ] **Step 1: Write the hotfixes update**

```sql
-- GarrType.db2: one row per garrison type (2 = Warlords garrison, 3 = Legion class hall).
-- The core reads it to tell which type a garrison site belongs to. The table only holds hotfix
-- overrides; the rows themselves come from the client file.
CREATE TABLE IF NOT EXISTS `garr_type` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Flags` int unsigned NOT NULL DEFAULT '0',
  `PrimaryCurrencyTypeID` int unsigned NOT NULL DEFAULT '0',
  `SecondaryCurrencyTypeID` int unsigned NOT NULL DEFAULT '0',
  `ExpansionID` int unsigned NOT NULL DEFAULT '0',
  `MapIDs1` int NOT NULL DEFAULT '0',
  `MapIDs2` int NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
```

- [ ] **Step 2: Add the store**

The field order was read from the client file (rows: `2 → 0, 824, 1101, 5, [1116, 0]` and `3 → 3, 1220, 0, 6, [1220, 1669]`):

```cpp
struct GarrTypeEntry
{
    uint32 ID;
    uint32 Flags;
    uint32 PrimaryCurrencyTypeID;
    uint32 SecondaryCurrencyTypeID;
    uint32 ExpansionID;
    int32 MapIDs[2];
};
```

Follow `GarrTalentTree` in every file it appears in:

- `GarrTypeLoadInfo`: seven fields named `ID`, `Flags`, `PrimaryCurrencyTypeID`, `SecondaryCurrencyTypeID`, `ExpansionID`, `MapIDs1`, `MapIDs2`; the first five unsigned `FT_INT`, the two map fields signed `FT_INT`; meta `&GarrTypeMeta::Instance` (already in `DB2Metadata.h:5095`); statement `HOTFIX_SEL_GARR_TYPE`.
- `HotfixDatabase.h`: `HOTFIX_SEL_GARR_TYPE` and `HOTFIX_SEL_GARR_TYPE_MAX_ID`.
- `HotfixDatabase.cpp`: `"SELECT ID, Flags, PrimaryCurrencyTypeID, SecondaryCurrencyTypeID, ExpansionID, MapIDs1, MapIDs2 FROM garr_type WHERE (`VerifiedBuild` > 0) = ?"` and the matching `PREPARE_MAX_ID_STMT`.
- `DB2Stores`: `sGarrTypeStore("GarrType.db2", &GarrTypeLoadInfo::Instance)`, its `extern` line and its `LOAD_DB2` line.

- [ ] **Step 3: Add `GarrTypeEntry const* GarrisonMgr::GetGarrTypeEntryForSite(uint32 garrSiteId) const`**

```cpp
GarrTypeEntry const* GarrisonMgr::GetGarrTypeEntryForSite(uint32 garrSiteId) const
{
    GarrSiteLevelEntry const* siteLevel = GetGarrSiteLevelEntry(garrSiteId, 1);
    if (!siteLevel)
        return nullptr;

    MapEntry const* map = sMapStore.LookupEntry(siteLevel->MapID);
    for (GarrTypeEntry const* garrType : sGarrTypeStore)
        for (int32 mapId : garrType->MapIDs)
            if (mapId > 0 && (mapId == int32(siteLevel->MapID) || (map && mapId == map->ParentMapID)))
                return garrType;

    return nullptr;
}
```

A `0` in `MapIDs` is an empty slot, which is why it is skipped. Expected results: sites 2 and 71 give type 2 (level 1 maps 1158 and 1152, parent 1116), site 161 gives type 3 (map 1220), site 163 gives `nullptr` (map 1463).

- [ ] **Step 4: Commit in `~/arguscore-engine`**

```bash
git add sql/updates/hotfixes/master src/server/game/DataStores src/server/database/Database/Implementation/HotfixDatabase.h src/server/database/Database/Implementation/HotfixDatabase.cpp src/server/game/Garrison/GarrisonMgr.h src/server/game/Garrison/GarrisonMgr.cpp
git commit -m "feat(engine,sql): load GarrType.db2 and resolve a garrison site's type"
```

- [ ] **Step 5: Build**

Run: `sh ~/garrison-foundation/build.sh`
Expected: `== warnings in touched files ==` followed by `none`, then `BUILD OK`.

- [ ] **Step 6: Boot and check**

Run: `sh ~/garrison-foundation/boot_check.sh`
Expected: `READY`; the new hotfixes file listed under `updates applied`; DBErrors growth equal to the baseline; `none` under the Server.log section.

Run: `docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SHOW TABLES IN hotfixes LIKE 'garr_type'" 2>/dev/null`
Expected: `garr_type`

Nothing calls the lookup yet. Task 2 exercises it.

---

### Task 2: Type column, and `Garrison` stores its type

**Files:**
- Create: `sql/updates/characters/master/<date>_<NN>_characters.sql`
- Modify: `src/server/database/Database/Implementation/CharacterDatabase.h:566-580`, `CharacterDatabase.cpp:681-695`
- Modify: `src/server/game/Garrison/Garrison.h:33` (enum), `:232` (`GetType`), members; `Garrison.cpp:34-48`, `:144`, `:227-255`
- Modify: `src/server/game/Handlers/CharacterHandler.cpp:287-305`
- Modify: `src/server/game/Entities/Player/Player.cpp:17949`

**Interfaces:**
- Consumes: `GarrisonMgr::GetGarrTypeEntryForSite` from Task 1.
- Produces:
  - `enum GarrisonType : uint8` (same values as today).
  - `bool Garrison::LoadFromDB(GarrisonType type, PreparedQueryResult garrison, PreparedQueryResult blueprints, PreparedQueryResult buildings, PreparedQueryResult followers, PreparedQueryResult abilities)`
  - `GarrisonType Garrison::GetType() const` returning the stored type.
  - Every `CHAR_SEL_CHARACTER_GARRISON*` statement takes `(guid, garrisonType)`.

- [ ] **Step 1: Write the characters update**

```sql
-- A character can hold more than one garrison (Warlords garrison = 2, Legion class hall = 3).
-- Each garrison row and its blueprints, buildings and followers are now keyed by garrison type.
-- Existing rows are Warlords garrisons and take the default.
-- Plain ALTER, as in 2026_08_13_14_world.sql: the updater applies each file once.
ALTER TABLE `character_garrison`
  ADD COLUMN `garrisonType` TINYINT UNSIGNED NOT NULL DEFAULT 2 AFTER `guid`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`guid`, `garrisonType`);

ALTER TABLE `character_garrison_blueprints`
  ADD COLUMN `garrisonType` TINYINT UNSIGNED NOT NULL DEFAULT 2 AFTER `guid`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`guid`, `garrisonType`, `buildingId`);

ALTER TABLE `character_garrison_buildings`
  ADD COLUMN `garrisonType` TINYINT UNSIGNED NOT NULL DEFAULT 2 AFTER `guid`,
  DROP PRIMARY KEY,
  ADD PRIMARY KEY (`guid`, `garrisonType`, `plotInstanceId`);

ALTER TABLE `character_garrison_followers`
  ADD COLUMN `garrisonType` TINYINT UNSIGNED NOT NULL DEFAULT 2 AFTER `guid`;
```

- [ ] **Step 2: Plant a pre-change Warlords row (Review Focus 1)**

Before the build, with the old schema still in place:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "INSERT INTO characters.character_garrison (guid, siteLevelId, followerActivationsRemainingToday) VALUES (2, 5, 1)" 2>/dev/null
```

Guid 2 is Lindsey, who is not used for the in-game checks. Step 9 checks the row after the change and removes it.

- [ ] **Step 3: Change the select and insert statements**

| Statement | New SQL |
|-----------|---------|
| `CHAR_SEL_CHARACTER_GARRISON` | `SELECT siteLevelId, followerActivationsRemainingToday FROM character_garrison WHERE guid = ? AND garrisonType = ?` |
| `CHAR_INS_CHARACTER_GARRISON` | `INSERT INTO character_garrison (guid, garrisonType, siteLevelId, followerActivationsRemainingToday) VALUES (?, ?, ?, ?)` |
| `CHAR_SEL_CHARACTER_GARRISON_BLUEPRINTS` | `SELECT buildingId FROM character_garrison_blueprints WHERE guid = ? AND garrisonType = ?` |
| `CHAR_INS_CHARACTER_GARRISON_BLUEPRINTS` | `INSERT INTO character_garrison_blueprints (guid, garrisonType, buildingId) VALUES (?, ?, ?)` |
| `CHAR_SEL_CHARACTER_GARRISON_BUILDINGS` | `SELECT plotInstanceId, buildingId, timeBuilt, active FROM character_garrison_buildings WHERE guid = ? AND garrisonType = ?` |
| `CHAR_INS_CHARACTER_GARRISON_BUILDINGS` | `INSERT INTO character_garrison_buildings (guid, garrisonType, plotInstanceId, buildingId, timeBuilt, active) VALUES (?, ?, ?, ?, ?, ?)` |
| `CHAR_SEL_CHARACTER_GARRISON_FOLLOWERS` | same column list, `WHERE guid = ? AND garrisonType = ?` |
| `CHAR_INS_CHARACTER_GARRISON_FOLLOWERS` | `garrisonType` added after `guid`, twelve parameters |
| `CHAR_SEL_CHARACTER_GARRISON_FOLLOWER_ABILITIES` | `... WHERE gf.guid = ? AND gf.garrisonType = ? ORDER BY gfab.slot` |

`CHAR_INS_CHARACTER_GARRISON_FOLLOWER_ABILITIES` and `CHAR_UPD_CHARACTER_GARRISON_FOLLOWER_ACTIVATIONS` do not change.

- [ ] **Step 4: Add four type-filtered delete statements**

The four existing `CHAR_DEL_CHARACTER_GARRISON*` statements stay exactly as they are. They are used only when a character is deleted.

| New statement | SQL |
|---------------|-----|
| `CHAR_DEL_CHARACTER_GARRISON_BY_TYPE` | `DELETE FROM character_garrison WHERE guid = ? AND garrisonType = ?` |
| `CHAR_DEL_CHARACTER_GARRISON_BLUEPRINTS_BY_TYPE` | `DELETE FROM character_garrison_blueprints WHERE guid = ? AND garrisonType = ?` |
| `CHAR_DEL_CHARACTER_GARRISON_BUILDINGS_BY_TYPE` | `DELETE FROM character_garrison_buildings WHERE guid = ? AND garrisonType = ?` |
| `CHAR_DEL_CHARACTER_GARRISON_FOLLOWERS_BY_TYPE` | `DELETE gfab, gf FROM character_garrison_followers gf LEFT JOIN character_garrison_follower_abilities gfab ON gfab.dbId = gf.dbId WHERE gf.guid = ? AND gf.garrisonType = ?` |

The followers statement uses `LEFT JOIN` on purpose, so a follower with no ability rows is still deleted (Review Focus 2).

- [ ] **Step 5: Give `Garrison` its type**

- `Garrison.h`: `enum GarrisonType : uint8`; a private `GarrisonType _type` member, initialised to `GARRISON_TYPE_GARRISON` in the constructor; `GetType()` returns `_type`.
- `LoadFromDB` takes `GarrisonType type` as its first parameter and stores it before anything else.
- When `LoadFromDB` finds no `GarrSiteLevel` entry for the saved id, it logs one error to `entities.player` naming the owner's guid, the garrison type and the site level id, then returns `false` (Review Focus 4). Match the format style of the other `TC_LOG_ERROR` calls in `src/server/game/Garrison/`.
- Add a private `void DeleteOwnRowsFromDB(CharacterDatabaseTransaction trans) const` that appends the four `_BY_TYPE` statements with `(owner guid, _type)`.
- `SaveToDB` calls `DeleteOwnRowsFromDB` where it called `DeleteFromDB`, and passes `_type` to the four changed insert statements.
- `Delete` calls `DeleteOwnRowsFromDB` where it called `DeleteFromDB`.
- The static `DeleteFromDB(ObjectGuid::LowType, CharacterDatabaseTransaction)` is not changed.

- [ ] **Step 6: Make `Garrison::Create` resolve the type and report failures**

`Create(uint32 garrSiteId)`, in this order:

1. No level 1 site level: send `GarrisonCreateResult` with `Result = GARRISON_ERROR_INVALID_SITE_ID`, return `false`.
2. `sGarrisonMgr.GetGarrTypeEntryForSite(garrSiteId)` is `nullptr`: send `GarrisonCreateResult` with `Result = GARRISON_ERROR_INVALID_GARRISON_TYPE`, return `false`.
3. Otherwise set `_type = GarrisonType(garrType->ID)` and continue as today.

- [ ] **Step 7: Update the two callers**

- `CharacterHandler.cpp:287-305`: each of the five garrison statements gets `stmt->setUInt8(1, GARRISON_TYPE_GARRISON);`. Add `#include "Garrison.h"`.
- `Player.cpp:17950`: pass `GARRISON_TYPE_GARRISON` as the first argument of `LoadFromDB`.

Confirm `Player.cpp:4116` still calls the static `Garrison::DeleteFromDB(guid, trans)`.

- [ ] **Step 8: Commit, build, boot**

```bash
git add sql/updates/characters/master src/server/database/Database/Implementation/CharacterDatabase.h src/server/database/Database/Implementation/CharacterDatabase.cpp src/server/game/Garrison/Garrison.h src/server/game/Garrison/Garrison.cpp src/server/game/Handlers/CharacterHandler.cpp src/server/game/Entities/Player/Player.cpp
git commit -m "feat(engine,sql): store a garrison's type and key its rows by it"
```

Run: `sh ~/garrison-foundation/build.sh` — expected `none`, `BUILD OK`.
Run: `sh ~/garrison-foundation/boot_check.sh` — expected `READY`, the characters file under `updates applied`, growth equal to the baseline, `none`.

- [ ] **Step 9: Check the migrated row (Review Focus 1)**

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT guid, garrisonType, siteLevelId FROM characters.character_garrison; DELETE FROM characters.character_garrison WHERE guid = 2" 2>/dev/null
```

Expected: `2	2	5`

- [ ] **Step 10: [user] Create a class hall**

On Gairy: `.cast self 185506 triggered`, then `.save`.

Then on the host:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT guid, garrisonType, siteLevelId FROM characters.character_garrison" 2>/dev/null
```

Expected: `1	3	560`

If there is no row, stop and report. The spell's target needs a look before going on.

- [ ] **Step 11: [user] An unknown site fails cleanly**

On Gairy: `.cast self 192191 triggered`, then `.save`.
Expected: the character stays connected, and the query from Step 10 still prints only `1	3	560`.

Leave the type 3 row in place. Task 3 loads it. Ask the user to log Gairy out.

---

### Task 3: `Player` holds garrisons by type

**Files:**
- Modify: `src/server/game/Entities/Player/Player.h:78`, `:922-926`, `:2774-2776`, `:3239`
- Modify: `src/server/game/Entities/Player/Player.cpp:17949-17955`, `:19869`, `:23924`, `:24396`, `:28301-28315`
- Modify: `src/server/game/Handlers/CharacterHandler.cpp:305`
- Modify: `src/server/game/Handlers/MovementHandler.cpp:176`

**Interfaces:**
- Consumes: `Garrison::LoadFromDB(GarrisonType, ...)`, `Garrison::GetType()` from Task 2.
- Produces, on `Player`:
  - `Garrison* GetGarrison() const;` returns the Warlords garrison.
  - `Garrison* GetGarrison(GarrisonType type) const;` returns `nullptr` when the character has none of that type.
  - `void SendGarrisonRemoteInfo() const;`
  - `std::map<GarrisonType, std::unique_ptr<Garrison>> _garrisons;` (private)
  - Login query slots `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL`, `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL_FOLLOWERS`, `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL_FOLLOWER_ABILITIES`.

- [ ] **Step 1: Change the declarations in `Player.h`**

- Beside `class Garrison;` add the opaque declaration `enum GarrisonType : uint8;`.
- Replace `_garrison` with `_garrisons` as declared above. A `std::map` keeps the Warlords garrison first whenever the garrisons are walked in order.
- `GetGarrison()` moves out of the header; add the overload and `SendGarrisonRemoteInfo`.
- Add the three login query slots directly after `PLAYER_LOGIN_QUERY_LOAD_GARRISON_FOLLOWER_ABILITIES`.

- [ ] **Step 2: Implement the accessors and creation in `Player.cpp`**

- `GetGarrison()` returns `GetGarrison(GARRISON_TYPE_GARRISON)`.
- `CreateGarrison(uint32 garrSiteId)` stores a successfully created garrison under `garrison->GetType()`, replacing any garrison of that type.
- `DeleteGarrison()` deletes and erases the Warlords garrison only.
- `SendGarrisonRemoteInfo()` calls `SendRemoteInfo()` on every garrison.

- [ ] **Step 3: Loop where the single garrison was used**

- `Player.cpp:19869` (save): every garrison's `SaveToDB(trans)`.
- `Player.cpp:23924` (map change): `SendGarrisonRemoteInfo()`.
- `Player.cpp:24396` (daily reset): every garrison's `ResetFollowerActivationLimit()`.
- `MovementHandler.cpp:176`: `player->SendGarrisonRemoteInfo()` in place of the two lines there.

- [ ] **Step 4: Load the class hall at login**

- `CharacterHandler.cpp`, after line 305: three more queries with `(lowGuid, GARRISON_TYPE_CLASS_ORDER)` using `CHAR_SEL_CHARACTER_GARRISON`, `CHAR_SEL_CHARACTER_GARRISON_FOLLOWERS` and `CHAR_SEL_CHARACTER_GARRISON_FOLLOWER_ABILITIES`, set on the three new slots.
- `Player.cpp:17949`: the existing block stores into `_garrisons[GARRISON_TYPE_GARRISON]`. A second block loads `GARRISON_TYPE_CLASS_ORDER` from the three new slots, passing `nullptr` for blueprints and buildings, and stores into `_garrisons[GARRISON_TYPE_CLASS_ORDER]`.

- [ ] **Step 5: Commit, build, boot**

```bash
git add src/server/game/Entities/Player/Player.h src/server/game/Entities/Player/Player.cpp src/server/game/Handlers/CharacterHandler.cpp src/server/game/Handlers/MovementHandler.cpp
git commit -m "feat(engine): let a player hold one garrison per garrison type"
```

Run: `sh ~/garrison-foundation/build.sh` — expected `none`, `BUILD OK`.
Run: `sh ~/garrison-foundation/boot_check.sh` — expected `READY`, growth equal to the baseline, `none`.

- [ ] **Step 6: [user] Both garrisons persist**

On Gairy: log in, `.cast self 156020 triggered`, `.save`, log out, log in, `.save`, log out.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT guid, garrisonType, siteLevelId FROM characters.character_garrison ORDER BY garrisonType" 2>/dev/null
```

Expected:

```
1	2	5
1	3	560
```

- [ ] **Step 7: A bad site level is skipped and logged (Review Focus 4)**

With Gairy offline:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "UPDATE characters.character_garrison SET siteLevelId = 999999 WHERE guid = 1 AND garrisonType = 3" 2>/dev/null
```

**[user]** Log Gairy in, then out.

```bash
grep -c "999999" ~/arguscore-server/logs/Server.log
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "UPDATE characters.character_garrison SET siteLevelId = 560 WHERE guid = 1 AND garrisonType = 3" 2>/dev/null
```

Expected: the login succeeds and `grep` prints `1`. This also proves the class hall queries run at login.

- [ ] **Step 8: A follower with no abilities saves cleanly (Review Focus 2)**

With Gairy offline:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "INSERT INTO characters.character_garrison_followers (dbId, guid, garrisonType, followerId, quality, level, itemLevelWeapon, itemLevelArmor, xp, currentBuilding, currentMission, status) VALUES (9000001, 1, 3, 856, 4, 110, 760, 760, 0, 0, 0, 0)" 2>/dev/null
```

**[user]** Log Gairy in, `.save`, `.save`, log out.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT COUNT(*) FROM characters.character_garrison_followers WHERE dbId = 9000001; DELETE FROM characters.character_garrison_followers WHERE dbId = 9000001" 2>/dev/null
grep -c "Duplicate entry" ~/arguscore-server/logs/Server.log ~/arguscore-server/logs/DBErrors.log
```

Expected: count `1`, and `0` duplicate-entry lines in both logs since this boot.

- [ ] **Step 9: [user] Creating a held type again (Review Focus 3)**

On Gairy: log in, `.cast self 185506 triggered`, `.save`.
Expected: the query from Step 6 prints the same two rows, and Server.log has no new SQL error.

- [ ] **Step 10: [user] Map change (Review Focus 5)**

On Gairy: take a portal or `.tele` to a different continent and back.
Expected: no disconnect, and the character arrives where the teleport points.

Ask the user to log Gairy out.

---

### Task 4: Garrison info lists every garrison

**Files:**
- Modify: `src/server/game/Garrison/Garrison.h:262`, `Garrison.cpp:533-556`
- Modify: `src/server/game/Entities/Player/Player.h`, `Player.cpp`
- Modify: `src/server/game/Handlers/GarrisonHandler.cpp:23-27`

**Interfaces:**
- Consumes: `Player::_garrisons` from Task 3.
- Produces: `void Garrison::BuildInfo(WorldPackets::Garrison::GarrisonInfo& info)` and `void Player::SendGarrisonInfo()`. `Garrison::SendInfo` is removed.

- [ ] **Step 1: Split `Garrison::SendInfo`**

`BuildInfo` fills one `GarrisonInfo` with exactly what `SendInfo` filled between lines 540 and 553. It sends nothing.

- [ ] **Step 2: Add `Player::SendGarrisonInfo`**

It returns without sending when the character has no garrison. Otherwise it builds one `GetGarrisonInfoResult`, sets `FactionIndex` from the first garrison's `GetFaction()`, appends one entry per garrison in `_garrisons` order, and sends it.

- [ ] **Step 3: Call it from the handler**

`HandleGetGarrisonInfo` calls `_player->SendGarrisonInfo()`.

- [ ] **Step 4: Commit, build, boot**

```bash
git add src/server/game/Garrison/Garrison.h src/server/game/Garrison/Garrison.cpp src/server/game/Entities/Player/Player.h src/server/game/Entities/Player/Player.cpp src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine): list every garrison in the garrison info reply"
```

Run: `sh ~/garrison-foundation/build.sh` — expected `none`, `BUILD OK`.
Run: `sh ~/garrison-foundation/boot_check.sh` — expected `READY`, growth equal to the baseline, `none`.

- [ ] **Step 5: [user] The client knows both garrisons after a relog**

On Gairy, after logging in:

| Command | Expected |
|---------|----------|
| `/dump C_Garrison.HasGarrison(3)` | `true` |
| `/dump C_Garrison.HasGarrison(2)` | `true` |
| `/dump C_Garrison.GetLandingPageGarrisonType()` | `3` |

If `HasGarrison(3)` prints `false`: set `PacketLogFile = "world.pkt"` in `~/arguscore-server/configs/worldserver.conf`, restart, relog, and check whether the client sent `CMSG_GET_GARRISON_INFO`. If it did and the reply held both garrisons, change `SendGarrisonInfo` to send one reply per garrison (the spec's fallback) and repeat this step. Set `PacketLogFile` back to `""` afterwards.

- [ ] **Step 6: [user] A character with no garrison is unaffected**

Log in on Lindsey: `/dump C_Garrison.HasGarrison(3)` prints `false`, and the character plays normally.

---

### Task 5: `Leave` guard and criteria type lookups

Two commits, one build. Neither change can be reached in game on its own, so they share a regression check.

**Files:**
- Modify: `src/server/game/Garrison/Garrison.cpp:296-301`
- Modify: `src/server/game/Achievements/CriteriaHandler.cpp:2367-2882`

**Interfaces:**
- Consumes: `Player::GetGarrison(GarrisonType)` from Task 3.

- [ ] **Step 1: Guard `Garrison::Leave`**

`Leave` returns without teleporting when the site map's `ParentMapID` is negative. `Enter` is not changed: its own comparison of the owner's map with `ParentMapID` can never be true for a map without a parent.

- [ ] **Step 2: Commit**

```bash
git add src/server/game/Garrison/Garrison.cpp
git commit -m "fix(engine): do not teleport out of a garrison whose map has no parent"
```

- [ ] **Step 3: Change the 16 criteria checks**

Each of these sites reads the player's garrison and then rejects it when `garrison->GetType() != GarrisonType(<asset>)`. Change each to `referencePlayer->GetGarrison(GarrisonType(<asset>))` and drop the type comparison. Keep the `const` each site already uses and every other condition on the line.

| Line of the type comparison | Modifier tree type | Asset holding the type |
|------|------|------|
| 2370 | 126 | `secondaryAsset` |
| 2436 | 131 | `tertiaryAsset` |
| 2452 | 132 | `tertiaryAsset` |
| 2471 | 133 | `tertiaryAsset` |
| 2489 | 134 | `tertiaryAsset` |
| 2507 | 135 | `secondaryAsset` |
| 2518 | 137 | `reqValue` |
| 2537 | the case that contains this line | `tertiaryAsset` |
| 2553 | 142 | `tertiaryAsset` |
| 2571 | 143 | `secondaryAsset` |
| 2598 | the case that contains this line | `secondaryAsset` |
| 2642 | 146 | `secondaryAsset` |
| 2778 | 166 | `reqValue` |
| 2824 | 170 | `secondaryAsset` |
| 2852 | 175 | `tertiaryAsset` |
| 2881 | 178 | `reqValue` |

Run: `grep -c "GetType() != GarrisonType" src/server/game/Achievements/CriteriaHandler.cpp`
Expected: `0`

The other 14 `GetGarrison()` calls in this file name no type and are not changed.

- [ ] **Step 4: Commit, build, boot**

```bash
git add src/server/game/Achievements/CriteriaHandler.cpp
git commit -m "fix(engine): criteria look up the garrison type the data asks for"
```

Run: `sh ~/garrison-foundation/build.sh` — expected `none`, `BUILD OK`.
Run: `sh ~/garrison-foundation/boot_check.sh` — expected `READY`, growth equal to the baseline, `none`.

- [ ] **Step 5: [user] Regression check**

On Gairy: log in, open the achievements window, repeat the three `/dump` commands from Task 4 Step 5.
Expected: no disconnect, and the same three results.

---

### Task 6: Land it

- [ ] **Step 1: Merge the engine branch into `dev`, and `dev` into the priest branch**

With Gairy offline and no build running:

```bash
cd ~/arguscore
git switch dev && git merge --no-ff --no-edit engine/garrison-foundation
git switch feature/priest-class-hall && git merge --no-edit dev
git diff --stat integration/garrison-foundation feature/priest-class-hall -- src sql
```

Expected: the `git diff` prints nothing, which means the tree that was tested is the tree now on the priest branch. No rebuild is needed.

- [ ] **Step 2: Remove the throwaway branch**

```bash
git branch -D integration/garrison-foundation
```

Keep the worktree `~/arguscore-engine`. The next engine branch is stacked on this one.

- [ ] **Step 3: Ask the user about Gairy's garrisons**

The Warlords garrison on Gairy exists only for this test. With Gairy offline, remove it:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "DELETE FROM characters.character_garrison WHERE guid = 1 AND garrisonType = 2" 2>/dev/null
```

Ask whether the class hall row should stay. The priest hall content will create it through a quest later, so the user may want Gairy clean.

- [ ] **Step 4: Ask the user which branches to push**

Candidates: `engine/garrison-foundation`, `dev`, `feature/priest-class-hall`. Push only the ones the user names.

- [ ] **Step 5: Report**

State which checks passed, any that did not, and the three open points from the spec with what the tests showed for each.
