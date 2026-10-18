# Emerald Nightmare: Il'gynoth Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Il'gynoth (105393) can be pulled, fought through the outside and heart phases, and killed on Normal in the Emerald Nightmare, with his retail Normal mechanics.

**Architecture:**
- **Heart.** The heart's `BossAI` (`boss_ilgynoth`) runs the fight. It owns the outside/heart phase machine, the wave schedule (DBM times, Icy Veins counts) and every summon. It toggles the forcefield gameobject 251555 through the instance.
- **Eye and adds.** The Eye and each add type have a small `ScriptedAI`. The Nightmare Ichor alone applies the Eye's 5% loss.
- **Spells and pools.** Spell/aura scripts cover Spew Corruption, Cursed Blood and Dark Reconstitution. An `AreaTriggerAI` handles the Nightmare Corruption pool.

**Tech Stack:**
- C++ with the ArgusCore / TrinityCore script API.
- MySQL world SQL updates.
- Python 3 with `/tmp/stage1/wdc1.py` and `~/emerald-nightmare/dump_ilg2.py` for the DB2 dumps.
- The Docker build through `~/emerald-nightmare/build.sh`.

**Spec:** `docs/superpowers/specs/2026-10-17-emerald-nightmare-ilgynoth-design.md`

## Global Constraints

**Where and how to work**
- Work over SSH on `chris@172.30.1.185` in `~/arguscore`, branch `dev` (`docs/BRANCHING.md` rule 3).
- Only one session uses `~/arguscore` at a time. Before every build:
  - ask the session holding the folder (the Nythendra calibration sessions or "Priest class hall engine work");
  - check the tree has only your own changes;
  - check `pgrep -x docker-buildx` prints nothing (rules 9, 10).
- Never use `--no-cache`. Never restart while a character is online (rule 13; `boot_check.sh` refuses if one is).
- Commits: one kind per commit (`feat(scripts,sql)` for script plus SQL). Commit only files you changed, with `git commit -- <paths>`, because other sessions leave edits in the tree. Push `dev` after each stage commit.
- Core (engine) fixes go on `engine` (`~/arguscore-engine`), then merge into `dev`. Never in this plan's commits.
- No leak-derived sources (LegionCore and similar). Normal only; spawn difficulty `'14'`.

**SQL files**
- One file per task: `sql/updates/world/master/<date>_NN_world.sql`, with the next free `NN` on the day of the work (`ls sql/updates/world/master | tail`).
- Each file is re-runnable: `DELETE` then `INSERT`/`UPDATE`.
- SQL-only changes still need a rebuild: the image COPYs `sql/`.

**Ids**

| What | Id |
|---|---|
| Data id | `DATA_ILGYNOTH = 1`, encounter 1873 |
| Heart | 105393 |
| Eye | 105906 |
| Dominator Tentacle | 105304 |
| Deathglare Tentacle | 105322 |
| Corruptor Tentacle | 105383 |
| Nightmare Horror | 105591 |
| Nightmare Ichor | 105721 |
| Forcefield (gameobject) | 251555 |

**Spells**

| Spell | Id |
|---|---|
| Stuff of Nightmares | 209915 |
| Nightmare Gaze | 210931 |
| Ichor summon | 209966 |
| Fixate | 210099 |
| Touch of Corruption proc | 209467 |
| Nightmare Explosion | 209471 |
| Eye of Fate | 210984 |
| Seeping Corruption | 209387 |
| Nightmare Corruption pool | 208931 (area trigger 6514, aura 212886) |
| Mind Flay | 208697 |
| Spew Corruption | 208929 |
| Nightmarish Fury | 215234 |
| Ground Slam | 208689 |
| Rupturing Roar | 208685 |
| Horror visual | 210289 |
| Dark Reconstitution | 210781 |
| Final Torpor | 223121 |
| Cursed Blood | 215128 |
| Cursed Blood burst | 215143 |
| Reabsorption | 212942 |
| Violent Bloodburst | 215971 (→ 215975, area trigger 7424) |

