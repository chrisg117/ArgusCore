# Class Hall Talents Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A character with a class hall can research a hall talent, see it finish online or offline, and swap a choice; a finished talent casts its player spell and answers the talent condition checks.

**Architecture:** A new `ClassHallTalents` class in its own files, owned by `ClassHall`, with one new table. `ClassHall` forwards load, save, delete, update and the info reply to it. One new request handler, three new reply packets, one hook in `Player::Update`, and three cases in `CriteriaHandler`.

**Tech Stack:** C++ (ArgusCore, TrinityCore-derived, Legion 7.3.5 build 26972), MySQL 8.0, Docker Compose build on host `172.30.1.185`, WoW 7.3.5 client for in-game checks.

**Spec:** `docs/superpowers/specs/2026-10-13-class-hall-talents-design.md` (on `dev`)

## Global Constraints

- Engine commits contain core code and schema SQL only. No NPC flags, gossip, scripts, GM commands, debug code or TODO comments.
- One commit per task, with the exact message given. Every commit ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Engine commits are made only in the worktree `~/arguscore-engine` on branch `engine/class-hall-talents`, cut from `engine/class-hall-foundation`.
- All image builds run from `~/arguscore` and nowhere else. Never pass `--no-cache`. Never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while any character has `online = 1`. Ask the user to close the game client before every restart.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` under `sql/updates/<db>/master/`, with the server's UTC date at commit time and the lowest `NN` not used for that date and database on `main`, `dev` or either priest feature branch.
- Do not use LegionCore or any other leak-derived core as a reference.
- Never force-push, never push `main`, never open upstream issues or pull requests.
- No new compiler warnings in touched files.
- The Warlords `Garrison` class, its tables and its statements do not change.
- New files carry the same licence header as `src/server/game/ClassHall/ClassHall.h`.
- Test rows planted by hand are never committed and are removed in Task 9.
- Steps marked **[user]** are done by the user in the game client. Stop and wait for the result.
- After every boot, the first **[user]** step also includes logging Lindsey (no hall) in and out once. Anything odd there is reported before going on.
- Keep the run ledger in `~/class-hall-talents/progress.md` on the host.

## Differences from the spec

- The spec lists seven commits. This plan has eight: the packet definitions are their own commit (Task 3) and are built together with the handler commit (Task 4). `Opcodes.h` and `WorldSession.h` are included almost everywhere, so adding the three reply numbers one commit at a time would cost two extra full builds. The spec's commit sequence is updated to match.
- The research window is opened with `/run ToggleOrderHallTalentUI()`. The client falls back to the character's class tree when no NPC has named one. A planted gossip row is only the fallback (Task 0, Step 7).

## Review Focus

1. A refused request must take nothing: currency and talents unchanged. Pinned in Task 4, Steps 7 to 10 (each reads the resource count afterwards).
2. A request for a talent id that does not exist, or id 0, must be refused and must not crash. Pinned in Task 4, Step 9.
3. A talent whose unlock condition fails (row 1 at level 100) must be refused even when requested directly. Pinned in Task 4, Step 10.
4. Saved talents whose hall row is missing must not load and must not block login. Pinned in Task 2, Step 8.
5. A swap requested while other research is running must be refused. Pinned in Task 8, Step 6.

Not covered by any in-game test, for the final reviewer to read with care: the "talent selected" check (201; no client data usable with a priest), `.pdump load` of a character with talents, and the gold cost path (0 in every client row).

## Facts the tasks rely on

Read from the client files and the code at `engine/class-hall-foundation` `98c1527a`:

- Priest tree 134: row 0 = 446 Divine Fate, 447 Daily Prayers; row 1 = 452, 453; row 5 = 456; row 6 = 501, 502; row 7 = 503.
- 446: cost 50 of currency 1220, 7200 s, unlock condition 41962, player spell 219866 (passive), spell condition 41975. Swap cost 500, swap time 86400 s.
- 503: cost 15000, 432000 s, no unlock condition, player spell 243341 (passive).
- Unlock condition 41962 needs level 100 and one completed quest out of twelve (one per class; 42210 is in the list). Gairy must have one rewarded before row 0 unlocks.
- Unlock condition of row 1 needs level 105. Gairy is level 100.
- Condition 41975 is not in `PlayerCondition.db2`, neither as a record nor as a copy. `ConditionMgr::IsPlayerMeetingCondition(player, id)` is expected to return `true` for an id with no row (`ConditionMgr.cpp:2828`); Task 6, Step 1 confirms it.
- Player condition 42484 is only "any class's row 5 talent researched" (tree 35275, type 202; the priest's is 456). Modifier tree 41426 is "446 or 447, researching (207) or researched (202)".
- `GarrTalentMeta` (`DB2Metadata.h:5036`): 20 fields, id at index 7, tree as parent at index 8.
- Reply numbers, inferred: `0x28FE` result, `0x28FF` completed, `0x2900` removed.
- `GarrisonTalent` (`GarrisonPackets.h:133`) is `{ int32 GarrTalentID; Timestamp<> ResearchStartTime; int32 Flags; }` and already has its `operator<<`.
- Garrison error codes exist in `Garrison.h`: `GARRISON_ERROR_NO_GARRISON` 1, `NOT_ENOUGH_CURRENCY` 46, `NOT_ENOUGH_GOLD` 47, `INVALID_TALENT` 84, `ALREADY_RESEARCHING_TALENT` 85, `FAILED_CONDITION` 86, `INVALID_TIER` 87, `INVALID_CLASS` 88.

## File map

| File | Change | Task |
|------|--------|------|
| `sql/updates/hotfixes/master/<date>_<NN>_hotfixes.sql` | create: `garr_talent`, `garr_talent_locale` | 1 |
| `src/server/game/DataStores/DB2Structure.h`, `DB2LoadInfo.h`, `DB2Stores.h`, `DB2Stores.cpp` | add `GarrTalentEntry`, load info, `sGarrTalentStore` | 1 |
| `src/server/database/Database/Implementation/HotfixDatabase.h`, `.cpp` | add `HOTFIX_SEL_GARR_TALENT` and its two companions | 1 |
| `sql/updates/characters/master/<date>_<NN>_characters.sql` | create: `character_class_hall_talent` | 2 |
| `src/server/game/ClassHall/ClassHallTalents.h`, `.cpp` | create: the class | 2, 4, 5, 6, 7, 8 |
| `src/server/game/ClassHall/ClassHall.h`, `.cpp` | own and forward to the talents | 2, 5 |
| `src/server/database/Database/Implementation/CharacterDatabase.h`, `.cpp` | three statements | 2 |
| `src/server/game/Entities/Player/Player.h`, `.cpp` | login slot, load; later the update hook | 2, 5 |
| `src/server/game/Handlers/CharacterHandler.cpp` | one login query | 2 |
| `src/server/game/Tools/PlayerDump.cpp` | one table line | 2 |
| `src/server/game/Server/Protocol/Opcodes.h`, `.cpp` | three reply numbers; later the request handler | 3, 4 |
| `src/server/game/Server/Packets/GarrisonPackets.h`, `.cpp` | one request, three replies | 3 |
| `src/server/game/Server/WorldSession.h` | handler declaration | 4 |
| `src/server/game/Handlers/GarrisonHandler.cpp` | the handler | 4 |
| `src/server/game/Achievements/CriteriaHandler.cpp` | three condition cases | 7 |

`src/server/game/CMakeLists.txt` collects sources by itself.

---

### Task 0: Workspace, baseline and test setup

**Files:**
- Create (host, outside the repo): `~/class-hall-talents/build.sh`, `boot_check.sh`, `progress.md`

**Interfaces:**
- Produces: `build.sh` (merge the engine branch into the integration branch and build), `boot_check.sh` (restart and report the boot). Every later task runs these two.

- [ ] **Step 1: Check the preconditions**

```bash
cd ~/arguscore && git status -sb && pgrep -x docker-buildx; docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT name, online FROM characters.characters" 2>/dev/null
```

Expected: a clean tree on `feature/priest-class-hall` or `feature/priest-artifact-weapon`, no `docker-buildx` process, every character `online = 0`.

If the tree is not clean or a build is running, stop. Another session is using the build folder; the user decides when it is free.

- [ ] **Step 2: Create the two branches**

```bash
git -C ~/arguscore-engine switch -c engine/class-hall-talents engine/class-hall-foundation
cd ~/arguscore && git switch -c integration/class-hall-talents feature/priest-class-hall
```

- [ ] **Step 3: Write the two scripts**

Copy `~/class-hall-foundation/build.sh` and `boot_check.sh` to `~/class-hall-talents/`, with `d=~/class-hall-talents`, both branch names changed to the `-talents` ones, and the warning filter in `build.sh` set to:

```
ClassHall|Garrison|DB2|HotfixDatabase|CharacterDatabase|Player\.|PlayerDump|CharacterHandler|Opcodes|WorldSession|CriteriaHandler
```

Start builds with `(setsid nohup sh ~/class-hall-talents/build.sh > ~/class-hall-talents/build.out 2>&1 < /dev/null &)` and wait for `BUILD OK` or `BUILD FAILED` in `build.out`.

- [ ] **Step 4: Record the baseline**

Ask the user to close the game client. Run: `sh ~/class-hall-talents/boot_check.sh`
Expected: `READY`, then `BASELINE recorded: 13422`. A different number is the baseline for this plan; note it in the ledger.

- [ ] **Step 5: Confirm the quest gate's shape**

Print `ModifierTree.db2` row 60 (the row the root 47995 is a copy of) with the WDC1 reader `/tmp/stage1/wdc1.py`. The field order is asset, second asset, parent, type, third asset, operator, amount.
Expected: operator 8 ("some") with amount 1, so one completed quest is enough. If it is 4 ("all"), stop and report: row 0 could then never unlock.

- [ ] **Step 6: [user] Give Gairy a hall, the quest and resources**

On Gairy: `.cast self 185506 triggered`, `.quest add 42210`, `.quest complete 42210`, `.quest reward 42210`, `.modify currency 1220 20000`, `.save`.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT siteLevelId FROM characters.character_class_hall WHERE guid = 1; SELECT Quantity FROM characters.character_currency WHERE CharacterGuid = 1 AND Currency = 1220; SELECT COUNT(*) FROM characters.character_queststatus_rewarded WHERE guid = 1 AND quest = 42210" 2>/dev/null
```

