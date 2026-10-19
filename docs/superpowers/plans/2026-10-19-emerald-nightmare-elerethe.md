# Emerald Nightmare: Elerethe Renferal Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Elerethe Renferal (106087) can be pulled, fought through her Spider and Roc forms and both flights across three platforms, and killed on Normal in the Emerald Nightmare, with her retail Normal mechanics.

**Architecture:**
- **Boss.** One `BossAI` (`boss_elerethe_renferal`) runs the fight. It owns:
  - the form, the energy fill and the platform index;
  - the flight, and every timer;
  - every summon (spiderlings, tornadoes, the Feathers, the Eye of the Storm).
- **The Roc.** The client's transform aura (226039 / 226055) changes her look. She stays one unit with one health pool.
- **Scenery.** Egg sacs are DB spawns with a small AI. The bridge webs are static custom area triggers with an `AreaTriggerAI`.
- **Spells.** Spell and aura scripts cover the Necrotic Venom target count, the Dark Storm ticks and Web of Pain pairing, plus the Feather crush if the data does not do it.

**Tech Stack:**
- C++ with the ArgusCore / TrinityCore script API.
- MySQL world SQL updates.
- Python 3 with `/tmp/stage1/wdc1.py`, `/tmp/stage1/spellinfo.py` and `/tmp/stage1/spellnames.py` for the DB2 dumps.
- The Docker build through `~/emerald-nightmare/build.sh`.

**Spec:** `docs/superpowers/specs/2026-10-19-emerald-nightmare-elerethe-design.md`

## Global Constraints

**Where and how to work**
- Work over SSH on `chris@172.30.1.185` in `~/arguscore`, branch `dev` (`docs/BRANCHING.md` rule 3).
- Only one session uses `~/arguscore` at a time. Before every build:
  - ask the session holding the folder (today "Spec Il'gynoth", `local_cca93dab-95a3-4eb2-abc0-1ce9c30bf3f0`);
  - check the tree has only your own changes;
  - check `pgrep -x docker-buildx` prints nothing (rules 9, 10).
- Never use `--no-cache`. Never restart while a character is online (rule 13; `boot_check.sh` refuses if one is).
- Commits: one kind per commit (`feat(scripts,sql)` for script plus SQL). Commit only files you changed, with `git commit -- <paths>`. Push `dev` after each stage commit.
- Core (engine) fixes go on `engine` (`~/arguscore-engine`), then merge into `dev`. Never in this plan's commits.
- No leak-derived sources (LegionCore and similar). Normal only; spawn difficulty `'14'`.

**SQL files**
- One file per task: `sql/updates/world/master/<date>_NN_world.sql`, with the next free `NN` on the day of the work (`ls sql/updates/world/master | tail`). On 2026-10-19, `_00`-`_03` are taken.
- Each file is re-runnable: `DELETE` then `INSERT`/`UPDATE`.
- SQL-only changes still need a rebuild: the image COPYs `sql/`.

**Ids**

| What | Id |
|---|---|
| Data id | `DATA_ELERETHE_RENFERAL = 2`, encounter 1876 |
| Elerethe | 106087 (boss; spider display 69474) |
| Roc display source | 105604 (display 70237; template not changed) |
| Venomous Spiderling | 106311 |
| Skittering Spiderling | 108542 |
| Twisting Shadows tornado | 106350 |
| Egg sac | from Task 1 (108540, 109855 or 113154) |
| Creature guids | from 1520103 (Elerethe), then the egg sacs |
| Custom area triggers | 152003 Eye of the Storm, 152004 Shimmering Feather, 152005 web P1→P2, 152006 web P2→P3 (template and create-properties ids; free as of 2026-10-19) |
| Static area trigger spawns | 1520002 and 1520003 (the bridge webs; 1520001 is the exit portal) |

**Spells**