**Text**

| Line | Broadcast | Type |
|---|---|---|
| Heart: pull | 109119 | yell (14) |
| Heart: heart phase | 109141 | yell (14) |
| Heart: reconstitution done | 109142 | yell (14) |
| Heart: low health (30%) | 109143 | yell (14) |
| Heart: death | 109144 | yell (14) |
| Heart: Dark Reconstitution almost done | 109470 | emote (41) |
| Heart: Final Torpor almost done | 122666 | emote (41) |
| Eye: Horror coming | 111379 | emote (41) |
| Dominator: Ground Slam | 109189 | whisper (42) |

**Wave tables** (seconds from the start of the outside phase; DBM 7.3.31, counts from Icy Veins)

| Add | First outside phase | After a heart phase |
|---|---|---|
| Dominator | 2 at 0 | 2 at 0 |
| Deathglare | 21.5×1, 83.5×2, 168.5×1, 223.5×1 | 21.5×1, 111.5×2, 241.5×1 |
| Corruptor | 79×2, 174×2, 209×1 | 45×2, 140×2, 175×1, 260×1, 300×1 |
| Horror | 60, then every 280 | 95, then every 280 |
| Afterwards | 1 random tentacle (Deathglare or Corruptor) every 60 s | same |

Ichors per death: Dominator 2, Deathglare 2, Corruptor 1, Horror 4. Twenty explosions in range kill the Eye; the range is 13 yd.

**Ability timers**

| Unit | Ability | Timing |
|---|---|---|
| Dominator | Fury | first 6 s, then 10.9 s |
| Dominator | Slam | first 12 s, then 20.5 s |
| Horror | Eye of Fate | first 14 s, then 10 s |
| Eye | Nightmare Gaze | every 2-3 s |
| Heart | Cursed Blood | every 15 s; targets 1 solo, else `clamp(players / 5, 2, 6)` |
| Heart | Dark Reconstitution | 50 s cast, heart phase 1 |
| Heart | Final Torpor | 90 s cast, heart phase 2 |

**Tests** are in-game checks; the scripts have no unit-test harness. Each task's test is the exact observation that must hold. DBM 7.3.31 is installed in the client (`E:\legion\interface\addons`), and its bars are the timing reference.

## Review Focus

- **Wipe or evade in either phase.** Afterwards no add, Ichor or pool is left. The forcefield is closed, the Eye is back at full health with Stuff of Nightmares, the heart is at full health with phase count 0, and the next pull starts with 2 Dominators. Task 2 tests a wipe in the outside phase, Task 5 one in the heart phase.
- **Ichor fixate target dies or leaves the boundary.** The Ichor picks a new random player. With no player left it does nothing and despawns on evade. It never chases a dead player or crashes on a null target (Task 4).
- **Solo raid.** Cursed Blood picks 1 target. Ground Slam, Mind Flay and Spew pick the only player. No selection loops on zero candidates (Tasks 3 and 5).
- **Ichor still in its 2.5 s explosion cast when the Eye dies**, or two explosions in one tick taking the Eye under 0. The Eye dies exactly once and `ACTION_EYE_DIED` fires once (Task 4).
- **Heart killed during Dark Reconstitution, or with adds outside.** The boss state goes to Done, the cast stops (nobody is killed by it later), all adds despawn, the forcefield stays open after a relog, and the Eye does not respawn (Task 5).

---

### Task 1: Dump the spell and area trigger data

**Files:**
- Modify: `~/emerald-nightmare/dump_ilg2.py` (host, not in the repo)
- Create: `~/emerald-nightmare/ilgynoth_task1.txt` (output)