Expected: `560`, `20000`, `1`. If a column name differs, read the table definition and adjust the query, not the test.

- [ ] **Step 7: [user] Open the research window**

On Gairy: `/run ToggleOrderHallTalentUI()`
Expected: the research window opens and shows the priest tree with no talent chosen.

If it does not open: plant a gossip option for it. With the client closed, in the world database give creature 110725 the gossip flag (`npcflag | 1`), a `creature_template_gossip` row to a new menu 9000001, a `gossip_menu` row for that menu with any existing text id, a `gossip_menu_option` row `(9000001, 0, OptionNpc 32, 'I would like to research.')`, and a `gossip_menu_option_addon` row `(9000001, 0, 134)`. Restart, and have the user spawn the NPC with `.npc add temp 110725`. Record the rows in the ledger for removal in Task 9.

No commit in this task.

---

### Task 1: Load `GarrTalent.db2`

**Files:**
- Create: `sql/updates/hotfixes/master/<date>_<NN>_hotfixes.sql`
- Modify: `src/server/game/DataStores/DB2Structure.h` (before `GarrTalentTreeEntry`, line 1555)
- Modify: `src/server/game/DataStores/DB2LoadInfo.h` (before `GarrTalentTreeLoadInfo`, line 1782)
- Modify: `src/server/game/DataStores/DB2Stores.h:110`, `DB2Stores.cpp:128`, `DB2Stores.cpp:644`
- Modify: `src/server/database/Database/Implementation/HotfixDatabase.h:333`, `HotfixDatabase.cpp:532`

**Interfaces:**
- Produces: `sGarrTalentStore` and

