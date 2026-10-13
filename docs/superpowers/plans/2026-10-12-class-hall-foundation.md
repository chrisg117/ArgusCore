# Class Hall Foundation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A character can have a Legion class hall that is created by the retail spell, saved, loaded at login and reported to the client, while a Warlords garrison on the same character keeps working as before.

**Architecture:** A new `ClassHall` class in new files, with its own table, owned by `Player` beside the untouched `Garrison`. The two meet in three small places: the create-garrison spell effect routes by site type, the garrison info request handler builds one reply from both, and character deletion clears both.

**Tech Stack:** C++ (ArgusCore, TrinityCore-derived, Legion 7.3.5 build 26972), MySQL 8.0, Docker Compose build on host `172.30.1.185`, WoW 7.3.5 client for in-game checks.

**Spec:** `docs/superpowers/specs/2026-10-12-class-hall-foundation-design.md` (on `dev`)

## Global Constraints

- Engine commits contain core code and schema SQL only. No NPC flags, gossip, scripts, GM commands, debug code or TODO comments.
- One commit per task, with the exact message given. Every commit ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Engine commits are made only in the worktree `~/arguscore-engine` on branch `engine/class-hall-foundation`, cut from `main`.
- All image builds run from `~/arguscore` and nowhere else. The build cache trusts file times, so building a second directory would leave stale objects behind.
- Never pass `--no-cache` to a build. Never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while any character has `online = 1`.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` under `sql/updates/<db>/master/`. Use the commit date and the lowest `NN` not used for that date and database on `main`, `dev` or `feature/priest-class-hall`.
- Do not use LegionCore or any other leak-derived core as a reference.
- Do not push any branch and do not open upstream issues or pull requests. Pushing is the user's call in Task 5.
- No new compiler warnings in touched files.
- `Garrison`'s tables, statements and behaviour do not change. The only edit to existing garrison code is the `SendInfo` split in Task 4.
- New files carry the same licence header as `src/server/game/Garrison/Garrison.h`.
- Steps marked **[user]** are done by the user in the game client. Stop and wait for the result.

## Review Focus

1. A second create cast must keep the existing hall and leave exactly one row. Pinned in Task 3, Step 6.
2. A saved row whose site level does not exist must be skipped and logged, and the login must succeed. Pinned in Task 2, Step 7.
3. Deleting a character must remove its hall row. Pinned in Task 3, Step 8.
4. The info reply must be right for each mix: hall only, Warlords only, both, neither. Pinned in Task 4, Steps 5 to 7.
5. A character dump must include the hall row. Pinned in Task 3, Step 7.

## File map

| File | Change | Task |
|------|--------|------|
| `sql/updates/hotfixes/master/<date>_<NN>_hotfixes.sql` | create: `garr_type` table | 1 |
| `src/server/game/DataStores/DB2Structure.h` | add `GarrTypeEntry` | 1 |
| `src/server/game/DataStores/DB2LoadInfo.h` | add `GarrTypeLoadInfo` | 1 |
| `src/server/game/DataStores/DB2Stores.h`, `.cpp` | add `sGarrTypeStore` | 1 |
| `src/server/database/Database/Implementation/HotfixDatabase.h`, `.cpp` | add `HOTFIX_SEL_GARR_TYPE` | 1 |
| `src/server/game/Garrison/GarrisonMgr.h`, `.cpp` | add `GetGarrTypeEntryForSite` | 1 |
| `sql/updates/characters/master/<date>_<NN>_characters.sql` | create: `character_class_hall` table | 2 |
| `src/server/game/ClassHall/ClassHall.h`, `.cpp` | create: the class | 2, 3, 4 |
| `src/server/database/Database/Implementation/CharacterDatabase.h`, `.cpp` | three statements | 2 |
| `src/server/game/Entities/Player/Player.h`, `.cpp` | member, accessor, load, save, delete; later `CreateClassHall` | 2, 3 |
| `src/server/game/Handlers/CharacterHandler.cpp` | one login query | 2 |
| `src/server/game/Tools/PlayerDump.cpp` | one table line | 2 |
| `src/server/game/Spells/SpellEffects.cpp` | route by site type | 3 |
| `src/server/game/Garrison/Garrison.h`, `.cpp` | `SendInfo` becomes `BuildInfo` | 4 |
| `src/server/game/Handlers/GarrisonHandler.cpp` | build the reply from both | 4 |

Paths are relative to the repository root. Line numbers are from `main` at `cace9934`. `src/server/game/CMakeLists.txt` collects sources and include directories by itself, so the new folder needs no build file change.

---

### Task 0: Workspace and baseline

**Files:**
- Create (host, outside the repo): `~/class-hall-foundation/build.sh`, `~/class-hall-foundation/boot_check.sh`

**Interfaces:**
- Produces: `build.sh` (merge the engine branch into the integration branch and build the image), `boot_check.sh` (restart the world server and report the boot). Every later task runs these two.

- [ ] **Step 1: Check the preconditions**

```bash
cd ~/arguscore && git status -sb && pgrep -x docker-buildx; docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT name, online FROM characters.characters" 2>/dev/null
```

Expected: branch `feature/priest-class-hall` with no modified or untracked files, no `docker-buildx` process, every character `online = 0`.

If the tree is not clean or a build is running, stop. Another session is using this tree; the user decides when it is free.

- [ ] **Step 2: Create the two branches**

```bash
cd ~/arguscore
git worktree add ~/arguscore-engine -b engine/class-hall-foundation main
git switch -c integration/class-hall-foundation feature/priest-class-hall
```

`integration/class-hall-foundation` is a throwaway branch: the priest hall content plus the engine commits, so the server can be built and tested. It is deleted in Task 5.

- [ ] **Step 3: Write `~/class-hall-foundation/build.sh`**

```sh
#!/bin/sh
# Merges the engine branch into the integration branch and builds the image.
set -e
d=~/class-hall-foundation
cd ~/arguscore
[ "$(git branch --show-current)" = integration/class-hall-foundation ] || { echo "wrong branch"; exit 1; }
if pgrep -x docker-buildx >/dev/null; then echo "another build is running"; exit 1; fi
git merge --no-edit engine/class-hall-foundation
if ! BUILDKIT_PROGRESS=plain docker compose build worldserver > $d/build.log 2>&1; then
    tail -40 $d/build.log; echo "BUILD FAILED"; exit 1