| Spell | Id |
|---|---|
| Arachnid Form (cast) → Spider Transform | 210326 → 226055 |
| Darkwing Form (cast) → Bird Transform | 210308 → 226039 |
| Swarm (passive) | 214450 |
| Necrotic Venom | 215443 |
| Feeding Time | 212364 |
| Vile Ambush | 214339 |
| Web of Pain | 215288 (links 215300 source, 215307 target) |
| Web Crawlers | 210327 |
| Dripping Fangs (spiderling passive) | 210229 |
| Venomous Pool (corpse, AT) | 215489 / 213123 (AT 7078, aura 213124) |
| Twisting Shadows | 210864 (tornado aura 210850, fixate 221877) |
| Gathering Clouds | 212707 |
| Dark Storm (cast) | 210948 |
| Dark Storm tick | 213936 |
| Foul Winds (ramp) | 211137 |
| Eye of the Storm (safe aura) | 211127 |
| Raking Talons | 215582, quick repeat 222037 |
| Razor Wing | 210547 |
| Shimmering Feather | 212993 (crush 221993 → 221994) |
| Berserk | from Task 1 (Nythendra uses 26662) |

**Text** (106087)

| Line | Broadcast | Type |
|---|---|---|
| Web of Pain | 90971 | yell (14) |
| Feeding Time | 111635 | emote (41), raid boss emote |
| Gathering Clouds | 111411 | emote (41), raid boss emote |
| more lines | from Task 1 | as found |

