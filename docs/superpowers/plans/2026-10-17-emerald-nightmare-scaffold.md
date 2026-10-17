# Emerald Nightmare Scaffold Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A player can walk into the Emerald Nightmare (map 1520) through its portal in Val'sharah, get an instance with a working `InstanceScript` and a raid lock, and walk back out.

**Architecture:** A new `EmeraldNightmare/` script directory, laid out like `TrialOfValor/`: a header with the seven encounters and an instance script that loads their object and encounter data. An `instance_template` row attaches the script to map 1520. Two static server-side areatriggers use the core's built-in teleport action (`ActionType` 2), so the portal and the exit need no C++.

**Tech Stack:** C++ (ArgusCore, a TrinityCore fork for 7.3.5 build 26972), MySQL update files, Docker image build on the host `172.30.1.185`, in-game checks by the user on Gairy.

**Spec:** `docs/superpowers/specs/2026-10-17-emerald-nightmare-scaffold-design.md`

## Global Constraints

- Work goes on `dev` directly (rule 3 of `docs/BRANCHING.md`), committed as `feat(scripts,sql)`. Nothing goes on `engine`; if a core fix turns out to be needed, stop and ask (rule 5: it would go on `engine`).
- SQL files are named `YYYY_MM_DD_NN_world.sql` by the server's UTC date, under `sql/updates/world/master/`, using the next free `NN` on that date. Each is safe to re-run: `DELETE` then `INSERT`, with a comment on each block.
- Commit messages end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- `~/arguscore` is the only build folder and stays on `dev`. Never pass `--no-cache`. Never start a build while `pgrep -x docker-buildx` prints a process. Before touching the folder, confirm with the "Priest class hall engine work" session that it isn't mid-change there (rules 10 and 12).
- Never restart the world server while a character has `online = 1` or a client is connected. Ask the user to close the game client before a restart.
- Do not use any leak-derived core as a source. Allowed: client DB2 data, TrinityCore upstream, Wowhead and public guides, measurements in game.
- No boss, trash or door spawns, no boss scripts, no `DoorData` or `BossBoundaryData`: those belong to the boss specs.
- In-game steps go to the user a few at a time, in plain words.
- Gairy is left as she was: any raid lock from testing is removed at the end (`.instance unbind all`) unless the user wants to keep it.
- Push `dev` after each committed task (rule 14). Never force-push.

## Review Focus

1. **Bounce loop inside.** A player arriving at WorldSafeLocs 5533 must not land inside the exit trigger, or they would be sent straight back out. Pinned in Task 2 Step 2: distance from 5533 to the exit spawn is more than radius + 2.
2. **Bounce loop outside.** The same holds for a player leaving the raid at 5687 and the entrance trigger. Pinned in Task 2 Step 2.
3. **No raid group.** A non-GM player outside a raid group who walks into the portal stays in Val'sharah with the "need a raid group" message, and is not moved anywhere else. Pinned in Task 2 Step 6.
4. **Coming back.** Walking in a second time puts the player back in the same instance, with the lock from the first entry. Pinned in Task 2 Step 6.
5. **Other difficulties.** The exit trigger is spawned for 14, 15, 16 and 17, so a Heroic raid can also be left on foot. Pinned in Task 2 Step 6, with the raid set to Heroic.

Not covered by any in-game test, for the final reviewer to read with care: the dungeon finder (LFG 1348-1350, which the spec leaves out of scope), Raid Finder difficulty 17, and dying inside (there are no graveyards yet).

## Facts the tasks rely on

All checked 2026-10-17 on the host and in the client's DB2 files; details are in the spec.

- `dev` has no `EmeraldNightmare/` directory. Script CMake collects new files in a scripts directory on its own, so no CMake change is needed. The loader is `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp`, with blocks commented per instance.
- Template to copy: `src/server/scripts/BrokenIsles/TrialOfValor/trial_of_valor.h` and `instance_trial_of_valor.cpp`.
- `areatrigger_template` (`Id, IsCustom, Flags, ActionSetId, ActionSetFlags, VerifiedBuild`), `areatrigger_template_actions` (`AreaTriggerId, IsCustom, ActionType, ActionParam, TargetType`), `areatrigger_create_properties` (`Id, IsCustom, AreaTriggerId, IsAreatriggerCustom, Flags, …, Shape, ShapeData0-5, ScriptName, VerifiedBuild`), `areatrigger` (`SpawnId, AreaTriggerCreatePropertiesId, IsCustom, MapId, SpawnDifficulties, PosX, PosY, PosZ, Orientation, PhaseUseFlags, PhaseId, PhaseGroup, ScriptName, Comment, VerifiedBuild`).
- The working example is "The Seat of the Triumvirate - Enter": template 110 (`Flags 1`, `ActionSetFlags 10`), action `(110, 1, 2, 6048, 5)`, create properties 105 and spawn 204.
- Shape 0 is a sphere; `ShapeData0` and `ShapeData1` are its radius and target radius (the Dalaran pads use 2.2, 2.2).
- Free on 2026-10-17: template and create properties ids 152001 and 152002, spawn ids 1220005 and 1520001.
- Helper scripts to copy: `~/class-hall-extras/build.sh` and `boot_check.sh`. The boot check records a baseline on its first run, then reports DBErrors growth and new Server.log ERROR/WARN lines.