fi
echo "== warnings in touched files =="
grep "warning:" $d/build.log | grep -E "ClassHall|Garrison|DB2|HotfixDatabase|CharacterDatabase|Player\.|PlayerDump|CharacterHandler|SpellEffects" || echo "none"
echo "BUILD OK"
```

Run it under `nohup` with output to `~/class-hall-foundation/build.out` and poll that file for `BUILD OK` or `BUILD FAILED`. Tasks 1 and 2 change widely included headers, so expect most of the core to recompile.

- [ ] **Step 4: Write `~/class-hall-foundation/boot_check.sh`**

```sh
#!/bin/sh
# Restarts the world server on the current image and reports the boot.
d=~/class-hall-foundation
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

Run: `sh ~/class-hall-foundation/boot_check.sh`
Expected: `READY`, then `BASELINE recorded: <number>`. That number is the baseline for this plan.

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
- Produces: `GarrTypeEntry` (fields below), `sGarrTypeStore`, and `GarrTypeEntry const* GarrisonMgr::GetGarrTypeEntryForSite(uint32 garrSiteId) const`, which returns `nullptr` when the site has no type. `GarrTypeEntry::ID` is the garrison type: 2 for a Warlords garrison, 3 for a class hall.

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

- [ ] **Step 3: Add the lookup to `GarrisonMgr`**

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

A `0` in `MapIDs` is an empty slot, which is why it is skipped. Expected results, checked against the client files: sites 2 and 71 give type 2 (level 1 maps 1158 and 1152, both with parent 1116), site 161 gives type 3 (map 1220), site 163 gives `nullptr` (map 1463, no parent).

- [ ] **Step 4: Commit in `~/arguscore-engine`**

```bash
git add sql/updates/hotfixes/master src/server/game/DataStores src/server/database/Database/Implementation/HotfixDatabase.h src/server/database/Database/Implementation/HotfixDatabase.cpp src/server/game/Garrison/GarrisonMgr.h src/server/game/Garrison/GarrisonMgr.cpp
git commit -m "feat(engine,sql): load GarrType.db2 and resolve a garrison site's type"
```

- [ ] **Step 5: Build**