**Timers** (DBM 7.3.31 `Renferal.lua`; seconds from the form's transform, or from the pull for the first Spider form)

| Form | Ability | Timing |
|---|---|---|
| Spider | Necrotic Venom | 12.2, then every 21.8, 4 casts |
| Spider | Feeding Time | 15.5, then +50, 2 casts; Vile Ambush ~10 s after each |
| Spider | Web of Pain | every 30 (calibrate; DBM has no timer) |
| Roc | Twisting Shadows | 6.6, then +40, +21.5, +32.5 (4 casts) |
| Roc | Gathering Clouds | 15.8 |
| Roc | Dark Storm, then the flight | 26 (first and second Roc form only) |
| Roc | Raking Talons | 52, then every 32.5, 3 casts (each = 215582 + one 222037) |
| Roc | Razor Wing | 59, then every 32.5, 3 casts |
| — | Energy fill | +100/90 per s (first Spider form), +100/92 (later Spider forms), +100/132 (Roc) |
| — | Berserk | 540 from the pull |

**Calibrate values** (constants in one block at the top of the boss file)

| Value | Start |
|---|---|
| Spiderlings per Feeding Time | 5 |
| Bridge drop per flight | 2 spiderlings + 1 tornado |
| Skittering Spiderlings per sac | 4 |
| Feathers per flight | 3 |
| Eye of the Storm radius | 15 yd |
| Flight time | ~8 s |
| Feeding Time rise / time in the web | 15 yd / 10 s |
| Necrotic Venom targets | 1 solo, 2 in a group |
| Pool AT 7078 radius / decal | 3 / 31 |
| Egg-sac pop range; Feather crush range | 3 yd; 3 yd |

**Tests** are in-game checks; the scripts have no unit-test harness. Each task's test is the exact observation that must hold. DBM 7.3.31 is installed in the client (`E:\Legion\Interface\AddOns`), and its bars are the timing reference.

## Review Focus

- **Evade or wipe while she is up in the web (Feeding Time) or in flight.** She lands back on her P1 spawn: selectable, gravity on, fly mode off, Spider display, 0 energy. No summon, pool, Feather or Eye is left behind. Players lose 210327, 211127 and 212993. Task 3 tests this mid-Feeding Time, Task 5 mid-flight.
- **A form change or a cast falls due during Feeding Time or the flight.** DBM's times never cause this, but a calibrated value might. The change or cast is held, not lost: energy stays at 100, and the change happens as soon as she is down again. Task 3 builds the rule; Task 7 tests it if a calibrated value creates an overlap.
- **She dies during Feeding Time or the flight** (from `.damage`). Boss state Done, she drops to the ground, every summon and the storm aura are gone, and nobody takes storm ticks after the kill (Task 5).
- **Solo raid.** Necrotic Venom picks 1, Vile Ambush the only player, Web of Pain is skipped. No selection loops on zero candidates, and nothing crashes when the only player is dead (Tasks 3 and 4).
- **Players who reset or relog with the fight's auras.** A player who leaves the Eye or the web, dies on the web, or relogs keeps no stale Eye of the Storm or Web Crawlers aura, and a spiderling never spawns from a player who is not on the web (Task 6).

---

### Task 1: Dump the data that is still open

**Files:**
- Create: `~/emerald-nightmare/elerethe_task1.txt` (host, not in the repo)
- Create: `elerethe-task1-findings.md` in the shared notes folder
- Modify: the spec, a new section "Amendment after the data dump (plan Task 1)"

**Interfaces:**
- Produces answers (a)-(h), which Tasks 2-6 read:
  - (a) **Berserk:** the spell id. Look for a "Berserk" spell in the 21xxxx-22xxxx range that an Emerald Nightmare boss uses; otherwise use 26662 as Nythendra does.
  - (b) **Egg sac:** which of 108540, 109855 and 113154 is the Emerald Nightmare sac (display 70606 or 42349: the model file name from `CreatureDisplayInfo` / `CreatureModelData` and the community listfile). Also whether any spell (221726 Pop Egg Sac, or a summon with misc 108542) releases Skittering Spiderlings.
  - (c) **Tornado:** does 210877's summon carry 210850, or does the tornado need to cast it? Does 221877 make it chase? Look for a SpellEffect with misc 106350, and at 210850 effect 0 (aura 236) and 221877 (aura 128).
  - (d) **Spiderling death pool:** what puts down 215489 or 213123 when 106311 dies. Look at which spells trigger them; if nothing does, the AI casts 215489 at death.
  - (e) **Feather:** what 212993's aura 42 proc fires on (SpellProcsPerMinute/`SpellAuraOptions` proc flags for 212993) and what 212986 (effect 138) and 221993 do. Is a jump landing a proc this engine supports? If not, the crush is an aura script (Task 6).
  - (f) **Malfurion:** is there a Malfurion creature in map 1520 data or a DB2 placement near her room, and which creature speaks 112999 / 113000? If none, the death lines are a known gap.
  - (g) **Her lines:** search `BroadcastText.db2` for the wiki quotes (the spec's Sources). For each found, its id and `SoundKitID`.
  - (h) **Necrotic Venom targets:** 215443's `SpellTargetRestrictions` MaxTargets by difficulty, which may replace the "2 in a group" guess.

- [ ] **Step 1:** Run `python3 /tmp/stage1/spellinfo.py ~/arguscore-server/data/dbc/enUS 215443 215489 213123 210850 210877 221877 212993 212986 221993 221994 221726 deep` and save the output to `elerethe_task1.txt`. Expected: every id prints at least one effect row.
- [ ] **Step 2:** For (a), (b), (f) and (g), write a small Python reader beside the other dump scripts (same `wdc1.read` pattern; layouts in `src/server/game/DataStores/DB2Structure.h`). Run it and append the output. For (g), match on 25-character prefixes of each quote.
- [ ] **Step 3:** Write the answers at the top of `elerethe_task1.txt` and in the findings note. Then add the spec amendment, commit `docs: Elerethe data-dump amendment`, push, and copy the spec to `C:\Users\chris\code\acore-webservice\docs\superpowers\specs\`. If an answer contradicts the spec's "The encounter" section, tell the user before Task 2.

### Task 2: Skeleton: spawn, boundary, energy and the form cycle (pull, evade, die)

**Files:**
- Create: `src/server/scripts/BrokenIsles/EmeraldNightmare/boss_elerethe_renferal.cpp`
- Modify: `src/server/scripts/BrokenIsles/EmeraldNightmare/emerald_nightmare.h`
- Modify: `src/server/scripts/BrokenIsles/EmeraldNightmare/instance_emerald_nightmare.cpp`
- Modify: `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp` (declare and call `AddSC_boss_elerethe_renferal()` under `// Emerald Nightmare`)
- Create: `sql/updates/world/master/<date>_NN_world.sql`

**Gate:** this task needs the user's `.gps` readings, items 1-6 of the spec's "Measured inputs".
- How to read them: each `.gps` command line in `GM.log` carries the player's X/Y/Z.
- If they are not in hand, ask the user and wait.
- **Walkability comes first.** If the user reports that the webs cannot be walked, stop and raise it before writing code.
- Fit each platform's three edge points to a circle (centre at that platform's floor z). Fit each bridge to a box, using its ends, its midpoint and a width of 8 yd (calibrate). Check that the union contains every reading, including the way in.

**Interfaces:**
- Produces, in `emerald_nightmare.h` under `// Elerethe Renferal` in `ENCreatureIds`: `NPC_VENOMOUS_SPIDERLING = 106311`, `NPC_SKITTERING_SPIDERLING = 108542`, `NPC_TWISTING_SHADOWS = 106350`, `NPC_ELERETHE_EGG_SAC = <Task 1 (b)>`.
- Produces, in `boss_elerethe_renferal.cpp`:
  - `enum ElerethePhases { PHASE_SPIDER = 1, PHASE_ROC = 2 };`
  - `struct PlatformData { Position landing; Position centre; float radius; };` and `static std::array<PlatformData, 3> const Platforms`, from the readings.
  - `static std::array<std::vector<Position>, 2> const FlightPaths` (from P1 to P2, and from P2 to P3: air points, then the landing spot).
  - `static std::array<Position, 2> const BridgeMidpoints`.
  - `struct boss_elerethe_renferal : BossAI`, with these members:
    - `uint8 _platform` (0-2);
    - `uint8 _formCount` (Spider forms started; 1 at the pull);
    - `uint8 _rocCount`;
    - `bool _busy` (true while up in the web or in flight);
    - `bool _formChangePending`.
  - `void StartSpiderForm()` and `void StartRocForm()`, which reset energy to 0 and schedule the form's events (stubs for the abilities in this task).
  - `void TryFormChange()`: if energy is 100 and `!_busy`, cast 210308 or 210326. Otherwise it sets `_formChangePending`.
  - `void SetBusy(bool busy)`: sets the flag. Clearing it re-runs `TryFormChange()` and moves the held events (Task 3 explains holding).
  - `enum EleretheTalks { TALK_WEB_OF_PAIN = 0, TALK_FEEDING_TIME, TALK_GATHERING_CLOUDS };`, plus the ids Task 1 (g) adds after them.
- The boundary in the instance: `{ DATA_ELERETHE_RENFERAL, new BoundaryUnionBoundary(...) }`, built from three `CircleBoundary` and two bridge boxes. Use the constructor in `AreaBoundary.h:138`; check whether it nests two at a time.

- [ ] **Step 1: SQL.** One block per item, each preceded by a comment:
  - `creature_template`:
    - `faction = 14` and the `ScriptName`s: 106087 `boss_elerethe_renferal`, 106311 `npc_en_venomous_spiderling`, 108542 `npc_en_skittering_spiderling`, 106350 `npc_en_twisting_shadows`, and the sac `npc_en_elerethe_egg_sac`.
    - `BaseAttackTime = 2000`; `unit_class = 4` on 106087.
    - The sac: `unit_flags` not selectable plus not attackable as the fork names them (check `UnitFlags` in `UnitDefines.h`), and `flags_extra` no threat. The tornado: not selectable, no threat.
  - `creature_template_difficulty`: `MinLevel = MaxLevel = 110` for all five (DifficultyID 0 rows).
  - `creature`: Elerethe at P1's landing spot and facing (guid 1520103), then each sac (1520104…), with `spawnDifficulties '14'`, `spawntimesecs 604800` and `wander_distance 0`.
  - `creature_text`: the three rows from Global Constraints, plus each line Task 1 (g) found (its broadcast id and type), `TextRange 0`. If Task 1 (f) found Malfurion's speaker, add 112999 and 113000 as yells (14) for that entry. Spawning that speaker is only in scope if it already has a spawn or a DB2 placement in her room.
  - Before choosing the names, check they are free: `grep -rn "\"npc_en_\|boss_elerethe" src/server/scripts` prints nothing.
- [ ] **Step 2: Header, instance, loader.** Add the ids, the boundary and the loader lines.
- [ ] **Step 3: Boss skeleton.** Model it on `boss_nythendra.cpp:107-240`, with energy as in `boss_ilgynoth.cpp:1110-1144`.
  - `JustAppeared` / `Reset`: `SetMaxPower(POWER_ENERGY, 100)`, `SetPower(POWER_ENERGY, 0)`, remove 226039 and 226055, `_platform = 0`.
  - `JustEngagedWith`:
    - `BossAI::JustEngagedWith`, `ENCOUNTER_FRAME_ENGAGE`;
    - `DoCastSelf(SPELL_SWARM, true)`;
    - schedule `EVENT_BERSERK` at 540 s, `EVENT_ENERGY` every 1 s;
    - `StartSpiderForm()`.
  - `EVENT_ENERGY`: add the form's rate. Keep a float accumulator, so 100/92 per second reaches 100 at 92 s. At 100, call `TryFormChange()`.
  - On a successful 210308 (`OnSpellCast` or `SpellCastFinished`): `DoCastSelf(226039, true)`, then `StartRocForm()`. On a successful 210326: `DoCastSelf(226055, true)`, `++_formCount`, `StartSpiderForm()`. Both transforms are what DBM keys on (UNIT_SPELLCAST_SUCCEEDED boss1).
  - `EnterEvadeMode`:
    - disengage the frame, `summons.DespawnAll()`, `me->RemoveAllAreaTriggers()`;
    - remove the transform auras;
    - `SetDisableGravity(false)`, `SetCanFly(false)`, clear the flags `SetBusy` sets;
    - `_EnterEvadeMode()`, then `_DespawnAtEvade()` so she respawns at her DB spawn on P1;
    - tell the instance to respawn the sacs (Task 6 adds that call; here a no-op).
  - `JustDied`: disengage the frame, `summons.DespawnAll()`, `RemoveAllAreaTriggers()`, `_JustDied()`. If Task 1 (f) found Malfurion, he says his two lines 5 s apart (found with `me->FindNearestCreature(<entry>, 200.0f)`).
  - Use the Task 1 (g) lines where the wiki places them (aggro in `JustEngagedWith`, the player kill in `KilledUnit`, and the rest beside their spells in later tasks).
- [ ] **Step 4: Build and boot.**
  - `sh ~/emerald-nightmare/build.sh`. Expected: `BUILD OK`, no warnings in the touched files.
  - `sh ~/emerald-nightmare/boot_check.sh`. Expected: `READY`; the new SQL applied; DBErrors growth no more than 13422; no new Server.log ERROR lines; no "script not found".
- [ ] **Step 5: In-game checks** (`.gm on`, `.go instance emerald_nightmare`, walk to her room):
  - (a) She stands at P1's landing spot. `.npc info` shows faction 14, `boss_elerethe_renferal` and level 110.
  - (b) `.gm off` and pull. The frame shows an energy bar rising from 0. DBM shows Roc Form at 90 s and Berserk at 540 s.
  - (c) At about 90 s she casts Darkwing Form and turns into the roc; DBM restarts its Roc-form bars. At about 132 s more she turns back, and DBM shows Roc Form at 92 s.
  - (d) Leave the boundary across each platform and bridge edge: she evades only outside the union.
  - (e) `.die` in each form: she is back on P1 as a spider with 0 energy.
  - (f) `.damage` her to death: `.instance getbossstate 2` is 3 (Done). Then `.instance unbind all`.
- [ ] **Step 6: Commit and push.** `git commit -m "feat(scripts,sql): Elerethe Renferal skeleton, boundary, energy and form cycle" -- <the files>`, then `git push origin dev`.

### Task 3: Spider form: Necrotic Venom, Feeding Time, Vile Ambush, Web of Pain, Venomous Spiderlings, pools

**Files:**
- Modify: `boss_elerethe_renferal.cpp`
- Create: `sql/updates/world/master/<date>_NN_world.sql` (`spell_script_names`; area trigger 7078 rows)

**Interfaces:**
- Consumes: Task 1 (d), (h); Task 2's `StartSpiderForm`, `SetBusy` and talk ids.
- Produces:
  - Events in `PHASE_SPIDER`: `EVENT_NECROTIC_VENOM`, `EVENT_FEEDING_TIME`, `EVENT_VILE_AMBUSH`, `EVENT_WEB_OF_PAIN`.
  - **Holding.** While `_busy`, an event that fires is pushed back by 1 s with `events.Repeat(1s)`, except `EVENT_VILE_AMBUSH` and the energy tick. Task 5 uses the same rule.
  - `struct npc_en_venomous_spiderling : ScriptedAI`: Dripping Fangs on itself in `JustAppeared`, `DoZoneInCombat`, chase the nearest player; the death pool per Task 1 (d) in `JustDied`.
  - `class spell_en_necrotic_venom : SpellScript`: an `OnObjectAreaTargetSelect` filter on 215443's effect 0. It keeps the target count (1 solo, else 2 or Task 1 (h)), ranged players first.
  - `void CastWebOfPain()`: picks two distinct random players. With fewer than two it returns and logs once per fight (`TC_LOG_DEBUG`, `scripts.ai`).
  - `struct at_en_venomous_pool : AreaTriggerAI` for 7078: apply 213124 on enter, remove on exit unless the unit is in another pool. Model it on `at_nythendra_infested_ground` (`boss_nythendra.cpp:482`).

- [ ] **Step 1: SQL.**
  - `spell_script_names` (215443, `spell_en_necrotic_venom`).
  - `areatrigger_template` and `areatrigger_create_properties` for 7078: a sphere, radius 3, `DecalPropertiesId` 31, `ScriptName` `at_en_venomous_pool`. Copy the column list from `2026_10_18_05_world.sql`.
- [ ] **Step 2: Necrotic Venom and Web of Pain.** Venom at 12.2 s, then every 21.8 s, 4 casts per Spider form. Web of Pain every 30 s with `Talk(TALK_WEB_OF_PAIN)`; the boss casts 215288 and applies the 215300 / 215307 pair to the chosen players.
- [ ] **Step 3: Feeding Time.** At 15.5 s and +50 s:
  - `DoCastSelf(212364)` and `Talk(TALK_FEEDING_TIME)`.
  - `SetBusy(true)`, `UNIT_FLAG_NOT_SELECTABLE`, `SetDisableGravity(true)`, then move straight up by the rise value.
  - Summon the spiderlings around the players' centroid, at ground z (`UpdateGroundPositionZ`), so they fall from the web.
  - `EVENT_VILE_AMBUSH` 10 s later: pick a random player's position, cast 214339 at it, then clear gravity and the flag, and `SetBusy(false)` once she lands (`MovementInform` for the jump's point id).