```cpp
struct GarrTalentEntry
{
    LocalizedString Name;
    LocalizedString Description;
    int32 IconFileDataID;
    int32 ResearchDurationSecs;
    int8 Tier;
    int8 UiOrder;
    int8 Flags;
    uint32 ID;
    uint32 GarrTalentTreeID;
    uint32 GarrAbilityID;
    uint32 PlayerConditionID;
    int32 ResearchCost;
    uint32 ResearchCostCurrencyTypesID;
    int32 ResearchGoldCost;
    uint32 PerkSpellID;
    uint32 PerkPlayerConditionID;
    int32 RespecCost;
    uint32 RespecCostCurrencyTypesID;
    int32 RespecDurationSecs;
    int32 RespecGoldCost;
};
```

- [ ] **Step 1: Write the hotfixes update**

Two tables, both `CREATE TABLE IF NOT EXISTS`, with a comment saying the tables hold hotfix overrides only and the rows come from the client file.

- `garr_talent`: one column per struct field in struct order (`Name` and `Description` as `text`, the three row fields as `tinyint`, signed and unsigned `int` as the struct says), then `VerifiedBuild int NOT NULL DEFAULT '0'`; primary key `(ID, VerifiedBuild)`; engine and charset as `garr_type` in `2026_10_12_00_hotfixes.sql`.
- `garr_talent_locale`: the definition `SHOW CREATE TABLE hotfixes.garr_ability_locale` prints, with the table name changed. Its columns are `ID`, `locale`, `Name_lang`, `Description_lang`, `VerifiedBuild`.

- [ ] **Step 2: Add the store**

Follow `GarrAbility`, the nearest store with two text fields, in every file it appears in:

- `GarrTalentLoadInfo`: twenty fields in struct order, named as the struct fields; `Name` and `Description` are `FT_STRING`; types and signs as the struct, with `ID` unsigned as in every other load info; meta `&GarrTalentMeta::Instance`; statement `HOTFIX_SEL_GARR_TALENT`.
- `HotfixDatabase.h`: `HOTFIX_SEL_GARR_TALENT`, `HOTFIX_SEL_GARR_TALENT_MAX_ID`, `HOTFIX_SEL_GARR_TALENT_LOCALE`, placed before the `GARR_TALENT_TREE` entries.
- `HotfixDatabase.cpp`: the select listing the twenty columns in struct order from `garr_talent`, `PREPARE_MAX_ID_STMT`, and `PREPARE_LOCALE_STMT` selecting `ID, Name_lang, Description_lang` from `garr_talent_locale`.
- `DB2Stores`: `sGarrTalentStore("GarrTalent.db2", &GarrTalentLoadInfo::Instance)`, its `extern` line and its `LOAD_DB2` line, each directly before the `GarrTalentTree` one.

- [ ] **Step 3: Commit in `~/arguscore-engine`**

```bash
git add sql/updates/hotfixes/master src/server/game/DataStores src/server/database/Database/Implementation/HotfixDatabase.h src/server/database/Database/Implementation/HotfixDatabase.cpp
git commit -m "feat(engine,sql): load GarrTalent.db2"
```

- [ ] **Step 4: Build and boot**

Run `build.sh` — expected `none` under the warnings line, then `BUILD OK`. This is a near-full build.
Run `boot_check.sh` — expected `READY`, the hotfixes file under `updates applied`, growth equal to the baseline, `none`.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SHOW TABLES IN hotfixes LIKE 'garr_talent%'" 2>/dev/null
grep -ci "GarrTalent.db2" ~/arguscore-server/logs/Server.log ~/arguscore-server/logs/DBErrors.log
```

Expected: `garr_talent`, `garr_talent_locale`, `garr_talent_tree`; and `0` for both logs (a store that fails to load is named there). Task 2 proves the rows are readable.

---

### Task 2: Store class hall talents and report them

**Files:**
- Create: `sql/updates/characters/master/<date>_<NN>_characters.sql`
- Create: `src/server/game/ClassHall/ClassHallTalents.h`, `ClassHallTalents.cpp`
- Modify: `src/server/game/ClassHall/ClassHall.h`, `ClassHall.cpp`
- Modify: `src/server/database/Database/Implementation/CharacterDatabase.h:584`, `CharacterDatabase.cpp:700`
- Modify: `src/server/game/Entities/Player/Player.h:928`, `Player.cpp:17960`
- Modify: `src/server/game/Handlers/CharacterHandler.cpp:309`
- Modify: `src/server/game/Tools/PlayerDump.cpp:98`

**Interfaces:**
- Consumes: `GarrTalentEntry`, `sGarrTalentStore` (Task 1).
- Produces:

```cpp
class TC_GAME_API ClassHallTalents
{
public:
    explicit ClassHallTalents(Player* owner);

    void LoadFromDB(PreparedQueryResult talents);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

private:
    struct Talent
    {
        GarrTalentEntry const* Entry = nullptr;
        time_t ResearchStartTime = 0;
        uint32 Flags = 0;
    };

    Player* _owner;
    std::map<uint32 /*garrTalentId*/, Talent> _talents;
};
```

- `ClassHall`: member `ClassHallTalents _talents`, accessors `ClassHallTalents& GetTalents()` and its `const` twin, and `bool LoadFromDB(PreparedQueryResult classHall, PreparedQueryResult talents)` in place of the one-argument version.
- Login slot `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL_TALENTS`.
- Statements `CHAR_SEL_CHARACTER_CLASS_HALL_TALENTS`, `CHAR_INS_CHARACTER_CLASS_HALL_TALENT`, `CHAR_DEL_CHARACTER_CLASS_HALL_TALENTS`.

- [ ] **Step 1: Write the characters update**

```sql
-- Talents (hall upgrades) of a Legion class hall. talentId is a GarrTalent.db2 id,
-- researchStartTime a unix time, flags the state as sent to the client.
-- No foreign key to character_class_hall: that row is saved by delete and insert,
-- so a cascading link would wipe the talents on every save.
CREATE TABLE IF NOT EXISTS `character_class_hall_talent` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `talentId` INT UNSIGNED NOT NULL,
  `researchStartTime` BIGINT NOT NULL DEFAULT '0',
  `flags` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`talentId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall talents';
```

- [ ] **Step 2: Add the three statements**

After the class hall statements, all `CONNECTION_ASYNC`:

