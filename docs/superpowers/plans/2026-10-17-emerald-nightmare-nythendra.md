# Emerald Nightmare: Nythendra Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Nythendra (102672) can be pulled, fought through both phases and killed on Normal in the Emerald Nightmare, with retail Normal mechanics.

**Architecture:** One `BossAI` (`boss_nythendra`) drives an energy-based two-phase event schedule. Spell and aura scripts own Rot, Volatile Rot and Infested Breath side effects; an `AreaTriggerAI` owns Infested Ground and registers each pool with the boss so Phase 2 can pull them in. The instance script adds a circle boundary; no doors.

**Tech Stack:** C++ (ArgusCore / TrinityCore script API), MySQL world SQL updates, Python 3 with `/tmp/stage1/wdc1.py` for DB2 dumps, Docker build via `~/emerald-nightmare/build.sh`.

**Spec:** `docs/superpowers/specs/2026-10-17-emerald-nightmare-nythendra-design.md`

## Global Constraints

- Work over SSH on `chris@172.30.1.185` in `~/arguscore`, branch `dev` (spec; `docs/BRANCHING.md` rule 3).
- One session owns `~/arguscore` at a time: before the first build ask the "Priest class hall engine work" session, check the tree is clean and `pgrep -x docker-buildx` prints nothing (rules 9, 10).
- Never `--no-cache`. No server restart while a character is online or a client is open (rule 13; `boot_check.sh` refuses if one is).
- One kind per commit: `feat(scripts,sql)` for script + SQL; commit only files you changed; push `dev` after each stage commit.
- SQL: one file per logical change, `sql/updates/world/master/2026_10_17_NN_world.sql` (`00` and `01` are taken; use the next free `NN`), re-runnable (`DELETE` then `INSERT`/`UPDATE`).
- No leak-derived sources (LegionCore etc.). Normal difficulty only; spawn on difficulty `14`.
- Boss values: `DATA_NYTHENDRA = 0`, encounter 1853, creature 102672, Corrupted Vermin 111005, Rot 203096, Volatile Rot 204463, Infested Breath 202977, Infested Ground 203045, Heart of the Swarm 203552, Burst of Corruption 203646.
- Timings (calibrate items): Phase 1 breaths at 45 s and 90 s, Rot every 16 s (first at 8 s; 1 target, 2 after first breath), Volatile Rot at 20/50/80 s on the tank, Heart of the Swarm 3.7 s cast + 20 s channel, first swell at 10 s then one per second, Burst x3 over 3 s.
- Creature text: group 0 type 41 broadcast 106119; group 1 type 42 broadcast 105376; group 2 type 42 broadcast 105969.
- Tests here are in-game checks (the scripts have no unit-test harness); each task's "test" is the exact observation that must hold.

## Review Focus

- Evade or wipe during Phase 2: no vermin, pools or channel aura left; next pull starts at 100 energy in Phase 1 (Task 4).
- Solo player (only the tank): Rot and Volatile Rot select the tank instead of failing, and a 0-target selection does not crash (Task 3).
- Rot target dies or leaves before the 9 s end: no pools from a non-expired removal; no stale registry entry (Task 3).
- Boss pulled outside the boundary or a player leaves the boundary mid-fight: boss evades cleanly, then respawns/resets (Task 2).
- Kill during Phase 2 or with a Volatile Rot in flight: encounter reaches Done, nothing keeps ticking afterwards (Task 4).

---

### Task 1: Dump the spell data

**Files:**
- Create: `~/emerald-nightmare/dump_spells.py` (host, not in the repo)
- Create: `~/emerald-nightmare/nythendra_spells.txt` (output)