- [ ] **Step 4: Build, boot, check** with DBM:
  - (a) Venom at 12.2 s marks Gairy; ticks after 10 s, leaving pools under Gairy that hurt while standing in them.
  - (b) Feeding Time at 15.5 s: the emote, she rises out of reach, spiderlings drop. Vile Ambush lands on Gairy's spot with a knockback, and she is selectable again.
  - (c) Spiderlings stack Dripping Fangs and leave a pool when they die. Swarm stacks on her when they are near her.
  - (d) Web of Pain never fires solo, and the debug line shows once.
  - (e) The second Feeding Time and its Vile Ambush end (about 75.5 s) before energy reaches 100, and the form change comes on DBM's 90 s bar. With DBM's times no cast overlaps Feeding Time or the flight, so the holding rule is only a guard. Task 7 re-checks it if calibration moves a timer into an overlap.
  - (f) `.die` while she is up in the web: the full reset from the Review Focus.
- [ ] **Step 5: Commit and push.** `feat(scripts,sql): Elerethe Spider form abilities and spiderlings`.

### Task 4: Roc form: Twisting Shadows, Gathering Clouds, Raking Talons, Razor Wing

**Files:**
- Modify: `boss_elerethe_renferal.cpp`

**Interfaces:**
- Consumes: Task 1 (c); Task 2's `StartRocForm`; Task 3's holding rule.
- Produces:
  - Events in `PHASE_ROC`: `EVENT_TWISTING_SHADOWS`, `EVENT_GATHERING_CLOUDS`, `EVENT_DARK_STORM` (a stub here; Task 5), `EVENT_RAKING_TALONS`, `EVENT_RAKING_TALONS_REPEAT`, `EVENT_RAZOR_WING`.
  - `struct npc_en_twisting_shadows : ScriptedAI`: per Task 1 (c), cast 210850 on itself in `JustAppeared` if the summon does not carry it, and follow the 221877 target if it has one. Despawn when 210850 ends.
  - The boss gets the tornado through `JustSummoned` (the summon's caster is the marked player, so check this). If it is not in `summons`, register it from the tornado AI with the instance's boss.

- [ ] **Step 1: Roc events.** The times are from Global Constraints.
  - Twisting Shadows: cast 210864 at a random player.
  - Gathering Clouds: `Talk(TALK_GATHERING_CLOUDS)`, cast 212707.
  - Raking Talons: cast 215582 on the victim, then `EVENT_RAKING_TALONS_REPEAT` when the cast ends (one 222037).
  - Razor Wing: cast 210547 at the victim.
- [ ] **Step 2: Tornado AI.** As in the Interfaces block.
- [ ] **Step 3: Build, boot, check** with DBM (`.die` isn't needed between checks; wait out the form):
  - (a) Each cast lands within 2 s of its DBM bar.
  - (b) A Twisting Shadows mark on Gairy turns into a tornado where Gairy stood 10 s later. It pulls and hurts for 4 s, then goes.
  - (c) Gathering Clouds pushes Gairy back for 8 s.
  - (d) Raking Talons hits twice and stacks the debuff. Razor Wing hits only in front of her.
  - (e) `.die` during Roc form: no tornado left; she is a spider on P1.
- [ ] **Step 4: Commit and push.** `feat(scripts): Elerethe Roc form abilities and tornadoes`.

### Task 5: The flight: Feathers, Dark Storm, the Eye of the Storm, the bridge drop

**Files:**
- Modify: `boss_elerethe_renferal.cpp`
- Create: `sql/updates/world/master/<date>_NN_world.sql` (custom area triggers 152003 and 152004; `spell_script_names`)

**Interfaces:**
- Consumes: Task 2's `FlightPaths`, `Platforms`, `BridgeMidpoints`, `SetBusy`; Task 4's tornado AI; Task 3's spiderling AI.
- Produces:
  - `void StartFlight()`, run after a successful 210948 when `_rocCount <= 2` and `_platform < 2`.
  - `struct at_en_eye_of_the_storm : AreaTriggerAI`: apply 211127 on enter, remove it on exit and on remove.
  - `struct at_en_shimmering_feather : AreaTriggerAI`: on the first player's enter, apply 212993 to them and remove the area trigger.
  - `class spell_en_dark_storm : AuraScript` on a custom periodic. Use 211122 itself if its 1 s trigger can be filtered; otherwise a 1 s boss event does the same. Each tick casts 213936 at each player inside the boundary who lacks 211127. The tick target filter is `spell_en_dark_storm_damage : SpellScript` on 213936 effect 0, removing units with 211127.
  - Foul Winds (211137) is applied with the storm and removed with it after 45 s.

- [ ] **Step 1: SQL.**
  - `areatrigger_template` and `areatrigger_create_properties` for 152003: custom, a sphere of radius 15, `ScriptName` `at_en_eye_of_the_storm`, a decal by eye (24 to start).
  - The same for 152004: a sphere of radius 1.5, `ScriptName` `at_en_shimmering_feather`, a decal by eye.
  - `spell_script_names` for the storm scripts.
- [ ] **Step 2: Flight.** On 210948's successful cast:
  - Drop the Feathers around her (`me->CastSpell` of a custom create, or `AreaTrigger::CreateAreaTrigger(152004, ...)` as the scaffold's static triggers are made; pick the one this fork supports for a non-spell create).
  - Start the storm and Foul Winds.
  - Summon the Eye at the next landing spot.
  - `SetBusy(true)`, not selectable, fly mode, `MoveSmoothPath` along `FlightPaths[_platform]`.
  - On arrival, `++_platform`, land (gravity on, fly off, selectable), `SetBusy(false)`.
  - Summon the bridge drop at `BridgeMidpoints[_platform - 1]`: 2 spiderlings and 1 tornado.
  - The flight is skipped after the second one (Berserk ends the fight first).
- [ ] **Step 3: Death and evade in flight.** `JustDied` and `EnterEvadeMode` also stop the storm (remove the storm aura and Foul Winds from her, and 211127 / 211137 from players) and drop her to the ground (`SetDisableGravity(false)`, `SetCanFly(false)`, `GetMotionMaster()->MoveFall()`).
- [ ] **Step 4: Build, boot, check:**
  - (a) At 26 s into the first Roc form, Dark Storm is cast. Feathers lie where she stood, and she flies over the bridge to P2's landing spot in about 8 s.
  - (b) On P1, the storm ticks Gairy every 1 s, harder each tick. In the Eye on P2 the ticks stop; stepping out brings them back until 45 s.
  - (c) A Feather gives 212993 and disappears.
  - (d) 2 spiderlings and 1 tornado appear at the bridge midpoint.
  - (e) The Raking Talons and Razor Wing bars still match DBM after the landing.
  - (f) Second Roc form: the flight goes to P3. The third Spider form never leads to a flight; Berserk at 540 s.
  - (g) `.die` mid-flight, and `.damage` her to death mid-flight: the Review Focus outcomes.
- [ ] **Step 5: Commit and push.** `feat(scripts,sql): Elerethe flight, Dark Storm and the Eye of the Storm`.

### Task 6: The bridges: Web Crawlers, egg sacs, Skittering Spiderlings, the Feather crush

**Files:**
- Modify: `boss_elerethe_renferal.cpp`
- Modify: `instance_emerald_nightmare.cpp` (sac respawn on reset)
- Create: `sql/updates/world/master/<date>_NN_world.sql` (custom area triggers 152005 and 152006, their static spawns, `spell_script_names` if a crush script is needed)

**Interfaces:**
- Consumes: Task 1 (b), (e); Task 2's sac spawns and `NPC_ELERETHE_EGG_SAC`.
- Produces:
  - `struct at_en_web_bridge : AreaTriggerAI`: on enter, apply 210327 to players while the encounter is `IN_PROGRESS`; on exit and on remove, take it off.
  - `struct npc_en_elerethe_egg_sac : ScriptedAI`: every 500 ms, if a player is within 3 yd, summon 4 Skittering Spiderlings at itself and `KillSelf()`. The summoner is the boss (`instance->GetCreature(DATA_ELERETHE_RENFERAL)`), so evade clears them. The sac does nothing when the encounter is not in progress.
  - `struct npc_en_skittering_spiderling : ScriptedAI`: chase the sac's popper, melee.
  - In the instance: on `SetBossState(DATA_ELERETHE_RENFERAL, NOT_STARTED)`, respawn every dead sac. Keep their guids from `OnCreatureCreate` in a `GuidVector`.
  - The crush, per Task 1 (e): either the data's proc, or `class spell_en_shimmering_feather : AuraScript` that, on the player landing (an `OnEffectPeriodic` that compares `IsFalling` with the last tick, or the hook Task 1 names), casts 221994 if a Venomous or Skittering Spiderling is within 3 yd.
  - The boss's `EnterEvadeMode` calls the sac respawn and removes 210327 and 212993 from players in the map.

- [ ] **Step 1: SQL.**
  - `areatrigger_template` and `areatrigger_create_properties` for 152005 and 152006: boxes fitted to each bridge (`Shape` 1, `ShapeData0-2` half-extents), `ScriptName` `at_en_web_bridge`, no decal (the web is scenery).
  - `areatrigger` spawns 1520002 and 1520003 at each bridge's midpoint, facing along it, `SpawnDifficulties '14'`.
- [ ] **Step 2:** The three AIs, the instance respawn, and the crush, as in the Interfaces block.
- [ ] **Step 3: Build, boot, check:**
  - (a) Out of combat, walking the web spawns nothing, and sacs stay put.
  - (b) In combat, walking the web spawns a Venomous Spiderling every 4 s; leaving it stops them, and the aura is gone.
  - (c) Walking past a sac: 4 Skittering Spiderlings come out and chase Gairy.
  - (d) With a Feather, Gairy jumps higher, takes no fall damage, and kills a spiderling by landing on it. Record whether the data's proc or the script did it.
  - (e) `.die`: the sacs are back, no spiderling is left, Gairy has no Web Crawlers or Feather aura.
  - (f) Die on the web, release, run back: no spiderlings spawn from the dead player.
- [ ] **Step 4: Commit and push.** `feat(scripts,sql): Elerethe bridges, egg sacs and the Feather crush`.

### Task 7: Calibration and wrap-up

**Files:**
- Modify: `boss_elerethe_renferal.cpp` (constants only)
- Modify: the spec ("Sources and what is uncertain", "Needs a group")
- Modify: `README.md` (her row)

- [ ] **Step 1:** Run a full fight with DBM 7.3.31 and note each event's real time against the Global Constraints. Change any constant that is off by more than 2 s from DBM. For what DBM does not time (the calibrate values), compare against a retail Normal video if the user supplies one; otherwise keep the start values and say so.
- [ ] **Step 2:** After any constant change, run the regression: Task 5 Step 4 (g) and Task 6 Step 3 (e).
- [ ] **Step 3:** Mark settled items in the spec. Move every mechanic the user could not confirm solo into "Needs a group".
- [ ] **Step 4:** Change her README row to the status the testing reached ("Playable, in testing" or "Playable, tested in game"), with its "What is in" text still at table level. Update the summary row to match.
- [ ] **Step 5:** Copy the spec and this plan to `C:\Users\chris\code\acore-webservice\docs\superpowers\` (`specs/` and `plans/`). Commit `docs: Elerethe calibration notes and README status`, push. Update the user's memory note `project_emerald_nightmare.md` with the commits and lessons.