| Statement | SQL |
|-----------|-----|
| `CHAR_SEL_CHARACTER_CLASS_HALL_TALENTS` | `SELECT talentId, researchStartTime, flags FROM character_class_hall_talent WHERE guid = ?` |
| `CHAR_INS_CHARACTER_CLASS_HALL_TALENT` | `INSERT INTO character_class_hall_talent (guid, talentId, researchStartTime, flags) VALUES (?, ?, ?, ?)` |
| `CHAR_DEL_CHARACTER_CLASS_HALL_TALENTS` | `DELETE FROM character_class_hall_talent WHERE guid = ?` |

- [ ] **Step 3: Write `ClassHallTalents`**

- `LoadFromDB` adds one `Talent` per row. A row whose talent is not in `sGarrTalentStore` is skipped with one error:

```cpp
TC_LOG_ERROR("entities.player", "ClassHallTalents::LoadFromDB: player {} has unknown GarrTalent {}, skipped.",
    _owner->GetGUID().ToString(), talentId);
```

- `SaveToDB` appends `CHAR_DEL_CHARACTER_CLASS_HALL_TALENTS` for the owner, then one `CHAR_INS_CHARACTER_CLASS_HALL_TALENT` per talent.
- `DeleteFromDB` appends `CHAR_DEL_CHARACTER_CLASS_HALL_TALENTS` for the given guid.
- `BuildInfo` appends one `WorldPackets::Garrison::GarrisonTalent` per talent to `info.Talents`.

It does not interpret the flags in this task.

- [ ] **Step 4: Forward from `ClassHall` and hook the login query**

- `ClassHall` constructs `_talents` with the owner. `LoadFromDB` loads the talents only after the hall row loaded. `SaveToDB`, `DeleteFromDB` and `BuildInfo` each call the talents' function after their own work.
- `Player.h`: the new login slot directly after `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL`. `Player.cpp`: pass both results to `ClassHall::LoadFromDB`.
- `CharacterHandler.cpp`: one query with `CHAR_SEL_CHARACTER_CLASS_HALL_TALENTS` and `lowGuid`, set on the new slot.
- `PlayerDump.cpp`: `{ "character_class_hall_talent", DTT_CHAR_TABLE },` directly after the `character_class_hall` line.

- [ ] **Step 5: Commit, build, boot**

```bash
git add sql/updates/characters/master src/server/game/ClassHall src/server/database/Database/Implementation/CharacterDatabase.h src/server/database/Database/Implementation/CharacterDatabase.cpp src/server/game/Entities/Player/Player.h src/server/game/Entities/Player/Player.cpp src/server/game/Handlers/CharacterHandler.cpp src/server/game/Tools/PlayerDump.cpp
git commit -m "feat(engine,sql): store class hall talents and report them to the client"
```

Run `build.sh` (near-full build) — expected `none`, `BUILD OK`.
Run `boot_check.sh` — expected `READY`, the characters file under `updates applied`, growth equal to the baseline, `none`.

- [ ] **Step 6: Find out how the client reads the flags (spec open point 2)**

With Gairy offline, plant six talents that started ten minutes ago, each with a different flag value:

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "INSERT INTO characters.character_class_hall_talent (guid, talentId, researchStartTime, flags) VALUES (1,446,UNIX_TIMESTAMP()-600,0),(1,447,UNIX_TIMESTAMP()-600,1),(1,452,UNIX_TIMESTAMP()-600,2),(1,453,UNIX_TIMESTAMP()-600,4),(1,449,UNIX_TIMESTAMP()-600,8),(1,455,UNIX_TIMESTAMP()-600,5)" 2>/dev/null
```

**[user]** Log Gairy in and run:

```
/run for _,i in ipairs({446,447,452,453,449,455,450}) do local t=C_Garrison.GetTalent(i) print(i,t.selected,t.researched,t.isBeingResearched,t.researchStartTime,t.talentAvailability) end
```

Then open the window with `/run ToggleOrderHallTalentUI()` and describe what each of the six looks like. Then `.save` and log out.

Write to the ledger: which flag value the client shows as researched, which as being researched, and whether a start time alone (flags 0) counts as being researched. Expected hypothesis: 1 is researched; being researched is either flags 0 with a start time or flag 4. The result is a ruling that Tasks 4, 5 and 8 use for `RESEARCHED`, `RESEARCHING` and `RESPEC`.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT talentId, flags FROM characters.character_class_hall_talent WHERE guid = 1 ORDER BY talentId" 2>/dev/null
```

Expected: the six rows unchanged after the save.

- [ ] **Step 7: An unknown talent is skipped and logged; the dump has the rows**

With Gairy offline, add a row with `talentId` 999999. **[user]** Log in, `.pdump write gairy_talents.dump Gairy`, log out.

```bash
grep -c "unknown GarrTalent 999999" ~/arguscore-server/logs/Server.log
grep -c "character_class_hall_talent" ~/arguscore-server/gairy_talents.dump; rm ~/arguscore-server/gairy_talents.dump
```

Expected: `1`, then a count of at least `1`. Delete the 999999 row afterwards (the save has already dropped it; confirm with a select).

- [ ] **Step 8: Talents without a hall row do not load (Review Focus 4); deletion removes the rows**

**[user]** Create a character `Talenttest`, log it in and out. Plant a talent row `(its guid, 446, now, 0)` with no hall row. **[user]** Log `Talenttest` in, `.save`, log out.
Expected: the login succeeds and the planted row is still there untouched (no hall, so nothing loaded and nothing saved).

**[user]** On `Talenttest`: `.cast self 185506 triggered`, `.save`, log out, then delete the character at the selection screen.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT COUNT(*) FROM characters.character_class_hall_talent WHERE guid = <its guid>; SELECT COUNT(*) FROM characters.character_class_hall WHERE guid = <its guid>" 2>/dev/null
```

Expected: `0` and `0`.

Remove all of Gairy's talent rows so Task 4 starts with none.

---

### Task 3: Add the garrison talent packets

**Files:**
- Modify: `src/server/game/Server/Protocol/Opcodes.h` (the `SMSG_GARRISON_*` block, line 1131 onwards), `Opcodes.cpp` (line 1226 onwards)
- Modify: `src/server/game/Server/Packets/GarrisonPackets.h`, `GarrisonPackets.cpp`

**Interfaces:**
- Produces, in `WorldPackets::Garrison`:

```cpp
class GarrisonResearchTalent final : public ClientPacket    // CMSG_GARRISON_RESEARCH_TALENT
{
    void Read() override;
    int32 GarrTalentID = 0;
};