Run: `sh ~/class-hall-foundation/build.sh`
Expected: `== warnings in touched files ==` followed by `none`, then `BUILD OK`.

- [ ] **Step 6: Boot and check**

Run: `sh ~/class-hall-foundation/boot_check.sh`
Expected: `READY`; the new hotfixes file listed under `updates applied`; DBErrors growth equal to the baseline; `none` under the Server.log section.

Run: `docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SHOW TABLES IN hotfixes LIKE 'garr_type'" 2>/dev/null`
Expected: `garr_type`

Nothing calls the lookup yet. Task 3 exercises it.

---

### Task 2: The class hall and its storage

**Files:**
- Create: `sql/updates/characters/master/<date>_<NN>_characters.sql`
- Create: `src/server/game/ClassHall/ClassHall.h`, `src/server/game/ClassHall/ClassHall.cpp`
- Modify: `src/server/database/Database/Implementation/CharacterDatabase.h:580`, `CharacterDatabase.cpp:695`
- Modify: `src/server/game/Entities/Player/Player.h:78`, `:926`, `:2776`, `:3239`
- Modify: `src/server/game/Entities/Player/Player.cpp:58`, `:4116`, `:17955`, `:19870`
- Modify: `src/server/game/Handlers/CharacterHandler.cpp:305`
- Modify: `src/server/game/Tools/PlayerDump.cpp:98`

**Interfaces:**
- Produces:

```cpp
class TC_GAME_API ClassHall
{
public:
    explicit ClassHall(Player* owner);

    bool LoadFromDB(PreparedQueryResult classHall);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

private:
    Player* _owner;
    GarrSiteLevelEntry const* _siteLevel;
};
```

- On `Player`: `ClassHall* GetClassHall() const`, private `std::unique_ptr<ClassHall> _classHall`, login slot `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL`.
- Statements `CHAR_SEL_CHARACTER_CLASS_HALL`, `CHAR_REP_CHARACTER_CLASS_HALL`, `CHAR_DEL_CHARACTER_CLASS_HALL`.

- [ ] **Step 1: Write the characters update**

```sql
-- Legion class hall: one row per character that has one. siteLevelId is a GarrSiteLevel.db2 id
-- (560 = site 161, level 1). Kept apart from the character_garrison tables, which hold the
-- Warlords garrison and its plots and buildings; a class hall has neither.
CREATE TABLE IF NOT EXISTS `character_class_hall` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `siteLevelId` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall, one row per character';
```

- [ ] **Step 2: Add the three statements**

After the garrison statements, all `CONNECTION_ASYNC`:

| Statement | SQL |
|-----------|-----|
| `CHAR_SEL_CHARACTER_CLASS_HALL` | `SELECT siteLevelId FROM character_class_hall WHERE guid = ?` |
| `CHAR_REP_CHARACTER_CLASS_HALL` | `REPLACE INTO character_class_hall (guid, siteLevelId) VALUES (?, ?)` |
| `CHAR_DEL_CHARACTER_CLASS_HALL` | `DELETE FROM character_class_hall WHERE guid = ?` |

- [ ] **Step 3: Write `ClassHall`**

- The constructor stores the owner and sets `_siteLevel` to `nullptr`.
- `LoadFromDB` returns `false` for an empty result. It looks the saved id up in `sGarrSiteLevelStore`. When there is no such entry it logs one error and returns `false` (Review Focus 2):

```cpp
TC_LOG_ERROR("entities.player", "ClassHall::LoadFromDB: player {} has a class hall with unknown GarrSiteLevel {}, not loaded.",
    _owner->GetGUID().ToString(), siteLevelId);
```

- `SaveToDB` appends `CHAR_REP_CHARACTER_CLASS_HALL` with the owner's guid counter and `_siteLevel->ID`.
- `DeleteFromDB` appends `CHAR_DEL_CHARACTER_CLASS_HALL` with the given guid.

- [ ] **Step 4: Hook it into `Player`**

- `Player.h`: forward declaration `class ClassHall;` beside `class Garrison;`; the accessor beside `GetGarrison()`; the member beside `_garrison`; the login slot directly after `PLAYER_LOGIN_QUERY_LOAD_GARRISON_FOLLOWER_ABILITIES`.
- `Player.cpp`: `#include "ClassHall.h"`. After the garrison load block (line 17955), load a `ClassHall` from the new slot and keep it only when `LoadFromDB` returns `true`. After the garrison save (line 19870), save the class hall when there is one. After `Garrison::DeleteFromDB(guid, trans)` (line 4116), call `ClassHall::DeleteFromDB(guid, trans)`.
- `CharacterHandler.cpp`, after line 305: one query using `CHAR_SEL_CHARACTER_CLASS_HALL` with `lowGuid`, set on the new slot.