**Interfaces:**
- Produces: answers, written as a note at the top of `ilgynoth_task1.txt` and copied to the shared notes folder (`ilgynoth-task1-findings.md`), that Tasks 2-5 read:
  - (a) the shape and size of area triggers 6514 and 7424 from `AreaTrigger.db2`, `AreaTriggerSphere.db2` and `AreaTriggerCylinder.db2`, and whether a decal id is available. Look at `DecalPropertiesId` in `AreaTrigger.db2`; the Nythendra SQL `2026_10_17_05` shows how it was set.
  - (b) what 210781 and 223121 target (implicit target types and radius) and whether a target filter on effect 0 can cut them down to "inside";
  - (c) Violent Bloodburst 215971: who casts it, and whether anything in the data triggers it. Decide whether it is built or recorded as a known gap.
  - (d) Spew Corruption 208929 on Normal: cast time, and target type on effect 0 (does it hit several nearby players?);
  - (e) the radius behind 209471's radius index 13, and 215143's radius;
  - (f) the forcefield's display 6391: its model file name from `GameObjectDisplayInfo.db2`, so the tester knows what to look for.

- [ ] **Step 1:** Extend `dump_ilg2.py` with three things: `SpellRadius.db2` lookups for the radius indexes it prints; `SpellTargetRestrictions` (`MaxTargets`) for 215128, 208929 and 210781; and an `AreaTrigger.db2` row dump by id. Use the same `wdc1.read` / `rel` pattern as the file. The field layouts are `SpellRadiusEntry`, `SpellTargetRestrictionsEntry` and `AreaTriggerEntry` in `src/server/game/DataStores/DB2Structure.h`.
- [ ] **Step 2:** Run it for every spell in the Global Constraints. Expected: every id prints at least one effect row and the script raises no exception. Save the output to `ilgynoth_task1.txt`.
- [ ] **Step 3:** Write answers (a)-(f) at the top. If an answer contradicts the spec's "The encounter" section, put it in the findings note and tell the user before Task 2.

### Task 2: Skeleton: heart, Eye, forcefield, boundary, spawns (pull, evade, die)

**Files:**
- Create: `src/server/scripts/BrokenIsles/EmeraldNightmare/boss_ilgynoth.cpp`
- Modify: `src/server/scripts/BrokenIsles/EmeraldNightmare/emerald_nightmare.h`
- Modify: `src/server/scripts/BrokenIsles/EmeraldNightmare/instance_emerald_nightmare.cpp`
- Modify: `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp` (declare and call `AddSC_boss_ilgynoth()` under `// Emerald Nightmare`, next to Nythendra)
- Create: `sql/updates/world/master/<date>_NN_world.sql`

**Gate:** this task needs the user's `.gps` readings, items 1-9 of the spec's "Measured inputs":
- How to read them: each `.gps` command line in `GM.log` carries the player's X/Y/Z.
- If the readings are not in hand, ask the user and wait.
- Fit the room-edge points to a boundary, a circle unless clearly not round, at the floor z, the way the Nythendra boundary was fitted. Check that it contains every other reading.
- Fit the chamber-edge points to the chamber shape, a circle or `std::array` polygon, with its z.

**Interfaces:**
- Produces, in `emerald_nightmare.h`:
  - Under `// Il'gynoth` in `ENCreatureIds`: `NPC_EYE_OF_ILGYNOTH = 105906`, `NPC_DOMINATOR_TENTACLE = 105304`, `NPC_DEATHGLARE_TENTACLE = 105322`, `NPC_CORRUPTOR_TENTACLE = 105383`, `NPC_NIGHTMARE_HORROR = 105591`, `NPC_NIGHTMARE_ICHOR = 105721`.
  - `enum ENGameObjectIds { GO_EYE_OF_ILGYNOTH_FORCEFIELD = 251555 };`.
  - In `ENDataTypes`, after `DATA_XAVIUS`: `DATA_EYE_OF_ILGYNOTH`, `DATA_ILGYNOTH_FORCEFIELD`.