class GarrisonResearchTalentResult final : public ServerPacket    // SMSG_GARRISON_RESEARCH_TALENT_RESULT
{
    WorldPacket const* Write() override;
    uint32 Result = 0;
    int32 GarrTypeID = 0;
    GarrisonTalent Talent;
};

class GarrisonTalentCompleted final : public ServerPacket    // SMSG_GARRISON_TALENT_COMPLETED
{
    WorldPacket const* Write() override;
    int32 GarrTypeID = 0;
    int32 GarrTalentID = 0;
    int32 Flags = 0;
};

class GarrisonTalentRemoved final : public ServerPacket    // SMSG_GARRISON_TALENT_REMOVED
{
    WorldPacket const* Write() override;
    int32 GarrTypeID = 0;
    int32 GarrTalentID = 0;
};
```

- [ ] **Step 1: Add the three reply numbers**

`Opcodes.h`, in alphabetical position: `SMSG_GARRISON_RESEARCH_TALENT_RESULT = 0x28FE`, `SMSG_GARRISON_TALENT_COMPLETED = 0x28FF`, `SMSG_GARRISON_TALENT_REMOVED = 0x2900`.
`Opcodes.cpp`: one `DEFINE_SERVER_OPCODE_HANDLER(..., STATUS_NEVER, CONNECTION_TYPE_INSTANCE)` line for each, as `SMSG_GARRISON_CREATE_RESULT` has.

- [ ] **Step 2: Add the four packet classes**

Follow `GarrisonCancelConstruction` for the request and `GarrisonCreateResult` for the replies. Write order:

- Result: `uint32 Result`, `int32 GarrTypeID`, then the talent with its existing `operator<<`.
- Completed: `int32 GarrTypeID`, `int32 GarrTalentID`, `int32 Flags`.
- Removed: `int32 GarrTypeID`, `int32 GarrTalentID`.

The 8.0.1 parser has one extra bit between the garrison type and the talent in the result reply. This first version leaves it out; Task 4, Step 6 decides.

- [ ] **Step 3: Commit**

```bash
git add src/server/game/Server/Protocol/Opcodes.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Server/Packets/GarrisonPackets.h src/server/game/Server/Packets/GarrisonPackets.cpp
git commit -m "feat(engine): add the garrison talent packets"
```

No build here. Task 4 changes `WorldSession.h`, which rebuilds the same files, so the two commits share one build. This commit has no behaviour to test on its own.

---

### Task 4: Research a class hall talent

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallTalents.h`, `ClassHallTalents.cpp`
- Modify: `src/server/game/Server/WorldSession.h:328`, `:1818`
- Modify: `src/server/game/Server/Protocol/Opcodes.cpp:422`
- Modify: `src/server/game/Handlers/GarrisonHandler.cpp`

**Interfaces:**
- Consumes: the packet classes (Task 3), `ClassHall::GetTalents()` (Task 2), the flag ruling from Task 2, Step 6.
- Produces:

```cpp
enum ClassHallTalentFlags : uint32
{
    CLASS_HALL_TALENT_FLAG_RESEARCHED  = /* ledger, Task 2 Step 6 */,
    CLASS_HALL_TALENT_FLAG_RESEARCHING = /* ledger; omit if the client needs no flag for it */
};

// in ClassHallTalents
void Research(uint32 garrTalentId);
bool HasTalent(uint32 garrTalentId) const;
bool IsResearched(uint32 garrTalentId) const;
bool IsResearching(uint32 garrTalentId) const;

// private
GarrisonError CanResearch(GarrTalentEntry const* talent) const;
```

- `void WorldSession::HandleGarrisonResearchTalent(WorldPackets::Garrison::GarrisonResearchTalent& researchTalent)`

- [ ] **Step 1: Add the three state queries**

`HasTalent`: the character has it in any state. `IsResearched`: has it with the researched flag. `IsResearching`: has it without the researched flag.

- [ ] **Step 2: Add `CanResearch`**

Checks in this order; the first failure is returned, `GARRISON_SUCCESS` otherwise:

| Check | Error |
|-------|-------|
| `talent` is not null, its tree exists in `sGarrTalentTreeStore` and the tree's `GarrTypeID` is `GARRISON_TYPE_CLASS_ORDER` | `INVALID_TALENT` |
| the tree's `ClassID` equals the owner's class | `INVALID_CLASS` |
| the owner does not have this talent | `INVALID_TALENT` |
| no talent of the owner is being researched | `ALREADY_RESEARCHING_TALENT` |
| `ConditionMgr::IsPlayerMeetingCondition(_owner, talent->PlayerConditionID)` | `FAILED_CONDITION` |
| `Tier` is 0, or the owner has a researched talent of the same tree with `Tier - 1` | `INVALID_TIER` |
| no other talent of the same tree and `Tier` is researched (a swap; Task 8 replaces this refusal) | `INVALID_TIER` |
| `ResearchCost` is 0 or less, or `_owner->HasCurrency(ResearchCostCurrencyTypesID, ResearchCost)` | `NOT_ENOUGH_CURRENCY` |
| `ResearchGoldCost` is 0 or less, or `_owner->HasEnoughMoney(uint64(ResearchGoldCost) * GOLD)` | `NOT_ENOUGH_GOLD` |

The gold cost is multiplied by `GOLD` as `Garrison` does for building costs (`Garrison.cpp:640`).

The row rule asks for a researched talent. "Chosen but still running" cannot pass anyway: while any talent is running, the check two lines above refuses first.

- [ ] **Step 3: Add `Research`**

It looks the talent up, calls `CanResearch`, and always sends one `GarrisonResearchTalentResult` with `GarrTypeID = GARRISON_TYPE_CLASS_ORDER` and the requested talent id.

On an error: `Result` is the error and nothing else changes.