**Interfaces:**
- Produces: for each of 202977, 202978, 203045, 203096, 203097, 203537, 203552, 203646, 204463, 204470: its `SpellEffect` rows (effect, aura, trigger spell, target pair, period, radius), cast time, duration, and whether it creates an area trigger (and that trigger's create-properties id). Later tasks read this file to decide which scripts are needed.

- [ ] **Step 1:** Write `dump_spells.py` using `wdc1.read` on `SpellEffect.db2`, `SpellMisc.db2`, `SpellCastTimes.db2`, `SpellDuration.db2` from `~/arguscore-server/data/dbc/enUS/` (the field layout is the core's `SpellEffectEntry`/`SpellMiscEntry` in `src/server/game/DataStores/DB2Structure.h`).
- [ ] **Step 2:** Run it for the ids above and save the output. Expected: every id prints at least one effect row; no exception.
- [ ] **Step 3:** Answer in a short note at the top of `nythendra_spells.txt`: (a) which spell creates Infested Ground and via which create-properties id, and whether `areatrigger_create_properties` already has it; (b) whether Rot expiry, Volatile Rot detonation and Burst's 25% stack are driven by the spell data itself; (c) the real cast times and durations for the breath. Copy any answer that contradicts the spec's "Retail mechanics" table into the shared notes folder and tell the user before continuing.

### Task 2: Skeleton boss, boundary, spawn (engage, evade, die)

**Files:**
- Create: `src/server/scripts/BrokenIsles/EmeraldNightmare/boss_nythendra.cpp`
- Modify: `src/server/scripts/BrokenIsles/EmeraldNightmare/emerald_nightmare.h` (add `NPC_CORRUPTED_VERMIN = 111005` to `ENCreatureIds`)
- Modify: `src/server/scripts/BrokenIsles/EmeraldNightmare/instance_emerald_nightmare.cpp` (boundary)
- Modify: `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp:36,69` (declare and call `AddSC_boss_nythendra()` under `// Emerald Nightmare`)
- Create: `sql/updates/world/master/2026_10_17_NN_world.sql`

**Interfaces:**
- Produces: `struct boss_nythendra : public BossAI` with the engage/evade/die skeleton; `enum NythendraEvents`, `enum NythendraSpells`, `enum NythendraTalks { TALK_INFESTED_BREATH = 0, TALK_ROT = 1, TALK_VOLATILE_ROT = 2 }`, `enum NythendraPhases { PHASE_ONE = 1, PHASE_TWO }`; `void AddSC_boss_nythendra()`; the SQL rows for template, creature text and spawn.

**Gate:** needs the user's three `.gps` readings (spawn; three or four room-edge points; entrance point), per the spec's "Measured inputs". If they are not in hand, ask the user for them now and wait. Put them in as `ROOM_CENTER`/`ROOM_RADIUS` constants at the top of `instance_emerald_nightmare.cpp` and as the spawn row.

- [ ] **Step 1: Write the SQL file.** Blocks, each preceded by a comment and re-runnable: `UPDATE creature_template SET faction = 14, ScriptName = 'boss_nythendra' WHERE entry = 102672`; `UPDATE creature_template SET faction = 14, unit_flags = <NON_ATTACKABLE|UNINTERACTIBLE> WHERE entry = 111005` (and `speed_walk`/`speed_run` 0); `DELETE`/`INSERT creature_text` for 102672, three rows per the Global Constraints, `Text` carrying the English broadcast text, `TextRange` 0; `DELETE`/`INSERT creature` with `map` 1520, `id` 102672, `spawnDifficulties` `'14'`, `spawntimesecs` 604800, `wander_distance` 0, `MovementType` 0, position from the readings, a free `guid` (query `SELECT MAX(guid)+1 FROM creature` and check against 1520xxx habits).
- [ ] **Step 2: Header.** Add `NPC_CORRUPTED_VERMIN` to `ENCreatureIds` under a `// Nythendra` comment.
- [ ] **Step 3: Instance script.** Add `#include "AreaBoundary.h"`, a `BossBoundaryData const boundaries = { { DATA_NYTHENDRA, new CircleBoundary(Position(<centre>), <radius>) } };` (the `CircleBoundary(Position const& center, double radius)` form from `AreaBoundary.h`), and `LoadBossBoundaries(boundaries);` after `LoadObjectData`. No `DoorData`.
- [ ] **Step 4: Boss skeleton** in `boss_nythendra.cpp`, modelled on `boss_guarm.cpp:150-260`: `JustAppeared` sets `SetMaxPower(POWER_ENERGY, 100)` and `SetPower(POWER_ENERGY, 100)`; `JustEngagedWith` calls `BossAI::JustEngagedWith` and `instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me, 1)`; `EnterEvadeMode` sends `ENCOUNTER_FRAME_DISENGAGE`, `summons.DespawnAll()`, `_EnterEvadeMode()`, `_DespawnAtEvade()`; `JustDied` sends disengage and `_JustDied()`; `UpdateAI` is `if (!UpdateVictim()) return;` then `DoMeleeAttackIfReady()`. Register with `RegisterEmeraldNightmareCreatureAI(boss_nythendra)` in `AddSC_boss_nythendra`.
- [ ] **Step 5: Loader.** Add the declaration and the call.
- [ ] **Step 6: Build and boot.** Ask the Priest session, then `sh ~/emerald-nightmare/build.sh` (Expected: `BUILD OK`, no warnings in touched files), then `sh ~/emerald-nightmare/boot_check.sh` (Expected: `READY`, the new SQL listed under updates applied, DBErrors growth not above baseline 13422, no new Server.log ERROR lines).
- [ ] **Step 7: In-game checks.** `.gm on`, `.go instance emerald_nightmare`, go to the room: (a) she is visible at the measured spot; `.npc info` shows faction 14 and script `boss_nythendra`; (b) hitting her shows the encounter frame and energy 100; her swings land (if `BaseAttackTime` 0 means she never swings, add `UPDATE creature_template SET BaseAttackTime = 2000 ...` to the SQL, rebuild); (c) walking out of the boundary makes her evade and reset; (d) `.die` on a GM'd player resets her; (e) `.damage` kills her and `.instance getbossstate 0` shows Done (3).
- [ ] **Step 8: Commit and push.** `git add` the six files; `git commit -m "feat(scripts,sql): Nythendra skeleton, boundary and spawn"`; `git push origin dev`.

### Task 3: Phase 1 — Rot, Volatile Rot, Infested Breath, Infested Ground

**Files:**
- Modify: `boss_nythendra.cpp`
- Modify: the SQL file from Task 2 (or a new `NN+1` file if Task 2 is already pushed): `spell_script_names` rows and, only if Task 1 says it is missing, `areatrigger_template`/`areatrigger_create_properties` rows for Infested Ground (free ids checked first)

**Interfaces:**
- Consumes: Task 1 findings (which scripts are really needed); Task 2 enums.
- Produces: `class at_nythendra_infested_ground : public AreaTriggerEntityScript` whose AI, on `OnCreate`, finds the boss through `AreaTrigger::GetCaster()` and calls `boss_nythendra::RegisterPool(ObjectGuid)`; the boss keeps `GuidUnorderedSet _pools`; `spell_nythendra_rot_select` (`SpellScript`, `OnObjectAreaTargetSelect`/`OnObjectTargetSelect` on the Rot cast) and `spell_nythendra_rot_expire` (`AuraScript`, `AfterRemove` for `AURA_REMOVE_BY_EXPIRE` only); `uint8 _rotTargets`.

- [ ] **Step 1: Events.** In `JustEngagedWith` schedule in `PHASE_ONE`: `EVENT_ROT` at 8 s then every 16 s (cast `SPELL_ROT` on `_rotTargets` random non-tank players, falling back to the victim when there is none, and `Talk(TALK_ROT, target)`); `EVENT_VOLATILE_ROT` at 20 s, 50 s, 80 s (`DoCastVictim(SPELL_VOLATILE_ROT)` and `Talk(TALK_VOLATILE_ROT, victim)`); `EVENT_INFESTED_BREATH` at 45 s and 90 s (`Talk(TALK_INFESTED_BREATH)`, `DoCast(randomPlayer, SPELL_INFESTED_BREATH)`, on `SpellCastCompleted`/`OnSpellCastFinished` subtract 50 energy via `SetPower(POWER_ENERGY, GetPower(POWER_ENERGY) - 50)` and set `_rotTargets = 2` after the first).
- [ ] **Step 2: Spell scripts** `spell_nythendra_rot_expire` (create three Infested Ground at the target's position, spread around it, via `CastSpell` of the pool spell Task 1 identified) and `spell_nythendra_volatile_rot_expire` (several pools at the target on expiry). Add only if Task 1 says the spell data does not already do it. Register both with `RegisterSpellScript` and the `spell_script_names` rows.
- [ ] **Step 3: Area trigger.** Infested Ground is created by spell 203044 (create-properties id 5887, no world row today): add `areatrigger_template` and `areatrigger_create_properties` rows for 5887 with the shape read from the client `AreaTrigger.db2`, and `ScriptName` `at_nythendra_infested_ground`. `at_nythendra_infested_ground` registers itself with the boss as described in Interfaces. Pool damage is the spell's aura 203045 applied to units inside (`OnUnitEnter`/`OnUnitExit`).
- [ ] **Step 4: Build, boot, check.** In game as Gairy solo (`.gm on`, `.cheat god`): (a) first Rot at about 8 s, then every 16 s, whisper text shows; (b) 9 s after Rot, three pools appear where Gairy stood and tick every 1.5 s; (c) Volatile Rot at about 20, 50, 80 s, 8 s fuse, pools after; (d) the breath emote shows at about 45 s, the cast takes 3 s, the cone ticks for 5 s, energy reads 50; (e) Rot on a player who leaves the instance before 9 s makes no pools and no error in `Server.log`.
- [ ] **Step 5: Commit and push.** `git commit -m "feat(scripts,sql): Nythendra phase one"`.

### Task 4: Phase 2 — Heart of the Swarm

**Files:**
- Modify: `boss_nythendra.cpp`; the SQL file (any new `spell_script_names`)

**Interfaces:**
- Consumes: `GuidUnorderedSet _pools` and `RegisterPool` from Task 3.
- Produces: `void StartPhaseTwo()`, `void EndPhaseTwo()`, events `EVENT_HEART_OF_THE_SWARM`, `EVENT_SWELL_VERMIN`, `EVENT_END_CHANNEL`; `PHASE_TWO` with its own energy ramp event.

- [ ] **Step 1: Transition.** After the second breath, energy is 0: cancel `PHASE_ONE` events, stop melee, `me->SetControlled(true, UNIT_STATE_ROOT)`, `DoCastSelf(SPELL_HEART_OF_THE_SWARM)` (3.7 s cast, then the channel per Task 1's finding; if the spell has no channel, hold her for 20 s with an event).
- [ ] **Step 2: Pools and recharge.** At channel start, for each registered pool call `at->InitSplines({ pool pos, boss pos }, <about 4 s worth of speed>)`, then remove it when it arrives (`OnDestinationReached`). The energy recharge is the spell's own periodic energize (+5 per second, Task 1), so the script does not ramp energy.
- [ ] **Step 3: Vermin.** At channel start summon about 40 `NPC_CORRUPTED_VERMIN` at random points inside the boundary (`me->SummonCreature` with `TEMPSUMMON_MANUAL_DESPAWN`, positions from `GetRandomPoint` clamped by `IsWithinBoundary`). From 10 s, once per second pick a random live vermin from `summons`, make it cast `SPELL_BURST_OF_CORRUPTION` three times at 1 s intervals (an event on the boss, `vermin->CastSpell`), then despawn it.
- [ ] **Step 4: Back to Phase 1.** At 20 s: `summons.DespawnAll()`, remove root, `SetPower(POWER_ENERGY, 100)`, `_rotTargets = 1`, reschedule the Phase 1 events from Task 3.
- [ ] **Step 5: Evade and death.** `EnterEvadeMode` and `JustDied` also remove root, `summons.DespawnAll()`, despawn every registered pool, `_pools.clear()`.
- [ ] **Step 6: Build, boot, check.** Solo: (a) at about 90 s she stops and casts Heart of the Swarm for 3.7 s then holds for about 20 s; (b) pools move to her and vanish; (c) vermin are on the ground, untargetable; from about 10 s one swells per second and Burst of Corruption hits three times, damage stacking 25% for 5 s; (d) after the channel energy is 100, no vermin or pools remain, Phase 1 restarts (Rot at 8 s); (e) `.die` mid-channel resets her: full energy, Phase 1 on the next pull, no vermin; (f) `.damage` her to death mid-channel: Done, nothing left on the ground.
- [ ] **Step 7: Commit and push.** `git commit -m "feat(scripts,sql): Nythendra phase two"`.

### Task 5: Calibration and wrap-up

**Files:**
- Modify: `boss_nythendra.cpp` (constants only), the spec's "Sources and what is uncertain" list if a number is settled

- [x] **Step 1:** Run a full fight and write down the real times of each event against the Global Constraints timings; for any ability that is wrong by more than a few seconds, compare against a retail video (user supplies the link, as for the Broken Shore) and change the constant.
- [x] **Step 2:** Final regression: re-run the Task 2 and Task 4 checks that touch evade and kill after any constant change.
- [x] **Step 3:** Update `docs/superpowers/specs/2026-10-17-emerald-nightmare-nythendra-design.md` "Sources and what is uncertain" to mark settled items; copy the spec and this plan to `C:\Users\chris\code\acore-webservice\docs\superpowers\`; commit `docs: Nythendra calibration notes`; push.