- Produces, in `boss_ilgynoth.cpp`:
  - `struct boss_ilgynoth : BossAI`, with `DoAction(int32)` handling `ACTION_PULL = 1`, `ACTION_EYE_DIED = 2` and `ACTION_ICHOR_INSIDE = 3`.
  - `void SetForcefield(bool open)`, which calls `instance->HandleGameObject(instance->GetGuidData(DATA_ILGYNOTH_FORCEFIELD), open)`.
  - `void StartOutsidePhase(bool afterHeart)` and `void StartHeartPhase()`, stubs in this task.
  - `struct npc_eye_of_ilgynoth : ScriptedAI`.
  - `bool IsInsideChamber(WorldObject const* obj)`, a file-level function using the chamber constants.
  - `enum IlgynothTalks { TALK_AGGRO = 0, TALK_HEART_PHASE, TALK_RECONSTITUTED, TALK_LOW_HEALTH, TALK_DEATH, TALK_RECONSTITUTION_WARN, TALK_TORPOR_WARN }`, plus `TALK_EYE_HORROR = 0` for the Eye and `TALK_GROUND_SLAM = 0` for the Dominator.

- [ ] **Step 1: SQL.** One block per item, each preceded by a comment:
  - `creature_template`: `faction = 14` and `ScriptName` for 105393 `boss_ilgynoth`, 105906 `npc_eye_of_ilgynoth`, 105304 `npc_dominator_tentacle`, 105322 `npc_deathglare_tentacle`, 105383 `npc_corruptor_tentacle`, 105591 `npc_nightmare_horror` and 105721 `npc_nightmare_ichor`.
  - On the Eye, `flags_extra` with this fork's no-threat and no-knockback bits; check `CreatureFlagsExtra` in `CreatureData.h`.
  - `creature`: the heart and the Eye on map 1520, from the readings, with `spawnDifficulties '14'`, `spawntimesecs 604800` and `wander_distance 0`. Use free guids after 1520101, checked with `SELECT MAX(guid)`.
  - `gameobject`: 251555 at the hollow mouth, `state` 1 (closed), the same difficulty, a free guid.
  - `creature_text`: the rows from Global Constraints. The `Text` column carries the English broadcast text, with `TextRange 0`.
- [ ] **Step 2: Header and instance.**
  - Add the ids above.
  - In the instance, add `{ NPC_EYE_OF_ILGYNOTH, DATA_EYE_OF_ILGYNOTH }` to `creatureData`.
  - Add a `gameObjectData[] = { { GO_EYE_OF_ILGYNOTH_FORCEFIELD, DATA_ILGYNOTH_FORCEFIELD }, { 0, 0 } }`, passed as the second argument of `LoadObjectData`.
  - Add `{ DATA_ILGYNOTH, new CircleBoundary(...) }` (or the fitted shape) to `boundaries`, with a comment saying how it was fitted.
  - Add `OnGameObjectCreate`: call the base, then if the go is the forcefield and `GetBossState(DATA_ILGYNOTH) == DONE`, open it with `HandleGameObject(ObjectGuid::Empty, true, go)`.