On success: take the currency with `_owner->RemoveCurrency(..., CurrencyDestroyReason::Garrison)` and the gold with `_owner->ModifyMoney`, as `Garrison.cpp:399` does; add the talent with `GameTime::GetGameTime()` as its start and the researching state; send the reply with `Result = GARRISON_SUCCESS` and the talent's start time and flags.

- [ ] **Step 4: Add the handler**

`WorldSession.h`: forward declaration `class GarrisonResearchTalent;` and the handler declaration, each after the last garrison one.
`Opcodes.cpp:422`: `STATUS_LOGGEDIN`, `PROCESS_THREADUNSAFE`, `&WorldSession::HandleGarrisonResearchTalent`.
`GarrisonHandler.cpp`: with a class hall, call `GetTalents().Research(researchTalent.GarrTalentID)`. Without one, send `GarrisonResearchTalentResult` with `Result = GARRISON_ERROR_NO_GARRISON`, the class hall type and the requested id.

- [ ] **Step 5: Commit, build, boot**

```bash
git add src/server/game/ClassHall src/server/game/Server/WorldSession.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine): research a class hall talent"
```

Run `build.sh` (near-full build; it covers Task 3 too) — expected `none`, `BUILD OK`.
Run `boot_check.sh` — expected `READY`, growth equal to the baseline, `none`.

- [ ] **Step 6: [user] Research Divine Fate (spec open point 1, result reply)**

On Gairy, with 20000 resources and no talents: open the window, click Divine Fate and confirm. Then run `/run local t=C_Garrison.GetTalent(446) print(t.selected,t.isBeingResearched,t.researchStartTime,time())`, then `.save`.

Expected: the window shows the countdown on Divine Fate without a relog; the print shows `true true`, and a start time within a few seconds of the client's `time()`.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT talentId, flags, UNIX_TIMESTAMP() - researchStartTime FROM characters.character_class_hall_talent WHERE guid = 1; SELECT Quantity FROM characters.character_currency WHERE CharacterGuid = 1 AND Currency = 1220" 2>/dev/null
```

Expected: one row for 446 that started under a few minutes ago, and `19950`.

If the database is right but the window did not react:
1. A start time that is garbage means the layout is off by the missing bit. Add `Bits<1>` (false) and `FlushBits()` between the garrison type and the talent in `GarrisonResearchTalentResult::Write`, commit as `fix(engine): ...`, rebuild (short), repeat this step with the row removed.
2. No reaction with either layout means `0x28FE` is wrong. Capture packets (`PacketLogFile = "world.pkt"`) to confirm the reply left the server, then stop and report. The fallback in the spec (resend the full garrison info after a change) is the user's call.

- [ ] **Step 7: [user] A second request while research runs is refused (Review Focus 1)**

`/run C_Garrison.ResearchTalent(503)`, then `.save`.
Expected: still one talent row, resources still `19950`.

- [ ] **Step 8: [user] Too few resources**

With Gairy offline, delete the talent row and set the resources to 10. **[user]** Log in, `/run C_Garrison.ResearchTalent(446)`, `.save`, log out.
Expected: no talent row, resources `10`. Set the resources back to 20000 afterwards.

- [ ] **Step 9: [user] Unknown, foreign and locked-row requests (Review Focus 2)**

`/run C_Garrison.ResearchTalent(999999)`, `/run C_Garrison.ResearchTalent(0)`, `/run C_Garrison.ResearchTalent(402)` (warrior), `/run C_Garrison.ResearchTalent(530)` (class-less tree), `/run C_Garrison.ResearchTalent(503)` (row 7 with row 6 empty), then `.save`.
Expected: the character stays connected, no talent row, resources `20000`, no new Server.log error.

- [ ] **Step 10: [user] A failed unlock condition, and a passed row rule (Review Focus 3)**

With Gairy offline, plant 446 and 501 as researched. **[user]** Log in, `/run C_Garrison.ResearchTalent(452)` (row 1, needs level 105), `.save`.
Expected: no row for 452, resources `20000`.

**[user]** `/run C_Garrison.ResearchTalent(503)`, `.save`, log out.
Expected: a row for 503 in the researching state, resources `5000`.

If a refused request cannot be told from a request the client never sent, capture packets for that one step.

Remove Gairy's talent rows and set the resources back to 20000.

---

### Task 5: Finish class hall research on time

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallTalents.h`, `ClassHallTalents.cpp`
- Modify: `src/server/game/ClassHall/ClassHall.h`, `ClassHall.cpp`
- Modify: `src/server/game/Entities/Player/Player.cpp` (in `Player::Update`, after `m_achievementMgr->UpdateTimedCriteria`)

**Interfaces:**
- Consumes: `GarrisonTalentCompleted` (Task 3), the flags (Task 4).
- Produces: `void ClassHall::Update()`, `void ClassHallTalents::Update()`, private `time_t _researchEndTime` (0 when nothing is being researched) and private `void FinishResearch(Talent& talent)`.

- [ ] **Step 1: Track the end time**

`_researchEndTime` is the earliest `ResearchStartTime + Entry->ResearchDurationSecs` among talents being researched, or 0. It is recomputed at the end of `LoadFromDB`, after a successful `Research`, and after a finish.

- [ ] **Step 2: Add `Update` and `FinishResearch`**

`ClassHallTalents::Update` returns at once when `_researchEndTime` is 0 or later than `GameTime::GetGameTime()`. Otherwise it finishes every talent whose time has come, in start order.

`FinishResearch` sets the researched state, clears the researching one, and sends `GarrisonTalentCompleted` with the class hall type, the talent id and the new flags.

`ClassHall::Update` calls it. `Player::Update` calls `_classHall->Update()` when the character has a hall.

- [ ] **Step 3: Commit, build, boot**

```bash
git add src/server/game/ClassHall src/server/game/Entities/Player/Player.cpp
git commit -m "feat(engine): finish class hall research on time"
```

Run `build.sh` (short) — expected `none`, `BUILD OK`. Run `boot_check.sh` — expected `READY`, baseline growth, `none`.

- [ ] **Step 4: [user] Research that ended offline**