---

### Task 0: Workspace

**Files:** none in the repository. Creates the host folder `~/emerald-nightmare/`.

- [ ] **Step 1: Check the build folder.** Ask the "Priest class hall engine work" session whether it is mid-change in `~/arguscore`, then run `cd ~/arguscore && git fetch -q origin && git status -sb | head -1 && git status -s | wc -l && pgrep -x docker-buildx | wc -l`. Expected: `## dev...origin/dev` with nothing ahead (behind is fine), `0`, `0`. If it's behind, `git merge --ff-only origin/dev`.
- [ ] **Step 2: Helper scripts.** `mkdir ~/emerald-nightmare` and copy `build.sh` and `boot_check.sh` from `~/class-hall-extras/`. Change each script's `d=` line to `~/emerald-nightmare`. Start `~/emerald-nightmare/ledger.md` with the date and `git log -1 --oneline`.
- [ ] **Step 3: Baseline boot.** With the user's client closed, run `~/emerald-nightmare/boot_check.sh` on the current image. Expected: `READY`, `BASELINE recorded: <n>`. Ledger `<n>`.
- [ ] **Step 4: Gairy's locks.** Ask the user to log in and run `.instance listbinds`. Ledger the output, so the end of Task 2 can put it back.

### Task 1: Instance script and `instance_template`

**Files:**
- Create: `src/server/scripts/BrokenIsles/EmeraldNightmare/emerald_nightmare.h`
- Create: `src/server/scripts/BrokenIsles/EmeraldNightmare/instance_emerald_nightmare.cpp`
- Modify: `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp` (declaration and call, after the Black Rook Hold block)
- Create: `sql/updates/world/master/<date>_<NN>_world.sql`

**Interfaces:**
- Produces, for every boss spec after this one:
  - `#define DataHeader "EmeraldNightmare"`, `#define ENScriptName "instance_emerald_nightmare"`, `uint32 const EncounterCount = 7;`
  - `enum ENDataTypes { DATA_NYTHENDRA = 0, DATA_ILGYNOTH, DATA_ELERETHE_RENFERAL, DATA_URSOC, DATA_DRAGONS_OF_NIGHTMARE, DATA_CENARIUS, DATA_XAVIUS };`
  - `enum ENCreatureIds { BOSS_NYTHENDRA = 102672, BOSS_ILGYNOTH = 105393, BOSS_ELERETHE_RENFERAL = 106087, BOSS_URSOC = 100497, BOSS_YSONDRE = 102679, BOSS_TAERAR = 102681, BOSS_LETHON = 102682, BOSS_EMERISS = 102683, BOSS_CENARIUS = 104636, BOSS_XAVIUS = 103769 };`
  - `template <class AI, class T> inline AI* GetEmeraldNightmareAI(T* obj)` and `#define RegisterEmeraldNightmareCreatureAI(ai_name)`, as in `trial_of_valor.h`.
  - `void AddSC_instance_emerald_nightmare();`