- [ ] **Step 3: Heart skeleton.** Model it on `boss_nythendra.cpp:107-240`.
  - `JustAppeared`: root it, set `UNIT_FLAG_UNINTERACTIBLE`, `SetForcefield(false)`.
  - `DoAction(ACTION_PULL)`: if not engaged, `DoZoneInCombat()`.
  - `JustEngagedWith`: `BossAI::JustEngagedWith`, frame-engage the heart, `Talk(TALK_AGGRO)`, `StartOutsidePhase(false)`.
  - `EnterEvadeMode`: disengage the frames of the heart and the Eye, `summons.DespawnAll()` (which takes the adds' pools with them), `me->RemoveAllAreaTriggers()` for the heart's own, `SetForcefield(false)`, respawn the Eye if it is dead, `_EnterEvadeMode()`, `_DespawnAtEvade()`.
  - `JustDied`: disengage, `summons.DespawnAll()`, `SetForcefield(true)`, `Talk(TALK_DEATH)`, `_JustDied()`.
  - `UpdateAI` calls `UpdateVictim()` only in the heart phase. In the outside phase it only updates events. The heart has no victim then, so it must not evade from having no target: the engage is kept by `DoZoneInCombat` on each outside-phase start, and the plan's test checks it.
- [ ] **Step 4: Eye skeleton.**
  - `JustAppeared`: root it, `DoCastSelf(SPELL_STUFF_OF_NIGHTMARES)`.
  - `JustEngagedWith`: frame-engage the Eye, then send `ACTION_PULL` to `instance->GetCreature(DATA_ILGYNOTH)`.
  - `JustDied`: disengage the frame, send `ACTION_EYE_DIED`.
  - `EnterEvadeMode`: tell the heart to evade (`heart->AI()->EnterEvadeMode()`) if it is engaged.
  - In this task `ACTION_EYE_DIED` only calls the `StartHeartPhase` stub: `SetForcefield(true)` and remove `UNIT_FLAG_UNINTERACTIBLE`.
- [ ] **Step 5: Loader.** Add the declaration and the call.
- [ ] **Step 6: Build and boot.**
  - Run `sh ~/emerald-nightmare/build.sh`. Expected: `BUILD OK`, no warnings in the touched files.
  - Run `sh ~/emerald-nightmare/boot_check.sh`. Expected: `READY`; the new SQL listed as applied; DBErrors growth no more than 13422; no new Server.log ERROR lines; no "script not found".
- [ ] **Step 7: In-game checks** (`.gm on`, `.go instance emerald_nightmare`, walk to the room):
  - (a) The heart and the Eye are at the readings. `.npc info` shows faction 14 and the script name. The heart cannot be selected. The forcefield stands at the mouth, and walking into it is blocked (or note that it is not; see the spec's Risks).
  - (b) `.gm off` and walk into the Eye's aggro range: the heart and the Eye frames show.
  - (c) `.die`: the encounter resets, and the Eye has full health and Stuff of Nightmares.
  - (d) Pull, then `.die` the Eye from GM (`.kill` or `.damage` past the aura; record which works here). The forcefield opens and the heart becomes selectable.
  - (e) `.damage` the heart to death: `.instance getbossstate 1` is 3 (Done). Relog: the forcefield is open. Then `.instance unbind all`.
- [ ] **Step 8: Commit and push.** `git commit -m "feat(scripts,sql): Il'gynoth skeleton, forcefield, boundary and spawns" -- <the files>`, then `git push origin dev`.

### Task 3: Outside phase: waves, tentacles, Horror, Nightmare Corruption pools

**Files:**
- Modify: `boss_ilgynoth.cpp`
- Create: `sql/updates/world/master/<date>_NN_world.sql` (`spell_script_names`, area trigger 6514 rows)

**Interfaces:**
- Consumes: Task 1 (a), (d); Task 2's `StartOutsidePhase` and constants.
- Produces:
  - `struct WaveList { std::vector<Milliseconds> times; std::vector<uint8> counts; };`, with four `static` tables per phase kind (first, after heart) taken from the Global Constraints.
  - Heart events `EVENT_WAVE_DEATHGLARE`, `EVENT_WAVE_CORRUPTOR`, `EVENT_WAVE_HORROR` and `EVENT_RANDOM_TENTACLE`, in `PHASE_OUTSIDE = 1`.
  - `void SummonTentacles(uint32 entry, uint8 count)`, which places each tentacle at a random point on the tentacle ring.
  - `Creature* SummonHorror()`.
  - Add AIs `npc_dominator_tentacle`, `npc_deathglare_tentacle`, `npc_corruptor_tentacle` and `npc_nightmare_horror`. Each has a `static constexpr uint8 ICHORS` (2, 2, 1, 4) and casts `SPELL_SUMMON_ICHOR` that many times in `JustDied`.
  - `class spell_ilgynoth_spew_corruption : AuraScript` (periodic dummy, which casts 208931 at the owner).
  - `struct at_ilgynoth_nightmare_corruption : AreaTriggerAI` (enter: apply 212886; exit: remove it unless the unit is in another pool, as `at_nythendra_infested_ground` does).

- [ ] **Step 1: SQL.**
  - `spell_script_names` row (208929, `spell_ilgynoth_spew_corruption`).
  - `areatrigger_template` and `areatrigger_create_properties` for 6514, with the shape from Task 1 (a), the `DecalPropertiesId` and `ScriptName` `at_ilgynoth_nightmare_corruption`. Copy the column list from the Nythendra pool rows in `2026_10_17_02`/`_05`.
- [ ] **Step 2: Wave schedule.**
  - `StartOutsidePhase(afterHeart)`:
    - pick the table set;
    - reset the cursors;
    - spawn 2 Dominators at the two measured spots, each frame-engaged;
    - schedule each wave event at its first time.
  - Each wave event spawns the table's count, then reschedules itself at `next time - this time` while entries remain.
  - When both tentacle lists are used up, schedule `EVENT_RANDOM_TENTACLE` every 60 s; it picks Deathglare or Corruptor at random.
  - The Horror repeats every 280 s.
  - Before the Horror spawns, the Eye says `TALK_EYE_HORROR` and the Horror casts 210289 on itself.
  - All summons are `TEMPSUMMON_MANUAL_DESPAWN` from the heart, so they are in `summons`.
- [ ] **Step 3: Tentacle AIs.**
  - All three are rooted, use `DoZoneInCombat` on summon, and choose targets with `SelectTarget(SelectTargetMethod::Random, 0, 0.0f, true)`. A null target skips the cast and retries in 1 s.
  - **Dominator:** Fury on its victim (6 s, then 10.9 s); Slam at a random player (12 s, then 20.5 s) with `Talk(TALK_GROUND_SLAM, target)`; and if `!me->IsWithinMeleeRange(me->GetVictim())` for every threat target, Rupturing Roar back to back.
  - **Deathglare:** Mind Flay at a random player, recast when the cast ends or is interrupted (an `OnSpellCast` / `OnSpellFailed` hook or a 1 s retry).
  - **Corruptor:** Spew Corruption back to back (calibrate) at the target type Task 1 (d) reports.
- [ ] **Step 4: Horror.** It moves and melees. Eye of Fate (14 s, then 10 s) on its victim. `JustAppeared` casts Seeping Corruption on itself.
- [ ] **Step 5: Spew aura script and pool AT.** As in the Interfaces block.
  - The Spew tick casts 208931 from the aura's caster (the Corruptor) at the owner's position, not from the player. That way every pool belongs to an add, and `summons.DespawnAll()` on evade removes it.
  - Check this in Step 6 (f). If a pool outlives its caster's despawn, the heart also removes area triggers of spell 208931 in a 100 yd search on evade.
- [ ] **Step 6: Build, boot, check** with a solo GM and the DBM bars:
  - (a) Deathglare at 21.5 s, Horror at 60 s with the emote, Corruptors at 79 s, each within 2 s of DBM's bar.
  - (b) Mind Flay is interruptible and gets recast.
  - (c) Spew leaves a pool every 2 s under Gairy. Standing in a pool deals damage and pacifies/silences; stepping out ends it.
  - (d) The Horror's Eye of Fate stacks, and it drops pools.
  - (e) Leaving a Dominator's melee range brings Rupturing Roar.
  - (f) `.die`: every add and pool is gone. The next pull starts again at 2 Dominators with the first-phase times.
- [ ] **Step 7: Commit and push.** `feat(scripts,sql): Il'gynoth outside phase adds and waves`.

### Task 4: Nightmare Ichor and the Eye

**Files:**
- Modify: `boss_ilgynoth.cpp`

**Interfaces:**
- Consumes: Task 3's `ICHORS` deaths; Task 2's `ACTION_EYE_DIED` and `ACTION_ICHOR_INSIDE`; `IsInsideChamber`.
- Produces:
  - `struct npc_nightmare_ichor : ScriptedAI`.
  - On the Eye: Nightmare Gaze on a 2-3 s timer.
  - `void npc_eye_of_ilgynoth::TakeExplosion()`: lower health by `CountPctFromMaxHealth(5)`. At 0 or below it calls `Unit::Kill(ichor, me)` exactly once; a `_dying` flag guards it.
  - The ownership check: Ichors summoned by an add are owned by the heart's `summons`. If not, the add summons them with `heart->SummonCreature` at its own position instead of casting 209966. Record which, and keep casting 209966 if it is owned.

- [ ] **Step 1: Ichor AI.**
  - `JustAppeared`: cast Touch of Corruption proc 209467 on itself; fixate (210099) on a random player in the boundary; chase it.
  - Each update: if the fixate target is dead or outside the boundary, pick a new one. With none, stop.
  - `DamageTaken`: if `damage >= me->GetHealth()`, set `damage = me->GetHealth() - 1`. On the first time, stop moving and cast 209471.
  - At the cast's end (`OnSpellCast` on 209471): if `me->IsWithinDist(eye, 13.0f)` (the Task 1 (e) radius), call `TakeExplosion()` on the Eye's AI. Then `me->KillSelf()`.
  - In the heart phase, if `IsInsideChamber(me)`, send `ACTION_ICHOR_INSIDE` to the heart and despawn.
- [ ] **Step 2: Eye.** Add Gaze, `TakeExplosion` and the `_dying` guard. On respawn, reset `_dying` and cast Stuff of Nightmares again.
- [ ] **Step 3: Heart `ACTION_ICHOR_INSIDE`.** `DoCastSelf(SPELL_REABSORPTION)`; the heart is the caster, so the raid damage comes from it.
- [ ] **Step 4: Build, boot, check:**
  - (a) Killing each tentacle type and the Horror gives 2, 2, 1 and 4 Ichors, each fixated on Gairy.
  - (b) An Ichor brought to 1 HP casts the explosion (DBM warns) and dies.
  - (c) Near the Eye the Eye loses 5%; more than 13 yd away it loses nothing.
  - (d) Twenty in range kill the Eye. The forcefield opens. `ACTION_EYE_DIED` fires once: add a `TC_LOG_DEBUG` line in `StartHeartPhase` and see it once in Server.log.
  - (e) Two Ichors exploding together at 5% Eye health: the Eye dies once and nothing crashes.
  - (f) The fixate target `.die`s: the Ichor stops and despawns on evade.
- [ ] **Step 5: Commit and push.** `feat(scripts): Il'gynoth Nightmare Ichor and the Eye`.

### Task 5: Heart phase, reconstitution, Final Torpor

**Files:**
- Modify: `boss_ilgynoth.cpp`
- Create: `sql/updates/world/master/<date>_NN_world.sql` (`spell_script_names`; 7424 rows only if Task 1 (c) says to build Bloodburst)

**Interfaces:**
- Consumes: Task 1 (b), (c), (e); `IsInsideChamber`; Task 3's `StartOutsidePhase(true)`; Task 4's Eye respawn reset.
- Produces:
  - Heart events `EVENT_CURSED_BLOOD`, `EVENT_RECONSTITUTION_WARN` and `EVENT_TORPOR_WARN`, in `PHASE_HEART = 2`.
  - `uint8 _heartPhases`.
  - `class spell_ilgynoth_cursed_blood : SpellScript` (target filter to the count rule, inside the chamber only) and `class spell_ilgynoth_cursed_blood_aura : AuraScript` (`AfterEffectRemove`, expired removals only, which casts 215143 from the target).
  - `class spell_ilgynoth_dark_reconstitution : SpellScript`: a target filter keeping only units for which `IsInsideChamber` is true. If Task 1 (b) shows the filter cannot apply, the heart kills inside players itself in `OnSpellCast` and the spell's targets are cleared.

- [ ] **Step 1: `StartHeartPhase`.**
  - Cancel the outside events.
  - `++_heartPhases`.
  - `SetForcefield(true)`, drop `UNIT_FLAG_UNINTERACTIBLE`, `Talk(TALK_HEART_PHASE)`.
  - Cast Dark Reconstitution if `_heartPhases == 1`, otherwise Final Torpor.
  - Schedule the warn emote 5 s before the cast ends: 45 s or 85 s.
  - Start Cursed Blood every 15 s.
  - The heart stays rooted; it is a heart.
- [ ] **Step 2: End of Dark Reconstitution** (`OnSpellCast` for 210781):
  - `Talk(TALK_RECONSTITUTED)`, `SetForcefield(false)`, set `UNIT_FLAG_UNINTERACTIBLE`.
  - Respawn the Eye at full health.
  - `StartOutsidePhase(true)`. Live adds are not despawned.
  - Final Torpor's end needs nothing beyond its own instakill; the wipe evades the heart.
- [ ] **Step 3: Low health.** `DamageTaken`: at 30% or below, `Talk(TALK_LOW_HEALTH)` once per fight; the flag resets on evade.
- [ ] **Step 4: Death during a cast.** `JustDied` does all of this:
  - interrupts the cast (`me->InterruptNonMeleeSpells(false)`, before `_JustDied`);
  - despawns every summon;
  - leaves the Eye dead;
  - keeps the forcefield open.
- [ ] **Step 5: Violent Bloodburst**, per Task 1 (c). Either cast 215971 as the data says and add the 7424 rows, or write "not built: <reason>" in the spec's "Sources and what is uncertain".
- [ ] **Step 6: Build, boot, check:**
  - (a) When the Eye dies, the yell, the 50 s Dark Reconstitution bar (DBM) and Cursed Blood every 15 s on Gairy appear, and the burst hits at the end.
  - (b) Leave the chamber before the end: you live. The forcefield closes, the Eye is back at full health with Stuff of Nightmares, the heart has the same health, adds alive are still there, and the DBM bars show Deathglare 21.5 s, Corruptor 45 s and Horror 95 s.
  - (c) Stay inside: you die.
  - (d) Walk a fixated Ichor into the chamber: Reabsorption.
  - (e) Second heart phase: a 90 s Final Torpor bar and its emote. At the end everyone dies and the encounter resets.
  - (f) `.die` in the heart phase: the full reset from the Review Focus.
  - (g) `.damage` the heart to death mid-cast with adds outside: Done, the adds gone, nobody killed afterwards, the forcefield open after a relog, the Eye not respawned. Then `.instance unbind all`.
- [ ] **Step 7: Commit and push.** `feat(scripts,sql): Il'gynoth heart phase, reconstitution and Final Torpor`.

### Task 6: Calibration and wrap-up

**Files:**
- Modify: `boss_ilgynoth.cpp` (constants only)
- Modify: the spec's "Sources and what is uncertain"

- [ ] **Step 1:** Run a full fight with DBM 7.3.31 and note each event's real time against the Global Constraints. Change any constant that is off by more than 2 s from DBM. For what DBM does not time (wave counts, Spew rate, Gaze rate, Cursed Blood count, explosion range, the 30% yell), compare against a retail Normal video the user supplies.
- [ ] **Step 2:** After any constant change, run the regression: Task 5 Step 6 (f) and (g).
- [ ] **Step 3:** Mark settled items in the spec. Copy the spec and this plan to `C:\Users\chris\code\acore-webservice\docs\superpowers\` (specs/ and plans/). Commit `docs: Il'gynoth calibration notes` and push. Update the user's memory note `project_emerald_nightmare.md` with the commits and lessons.