**[user]** Research Divine Fate, `.save`, log out.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -e "UPDATE characters.character_class_hall_talent SET researchStartTime = researchStartTime - 7300 WHERE guid = 1 AND talentId = 446" 2>/dev/null
```

**[user]** Log in, open the window, run `/run local t=C_Garrison.GetTalent(446) print(t.researched,t.isBeingResearched)`, `.save`, log out.
Expected: the window shows Divine Fate as finished, the print shows `true false`, and the row's flags are the researched value.

- [ ] **Step 5: [user] Research that ends while online (spec open point 1, completed reply)**

With Gairy offline, delete the row. **[user]** Log in, research Divine Fate, `.save`, log out. Move the start time back by 7140 seconds (one minute left). **[user]** Log in, open the window and wait a little over a minute.
Expected: the window marks Divine Fate finished, with its finish animation, without a relog. `/dump C_Garrison.GetTalent(446).researched` prints `true`.

If the server finished it (the row's flags changed after `.save`) but the window did not react: the completed reply's number or layout is wrong. Try the layout without the trailing `Flags` first (short build). If neither works, stop and report as in Task 4, Step 6.

Leave 446 researched for Task 6.

---

### Task 6: Cast a finished talent's spell

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallTalents.h`, `ClassHallTalents.cpp`

**Interfaces:**
- Consumes: `FinishResearch`, `Update` (Task 5).
- Produces: private `void ApplyPerk(Talent const& talent)`, private `void RemovePerk(Talent const& talent)` (used by Task 8), private `bool _passivePerksApplied`.

- [ ] **Step 1: Confirm the missing-condition behaviour**

Read `ConditionMgr::IsPlayerMeetingCondition(Player const*, uint32)` at `ConditionMgr.cpp:2828`.
Expected: it returns `true` when the id has no `PlayerCondition` row. If it returns `false`, `ApplyPerk` treats a condition id with no row in `sPlayerConditionStore` as met; record that as a ruling.

- [ ] **Step 2: Add `ApplyPerk` and `RemovePerk`**

`ApplyPerk` does nothing when the talent has no `PerkSpellID`, when the owner already has that aura, or when `PerkPlayerConditionID` is set and not met. A spell id unknown to `sSpellMgr` logs one error naming the talent and the spell, and does nothing. Otherwise it casts the spell on the owner as a triggered cast.

`RemovePerk` removes the owner's auras of the talent's `PerkSpellID`, when it has one.

- [ ] **Step 3: Call them**

- `FinishResearch` calls `ApplyPerk` after sending the reply.
- The first `Update` after the talents are loaded calls `ApplyPerk` for every researched talent whose spell `IsPassive()`, then sets `_passivePerksApplied`. The core does not save passive auras (`Aura::CanBeSaved`), and spells cannot be cast while the character is still loading. This runs before the end-time check.

- [ ] **Step 4: Commit, build, boot**

```bash
git add src/server/game/ClassHall
git commit -m "feat(engine): cast a finished class hall talent's spell"
```

Run `build.sh` (short) — expected `none`, `BUILD OK`. Run `boot_check.sh` — expected `READY`, baseline growth, `none`.

- [ ] **Step 5: [user] The aura is back after login**

**[user]** Log Gairy in (446 is researched from Task 5) and run `.list auras`.
Expected: spell 219866 is in the list. It is passive and hidden from the buff bar, so the list is the check.

- [ ] **Step 6: [user] The aura appears at finish**

With Gairy offline, delete the row. **[user]** Log in, `.list auras` (219866 is absent), research Divine Fate, `.save`, log out. Move the start time back by 7140 seconds. **[user]** Log in, wait a little over a minute, `.list auras`.
Expected: 219866 is in the list once the research has finished, without a relog.

Leave 446 researched for Task 7.

---

### Task 7: Answer the garrison talent condition checks

**Files:**
- Modify: `src/server/game/Achievements/CriteriaHandler.cpp:3002-3004`, `:3025-3026`, and its includes

**Interfaces:**
- Consumes: `Player::GetClassHall()`, `ClassHall::GetTalents() const`, `HasTalent`, `IsResearched`, `IsResearching` (Task 4).

- [ ] **Step 1: Replace the three "not implemented" cases**

In `CriteriaHandler::ModifierSatisfied`, each case returns `false` unless the reference player has a class hall whose talents answer `true` for `reqValue`:

| Case | Asks |
|------|------|
| `GarrisonTalentSelected` (201) | `HasTalent` |
| `GarrisonTalentResearched` (202) | `IsResearched` |
| `GarrisonTalentResearchInProgress` (207) | `IsResearching` |

Remove the `NYI` remarks on the three. Add `#include "ClassHall.h"`.

- [ ] **Step 2: Commit, build**

```bash
git add src/server/game/Achievements/CriteriaHandler.cpp
git commit -m "feat(engine): answer the garrison talent condition checks"
```

Run `build.sh` (short) — expected `none`, `BUILD OK`.

- [ ] **Step 3: Plant the test conditions**

With the client closed, before booting:

1. A `world.conditions` row that lets the priest spell Fade (586) be cast only when player condition 42484 passes: source type 17 (spell), source entry 586, condition type 56, value 42484. Read `ConditionMgr` for the exact target column values a spell condition on the caster needs.
2. A second test condition for the in-progress check: a `hotfixes.player_condition` row with id 900001 whose every column equals the client's row 42484 except `ModifierTreeID = 41426`. Generate the insert with a script from `PlayerCondition.db2`, with columns in `PlayerConditionLoadInfo` order.

Run `boot_check.sh` — expected `READY` and `none`. If row 900001 raises the DBErrors growth or does not load, remove it, skip Step 5, and record in the ledger that check 207 is covered by review only.

- [ ] **Step 4: [user] The researched check (202)**

**[user]** Log Gairy in and cast Fade.
Expected: the cast is refused (446 is researched, 456 is not).

With Gairy offline, plant 456 as researched. **[user]** Log in and cast Fade.
Expected: the cast works.

With Gairy offline, change 456 to the being-researched state with a start time of now. **[user]** Log in and cast Fade.
Expected: the cast is refused.

- [ ] **Step 5: [user] The in-progress check (207)**