- [ ] **Step 5: Add the table to the dump list**

`PlayerDump.cpp`: `{ "character_class_hall", DTT_CHAR_TABLE },` in alphabetical position, before `character_cuf_profiles`.

- [ ] **Step 6: Commit, build, boot**

```bash
git add sql/updates/characters/master src/server/game/ClassHall src/server/database/Database/Implementation/CharacterDatabase.h src/server/database/Database/Implementation/CharacterDatabase.cpp src/server/game/Entities/Player/Player.h src/server/game/Entities/Player/Player.cpp src/server/game/Handlers/CharacterHandler.cpp src/server/game/Tools/PlayerDump.cpp
git commit -m "feat(engine,sql): add the class hall and its storage"
```

Run: `sh ~/class-hall-foundation/build.sh` — expected `none`, `BUILD OK`.
Run: `sh ~/class-hall-foundation/boot_check.sh` — expected `READY`, the characters file under `updates applied`, growth equal to the baseline, `none`.

- [ ] **Step 7: A bad site level is skipped and logged (Review Focus 2)**

With Gairy offline:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "REPLACE INTO characters.character_class_hall (guid, siteLevelId) VALUES (1, 999999)" 2>/dev/null
```

**[user]** Log Gairy in, then out.

```bash
grep -c "unknown GarrSiteLevel 999999" ~/arguscore-server/logs/Server.log
```

Expected: the login succeeds and `grep` prints `1`. This also proves the login query runs.

- [ ] **Step 8: A good row loads and saves**

With Gairy offline:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "REPLACE INTO characters.character_class_hall (guid, siteLevelId) VALUES (1, 560)" 2>/dev/null
```

**[user]** Log Gairy in, `.save`, log out.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT guid, siteLevelId FROM characters.character_class_hall; DELETE FROM characters.character_class_hall WHERE guid = 1" 2>/dev/null
grep -c "unknown GarrSiteLevel" ~/arguscore-server/logs/Server.log
```

Expected: `1	560`, and `grep` still prints `1` (no new error line). The planted row is removed so that Task 3 starts with none.

---

### Task 3: Create a class hall for a class hall site

**Files:**
- Modify: `src/server/game/ClassHall/ClassHall.h`, `ClassHall.cpp`
- Modify: `src/server/game/Entities/Player/Player.h:2774`, `Player.cpp:28306`
- Modify: `src/server/game/Spells/SpellEffects.cpp:5378-5387`

**Interfaces:**
- Consumes: `GarrisonMgr::GetGarrTypeEntryForSite` (Task 1), `ClassHall` and `Player::_classHall` (Task 2).
- Produces: `bool ClassHall::Create(uint32 garrSiteId)` and `void Player::CreateClassHall(uint32 garrSiteId)`.

- [ ] **Step 1: Add `ClassHall::Create`**

It takes the site's level 1 entry with `sGarrisonMgr.GetGarrSiteLevelEntry(garrSiteId, 1)` and returns `false` when there is none. Otherwise it stores the entry, sends `GarrisonCreateResult` with `GarrSiteLevelID` set to the entry's id and `Result` left at `GARRISON_SUCCESS`, and returns `true`.

- [ ] **Step 2: Add `Player::CreateClassHall`**

When the character already has a class hall, it sends `GarrisonCreateResult` with `Result = GARRISON_ERROR_GARRISON_EXISTS` and changes nothing (Review Focus 1). Otherwise it creates a `ClassHall` and keeps it only when `Create` returns `true`.

- [ ] **Step 3: Route in the spell effect**

In `Spell::EffectCreateGarrison`, replacing the one `CreateGarrison` line:

```cpp
Player* player = unitTarget->ToPlayer();
GarrTypeEntry const* garrType = sGarrisonMgr.GetGarrTypeEntryForSite(effectInfo->MiscValue);
if (garrType && garrType->ID == GARRISON_TYPE_CLASS_ORDER)
    player->CreateClassHall(effectInfo->MiscValue);