- [ ] **Step 1: Header.** Write `emerald_nightmare.h` with exactly the Interfaces above, the TrinityCore licence block, and the include guard `DEF_EMERALD_NIGHTMARE_H_`.
- [ ] **Step 2: Instance script.** Write `instance_emerald_nightmare.cpp` modelled on `instance_trial_of_valor.cpp`:
  - `creatureData[]` maps the six single bosses (Nythendra, Il'gynoth, Elerethe, Ursoc, Cenarius, Xavius) to their `DATA_*`. It leaves out the four dragons and ends with `{ 0, 0 }`.
  - `encounters[]` maps each `DATA_*` to its id: Nythendra 1853, Il'gynoth 1873, Elerethe 1876, Ursoc 1841, Dragons 1854, Cenarius 1877, Xavius 1864.
  - Class `instance_emerald_nightmare : InstanceMapScript(ENScriptName, 1520)`. Its constructor calls `SetHeaders(DataHeader)`, `SetBossNumber(EncounterCount)`, `LoadObjectData(creatureData, nullptr)` and `LoadDungeonEncounterData(encounters)`. There is no `LoadDoorData` and no `LoadBossBoundaries`.
- [ ] **Step 3: Loader.** Add a `// Emerald Nightmare` block declaring and calling `AddSC_instance_emerald_nightmare()`, after the Black Rook Hold block in both places.
- [ ] **Step 4: SQL.** In the update file, `DELETE FROM instance_template WHERE map = 1520;` and then `INSERT INTO instance_template (map, parent, script) VALUES (1520, 0, 'instance_emerald_nightmare');`, with a comment naming the raid.
- [ ] **Step 5: Build.** Run `~/emerald-nightmare/build.sh`. Expected: build OK, and no warnings in the two new files or the loader.
- [ ] **Step 6: Boot.** Run `~/emerald-nightmare/boot_check.sh`. Expected: `READY`, the new SQL file listed under "updates applied", DBErrors growth equal to the baseline, and no new Server.log lines. In particular there must be no "script … not found" for `instance_emerald_nightmare`.
- [ ] **Step 7: In game (user).** With `.gm on` (GM mode skips the level 110 and raid-group entry checks), Gairy runs `.go instance emerald_nightmare` (the command matches script names, not map ids), then `.instance stats`, then `.instance getbossstate 0`. Expected: she arrives at the raid entrance, the stats name `instance_emerald_nightmare` for map 1520, and boss 0 reports state `NOT_STARTED` (0), not an error. Ledger the instance id.
- [ ] **Step 8: Commit and push.** `git add` the four files, then `git commit` with `feat(scripts,sql): Emerald Nightmare instance script and instance template`, then `git push origin dev`.

### Task 2: Entrance portal and exit trigger

**Files:**
- Create: `sql/updates/world/master/<date>_<NN+1>_world.sql`

This is a second SQL file because the triggers are a separate logical change from the instance template (rule 7). The spec had both in one file; splitting them keeps each commit's file matching what it changes.

**Interfaces:**
- Consumes: WorldSafeLocs 5533 (inside: 1810.12, 1424.19, 355.17, facing 339.91 degrees) and 5687 (Val'sharah: 3588.28, 6483.40, 177.97, facing 218.88 degrees).
- Produces: template and create properties 152001 (entrance) and 152002 (exit), spawns 1220005 (map 1220, difficulties `0`) and 1520001 (map 1520, difficulties `14,15,16,17`).

- [ ] **Step 1: Measure the spots (user).**
  - Gairy runs `.go xyz 3588.28 6483.40 177.97 1220`. She turns around, walks into the middle of the portal's visual and runs `.gps`.
  - She then runs `.go instance emerald_nightmare`, turns around, steps 6 yards back from where she arrived, and runs `.gps`. If the raid shows an exit portal visual there, she stands in its middle instead.
  - Ledger both positions. If no portal visual is visible in Val'sharah, stop and tell the user (see the spec's risks).
- [ ] **Step 2: Check the distances.** Compute the 2D distance from 5687 to the entrance spot, and from 5533 to the exit spot. Expected: each is more than 6 (radius 4 + 2). If not, move the trigger along the line away from the arrival point until it is, and ledger the change.
- [ ] **Step 3: SQL.** For each trigger, write `DELETE`s, then `INSERT`s, into the four tables, copying template 110's rows:
  - `areatrigger_template (Id, 1, 1, 0, 10, 0)`.
  - `areatrigger_template_actions (Id, 1, 2, <safe loc>, 5)`: 5533 for 152001, 5687 for 152002.
  - `areatrigger_create_properties (Id, 1, Id, 1, …, Shape 0, ShapeData0 4, ShapeData1 4)`.
  - `areatrigger` spawn: entrance on map 1220 with difficulties `'0'`; exit on map 1520 with `'14,15,16,17'`. Use the measured positions, orientation 0, an empty `ScriptName`, and the comments `The Emerald Nightmare - Entrance` and `The Emerald Nightmare - Exit`.
- [ ] **Step 4: Boot.** Run `~/emerald-nightmare/boot_check.sh` on the same image, since SQL needs no build. Expected: `READY`, the new file applied, DBErrors growth equal to the baseline, and no new Server.log lines.
- [ ] **Step 5: Walk in and out (user, GM on).** With `.gm on`, Gairy goes to 5687 (as in Step 1) and walks into the portal. Expected: she arrives inside at the raid entrance and is not sent straight back. She walks into the exit trigger. Expected: she arrives in Val'sharah and stays there.
- [ ] **Step 6: The Review Focus checks (user).**
  1. With `.gm off` and no group, she walks into the portal. Expected: the "need a raid group" message, and she is still in Val'sharah.
  2. With `.gm on`, she walks in again and runs `.instance stats`. Expected: the same instance id as the one ledgered in Task 1 Step 7.
  3. She leaves, sets raid difficulty to Heroic (`.gm on` keeps entry possible), walks in, and walks into the exit. Expected: she arrives in Val'sharah.
- [ ] **Step 7: Commit and push.** `git add` the SQL file, then `git commit` with `feat(scripts,sql): Emerald Nightmare entrance portal and exit trigger`, then `git push origin dev`.
- [ ] **Step 8: Put Gairy back (user).** Unless the user wants to keep them, `.instance unbind all`, then `.instance listbinds` against the ledgered list from Task 0 Step 4. Set raid difficulty back to Normal and turn `.gm off`.

### Task 3: Land it

**Files:**
- Modify: `README.md` (the "What this fork adds" content list)

- [ ] **Step 1: README.** Add Emerald Nightmare to the content section: "Emerald Nightmare: raid enterable through its Val'sharah portal; instance script for all seven encounters; no bosses scripted yet (Nythendra next)".
- [ ] **Step 2: Commit and push.** Commit as `chore(local): note the Emerald Nightmare scaffold in the README`, then push `dev`.
- [ ] **Step 3: Tell the other session.** Message the "Priest class hall engine work" session: `dev` moved (list the commit ids), and the build folder holds the new image.