With Gairy offline, point the `conditions` row at 900001 and leave Gairy with a single talent, 447, being researched with a start time of now. **[user]** Log in and cast Fade.
Expected: the cast works (447 in progress; nothing researched in row 0).

With Gairy offline, remove the 447 row. **[user]** Log in and cast Fade.
Expected: the cast is refused.

Remove the `conditions` row and row 900001. Leave Gairy with 446 researched and nothing else.

---

### Task 8: Swap a class hall talent

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallTalents.h`, `ClassHallTalents.cpp`

**Interfaces:**
- Consumes: `GarrisonTalentRemoved` (Task 3), `CanResearch`, `Research` (Task 4), `_researchEndTime` (Task 5), `RemovePerk` (Task 6).
- Produces: `CLASS_HALL_TALENT_FLAG_RESPEC` in `ClassHallTalentFlags` (value from the ledger; WowPacketParser names 0x2), and private `Talent const* GetResearchedTalentInRow(GarrTalentEntry const* talent) const`.

- [ ] **Step 1: [user] See what the client expects of a swap (spec open points 4 and 5)**

With 446 researched, on Gairy:

```
/run local t=C_Garrison.GetTalent(447) print(t.talentAvailability,t.researchCost,t.researchDuration,t.selected)
```

Write the output to the ledger. Expected: available, cost 500, duration 86400. If the client reports 447 as not available at all, swap cannot be driven from this client's window: stop, report, and leave swap to a follow-up as the spec allows.

- [ ] **Step 2: Treat the request as a swap**

`GetResearchedTalentInRow` returns the owner's researched talent of the same tree and `Tier` other than the given one, or null.

In `CanResearch`, when it returns one, the swap refusal from Task 4 is dropped and the cost checks use `RespecCost`, `RespecCostCurrencyTypesID` and `RespecGoldCost`.

In `Research`, on success for a swap: the respec costs are taken; the old talent's spell is removed with `RemovePerk`; the old talent is erased; `GarrisonTalentRemoved` is sent for it; the new talent is added in the researching state with the respec flag.

- [ ] **Step 3: Use the swap time**

Wherever a talent's duration is read (the end time in Task 5), a talent with the respec flag uses `RespecDurationSecs`. `FinishResearch` clears the respec flag.

- [ ] **Step 4: Commit, build, boot**

```bash
git add src/server/game/ClassHall
git commit -m "feat(engine): swap a class hall talent"
```

Run `build.sh` (short) — expected `none`, `BUILD OK`. Run `boot_check.sh` — expected `READY`, baseline growth, `none`.

- [ ] **Step 5: [user] Swap Divine Fate for Daily Prayers (spec open point 1, removed reply)**

With 446 researched and 20000 resources (set by hand if needed): **[user]** `.list auras` (219866 present), open the window, click Daily Prayers and confirm. Then `.list auras`, `/run local t=C_Garrison.GetTalent(446) print(t.selected,t.researched)`, `.save`.

Expected: the window shows Daily Prayers counting down from one day and Divine Fate unselected, without a relog; 219866 is gone; the print shows `false false`.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT talentId, flags FROM characters.character_class_hall_talent WHERE guid = 1; SELECT Quantity FROM characters.character_currency WHERE CharacterGuid = 1 AND Currency = 1220" 2>/dev/null
```

Expected: one row, 447, with the researching and respec flags; resources `19500`.

If the database is right but Divine Fate still shows as selected in the window until a relog, the removed reply's number or layout is wrong. Stop and report as in Task 4, Step 6.

- [ ] **Step 6: [user] A swap while research runs is refused (Review Focus 5); the swap finishes on its own time**

**[user]** `/run C_Garrison.ResearchTalent(446)`, `.save`, log out.
Expected: still only the 447 row, resources `19500`.

Move 447's start time back by 86340 seconds (one minute of the day left). **[user]** Log in, wait a little over a minute, `/dump C_Garrison.GetTalent(447).researched`, `.save`, log out.
Expected: `true`, and the row's flags are the researched value alone. Moving it back by only 7300 seconds first, and seeing it still running after a relog, proves the swap time is used and not the two-hour research time; do that check before the 86340 one.

---

### Task 9: Land it

- [ ] **Step 1: Clean up the test state**

With every character offline: remove Gairy's talent rows and any planted world or hotfixes rows listed in the ledger. Ask the user whether Gairy keeps the class hall row, quest 42210 and the Order Resources; the priest hall content will hand those out through quests later.

- [ ] **Step 2: Final review**

Dispatch one fresh reviewer on the most capable model over `engine/class-hall-foundation..engine/class-hall-talents`, with this plan, the spec, the Review Focus section and the ledger's rulings. Critical and Important findings get one fix pass, each fix rebuilt and rechecked in game; Minor findings go to the ledger as deferred.

- [ ] **Step 3: Merge**

With no build running, each merge in a temporary worktree except the last:

```bash
cd ~/arguscore
git worktree add /tmp/wt-dev dev && git -C /tmp/wt-dev merge --no-ff --no-edit engine/class-hall-talents && git worktree remove /tmp/wt-dev
git worktree add /tmp/wt-art feature/priest-artifact-weapon && git -C /tmp/wt-art merge --no-edit dev && git worktree remove /tmp/wt-art
git switch feature/priest-class-hall && git merge --no-edit dev
git diff --stat integration/class-hall-talents feature/priest-class-hall -- src sql
git branch -D integration/class-hall-talents
```

Expected: the `git diff` prints nothing, so the tree that was tested is the tree on the priest branch and no rebuild is needed.

- [ ] **Step 4: Update the branch list and push**

On `dev`, in a temporary worktree: add `engine/class-hall-talents` to "Current branches" in `docs/BRANCHING.md` and correct the spec's open points with what the tests showed, as one `chore(local)` commit. Merge `dev` into both priest branches again.

Push `engine/class-hall-talents`, `dev`, `feature/priest-artifact-weapon` and `feature/priest-class-hall`. This is covered by rule 13 of the branching rules.

- [ ] **Step 5: Report**

State which checks passed and any that did not; every `Ruling:` line from the ledger with what it costs if wrong; every deferred minor; and the answer the tests gave to each open point in the spec. Tell the user which branch `~/arguscore` is on.