else
    player->CreateGarrison(effectInfo->MiscValue);
```

Every site that is not a class hall site takes the `else` branch, which is today's behaviour. Add `#include "GarrisonMgr.h"` if the file does not have it.

- [ ] **Step 4: Commit, build, boot**

```bash
git add src/server/game/ClassHall src/server/game/Entities/Player/Player.h src/server/game/Entities/Player/Player.cpp src/server/game/Spells/SpellEffects.cpp
git commit -m "feat(engine): create a class hall for a class hall site"
```

Run: `sh ~/class-hall-foundation/build.sh` — expected `none`, `BUILD OK`.
Run: `sh ~/class-hall-foundation/boot_check.sh` — expected `READY`, growth equal to the baseline, `none`.

- [ ] **Step 5: [user] Create a class hall**

On Gairy: `.cast self 185506 triggered`, then `.save`.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT guid, siteLevelId FROM characters.character_class_hall; SELECT COUNT(*) FROM characters.character_garrison WHERE guid = 1" 2>/dev/null
```

Expected: `1	560`, then `0`. The second number shows the spell no longer makes a Warlords garrison out of a class hall site.

If there is no class hall row, stop and report. The spell's target needs a look before going on.

- [ ] **Step 6: [user] A second cast keeps the hall (Review Focus 1)**

On Gairy: `.cast self 185506 triggered`, then `.save`.
Expected: the character stays connected, the query from Step 5 prints the same result, and Server.log has no new SQL error.

- [ ] **Step 7: [user] The dump includes the hall (Review Focus 5)**

On Gairy: `.pdump write gairy_hall.dump Gairy`

```bash
grep -c "character_class_hall" ~/arguscore-server/gairy_hall.dump; rm ~/arguscore-server/gairy_hall.dump
```

Expected: `1`. If the file is not in `~/arguscore-server`, find it with `docker exec arguscore-worldserver-1 find / -name gairy_hall.dump -not -path "/proc/*"` and read it there.

- [ ] **Step 8: [user] Deleting a character removes its hall (Review Focus 3)**

Create a new character named `Halltest`, log it in, `.cast self 185506 triggered`, `.save`, log out.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT c.guid FROM characters.characters c JOIN characters.character_class_hall h ON h.guid = c.guid WHERE c.name = 'Halltest'" 2>/dev/null
```

Expected: one guid. Note it.

**[user]** Delete `Halltest` at the character selection screen.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT COUNT(*) FROM characters.character_class_hall WHERE guid = <noted guid>" 2>/dev/null
```

Expected: `0`

- [ ] **Step 9: [user] The Warlords path still works**

On Lindsey: `.cast self 156020 triggered`, `.save`, log out.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT guid, siteLevelId FROM characters.character_garrison; SELECT COUNT(*) FROM characters.character_class_hall WHERE guid = 2" 2>/dev/null
```

Expected: `2	5`, then `0`.

Ask the user to log every character out.

---

### Task 4: Include the class hall in the garrison info reply

**Files:**
- Modify: `src/server/game/Garrison/Garrison.h:262`, `Garrison.cpp:533-556`
- Modify: `src/server/game/ClassHall/ClassHall.h`, `ClassHall.cpp`
- Modify: `src/server/game/Handlers/GarrisonHandler.cpp:19-27`

**Interfaces:**
- Consumes: `Player::GetClassHall()` (Task 2).
- Produces: `void Garrison::BuildInfo(WorldPackets::Garrison::GarrisonInfo& info)` and `void ClassHall::BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const`. `Garrison::SendInfo` is removed; the handler was its only caller.

- [ ] **Step 1: Turn `Garrison::SendInfo` into `Garrison::BuildInfo`**

`BuildInfo` fills the given entry with exactly what `SendInfo` put into `garrison` between lines 540 and 553. It builds no packet and sends nothing.

- [ ] **Step 2: Add `ClassHall::BuildInfo`**

It sets `GarrTypeID = GARRISON_TYPE_CLASS_ORDER`, `GarrSiteID = _siteLevel->GarrSiteID` and `GarrSiteLevelID = _siteLevel->ID`. Every other field keeps its default.

- [ ] **Step 3: Build the reply in the handler**

```cpp
void WorldSession::HandleGetGarrisonInfo(WorldPackets::Garrison::GetGarrisonInfo& /*getGarrisonInfo*/)
{
    WorldPackets::Garrison::GetGarrisonInfoResult garrisonInfo;
    garrisonInfo.FactionIndex = _player->GetTeam() == HORDE ? GARRISON_FACTION_INDEX_HORDE : GARRISON_FACTION_INDEX_ALLIANCE;

    if (Garrison* garrison = _player->GetGarrison())
        garrison->BuildInfo(garrisonInfo.Garrisons.emplace_back());

    if (ClassHall const* classHall = _player->GetClassHall())
        classHall->BuildInfo(garrisonInfo.Garrisons.emplace_back());

    if (garrisonInfo.Garrisons.empty())
        return;

    SendPacket(garrisonInfo.Write());
}
```

A character with neither gets no reply, as today. A character with only a Warlords garrison gets the same single entry as today. Add `#include "ClassHall.h"`.

- [ ] **Step 4: Commit, build, boot**

```bash
git add src/server/game/Garrison/Garrison.h src/server/game/Garrison/Garrison.cpp src/server/game/ClassHall src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine): include the class hall in the garrison info reply"
```

Run: `sh ~/class-hall-foundation/build.sh` — expected `none`, `BUILD OK`.
Run: `sh ~/class-hall-foundation/boot_check.sh` — expected `READY`, growth equal to the baseline, `none`.

- [ ] **Step 5: [user] Hall only (Review Focus 4)**

On Gairy, after logging in:

| Command | Expected |
|---------|----------|
| `/dump C_Garrison.HasGarrison(3)` | `true` |
| `/dump C_Garrison.HasGarrison(2)` | `false` |
| `/dump C_Garrison.GetLandingPageGarrisonType()` | `3` |

If `HasGarrison(3)` prints `false`: set `PacketLogFile = "world.pkt"` in `~/arguscore-server/configs/worldserver.conf`, restart, relog, and check whether the client sent `CMSG_GET_GARRISON_INFO` and what the reply held. Report what the capture shows before changing any code. Set `PacketLogFile` back to `""` afterwards.

- [ ] **Step 6: [user] Warlords only (Review Focus 4)**

On Lindsey, after logging in: `/dump C_Garrison.HasGarrison(2)` prints `true` and `/dump C_Garrison.HasGarrison(3)` prints `false`.

- [ ] **Step 7: [user] Both, and neither (Review Focus 4)**

On Gairy: `.cast self 156020 triggered`, `.save`, log out, log in.
Expected: `/dump C_Garrison.HasGarrison(2)` and `/dump C_Garrison.HasGarrison(3)` both print `true`.

If only one prints `true`, the client wants one reply per garrison. Change the handler to send a separate `GetGarrisonInfoResult` for each, rebuild, and repeat Steps 5 to 7.

On any character with neither (create one if needed): both commands print `false` and the character plays normally.

Ask the user to log every character out.

---

### Task 5: Land it

- [ ] **Step 1: Merge the engine branch into `dev`, and `dev` into the priest branch**

With every character offline and no build running:

```bash
cd ~/arguscore
git switch dev && git merge --no-ff --no-edit engine/class-hall-foundation
git switch feature/priest-class-hall && git merge --no-edit dev
git diff --stat integration/class-hall-foundation feature/priest-class-hall -- src sql
```

Expected: the `git diff` prints nothing, which means the tree that was tested is the tree now on the priest branch. No rebuild is needed.

- [ ] **Step 2: Remove the throwaway branch**

```bash
git branch -D integration/class-hall-foundation
```

Keep the worktree `~/arguscore-engine`. The next engine branch is stacked on this one.

- [ ] **Step 3: Clean up the test rows**

The Warlords garrisons on Gairy and Lindsey exist only for this test. With both offline:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "DELETE FROM characters.character_garrison WHERE guid IN (1, 2)" 2>/dev/null
```

Ask the user whether Gairy's class hall row should stay. The priest hall content will create it through a quest later, so the user may want Gairy clean.

- [ ] **Step 4: Ask the user which branches to push**

Candidates: `engine/class-hall-foundation`, `dev`, `feature/priest-class-hall`. Push only the ones the user names.

- [ ] **Step 5: Report**

State which checks passed, any that did not, and the three open points from the spec with what the tests showed for each.
