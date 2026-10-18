# Emerald Nightmare: Nythendra, the first boss

Date: 2026-10-17
Branch: `dev` (content goes on `dev` directly while one session works on
it, rule 3 of `docs/BRANCHING.md`)
Status: design approved by the user 2026-10-17 (no doors in v1, boundary
only; the user measures her position and room edge in game). Written spec
awaiting review.

## Goal

Nythendra (creature 102672, encounter 1853, `DATA_NYTHENDRA`) stands in her
room in the Emerald Nightmare and can be fought and killed on Normal
difficulty with her retail Normal mechanics: the energy-driven two-phase
fight with Infested Breath, Rot, Volatile Rot, Infested Ground and Heart of
the Swarm. Killing her sets the boss state to Done and records the raid
lock. Solo testing is done with GM commands (user decision, 2026-10-17).

## Decisions

- Retail-faithful mechanics, Normal difficulty only. Heroic (Infested
  stacks), Mythic (Infested Mind) and Raid Finder are later pieces
  (user, 2026-10-17).
- One spec per boss, content straight on `dev` (user, 2026-10-17).
- No doors in v1 (user, 2026-10-17). Her room is guarded by a boss
  boundary only. The 16 `NightmareBoss1DungeonDoor` templates
  (254984-254999, display 32552, invisible) are not spawned: the client
  data has no rows for map 1520 and Wowhead shows no pins for them, so
  their positions are unknown. If retail turns out to have an
  encounter-closed barrier, a later patch adds `DoorData` for it.
- Her spawn position and the boundary are measured in game by the user
  (user, 2026-10-17); see "Measured inputs".
- No leak-derived source (LegionCore and similar) is used. Mechanics come
  from the Wowhead and Icy Veins guides and spell tooltips of the Legion
  era, and from the client's own DB2 data.

## Sources and what is uncertain

Wowhead NPC page 102672 and spell tooltips (nether.wowhead.com), Icy Veins
"Nythendra Abilities" (23 Sep 2016) and "Strategy/Tactics for
Normal/Heroic" (4 Oct 2016). Research notes are in the session notes folder
(`nythendra-research.md`). All numbers in the tooltips are at level 110 and
come from the client's spell data, so the script does not hard-code damage.

Uncertain points, all marked "calibrate" below and tuned against a retail
video the same way the Broken Shore work was (no behaviour is invented
beyond these):

1. **Breath count.** Energy falls by 50 twice before Phase 2. Icy Veins
   says the breath is cast "once per Phase One"; the Wowhead zh-TW guide
   says twice at 50 energy each. The design is energy-driven with two
   breaths, at about 45 s and 90 s ("after about 45 seconds", and a phase
   of "about 90 seconds").
2. **Volatile Rot timing.** Three casts per Phase 1 on the current tank;
   the guides give no times. Design: 20 s, 50 s, 80 s.
3. **Rot ramp.** "More and more players are targeted" as the phase goes
   on. Design: 1 target before the first breath, 2 after it.
4. **Corrupted Vermin entry.** Three templates exist (102998, 103525,
   111005, all faction 35, critters). The task names 111005; it is used.
   A second creature template named Nythendra (103160) exists and is left
   alone.

### Calibration (Task 5, 2026-10-17)

Timers are now taken from Deadly Boss Mods 7.3.31 (interface 70300, the
7.3.5 client), `DBM-EmeraldNightmare/Nythendra.lua`: retail combat-log
timers, open source. The script is fitted to the addon, and the user checked
a full fight in game with the unmodified addon installed.

| Item | Was | Now (DBM) | Status |
|---|---|---|---|
| 1. Breath count and times | 45 s, 90 s | 2 per phase: 37 s on the pull, 43 s after a swarm; 39 s start to start | **Settled.** First breath 2-3 s off the bar, second on it. |
| 2. Volatile Rot | 20 / 50 / 80 s | 20 s on the pull, 29 s after a swarm, then every 22 s, 3 per phase | **Settled.** |
| Rot timer | 8 s, every 16 s | 5 s on the pull, 12 s after a swarm, every 15 s | **Settled.** |
| Heart of the Swarm start | about 98 s | 10 s after the second breath's cast ends (about 88 s; DBM 86-91 s) | **Settled.** 2-3 s early on the second cycle; DBM marks its 98 s repeat as uncertain. |
| Heart of the Swarm length | 3.7 s + 20 s | DBM 23 s from cast start | **Settled**, unchanged. |
| Enrage | none on Normal | Berserk 26662 at 10 min (Normal, Raid Finder), 8 min above | **Settled.** DBM's `berserkTimer`; the earlier "none on Normal" was wrong. |
| Stand-up after the swarm | moved at once | rooted 6 s after the channel (retail video: about 3 s of animation, too short in game) | **Settled.** |
| Breath facing | turned after the target | faces the target at cast start, `UNIT_FLAG2_CANNOT_TURN` until the channel ends | **Settled.** |
| Reset boundary | 55 yd | 90 yd: 73 yd so a pool dropped at the room edge does not evade her, then 90 yd to reach where the entrance gate would stand, (1842.2, 1387.2), 85.8 yd from the centre | **Settled.** |

Also added: the retail cast markers DBM keys on, 203095 (Rot, once per
wave) and 202968 (Infested Breath, at its start), both instant dummies cast
before the real spell.

3. **Rot ramp** (1 target, 2 after the first breath), the vermin count
   (40), the first swell (10 s) and swell rate (1/s), the pool pull-in
   speed (12 yd/s) and the pool radius (5 yd) are not timed by DBM and are
   **still open**. The Heart of the Swarm pose (lying flat) comes from the
   spell's client visual, not the script; the retail Normal tank-POV video
   (youtube.com/watch?v=dAj2s59iwss, about 1:55-2:08) shows her lying flat,
   and the user is to compare it with the in-game pose.

## What exists today

Checked 2026-10-17 on `dev` (`bce66df3`) and the server's databases.

- Scripts: `emerald_nightmare.h` and `instance_emerald_nightmare.cpp` with
  `DATA_NYTHENDRA = 0`, `BOSS_NYTHENDRA = 102672`, the creature object data
  and encounter 1853; no `DoorData`, no `BossBoundaryData`; no
  `AddSC_boss_*`.
- `creature_template` 102672: faction 35, `ScriptName` empty, display
  68077 (bounding radius 18.75, combat reach 18.75), speed run 1.14286,
  Classification 1, `unit_class` 1. `creature_template_difficulty` has
  difficulty 0 only: level 110, HealthModifier 540.225, DamageModifier 1,
  TypeFlags 2097260, TypeFlags2 69761.
- `creature_template` 111005 (Corrupted Vermin): faction 35, four displays
  (43156, 45887, 45564, 45886), HealthModifier 2, no script.
- `hotfixes.broadcast_text` 105376 (Rot), 105969 (Volatile Rot), 106119
  (Infested Breath). No `creature_text` for 102672.
- No creature, gameobject or areatrigger spawns on map 1520 except the
  scaffold's exit trigger. No `spell_script_names` for the spells below.
- Spell data lives in the client's DB2 files (`hotfixes.spell_effect` is
  empty on this server). `/tmp/stage1/wdc1.py` on the host reads them.

## Retail mechanics to implement (Normal)

### Energy and phases

Nythendra has an energy bar (`POWER_ENERGY`, max 100) that only marks the
phase change. It starts at 100, does not regenerate by itself and is set by
the script.

**Phase 1, "The Plagued Dreamer"**, about 90 s:

| Ability | Spell | Rule |
|---|---|---|
| Infested Breath | 202977 | 3 s cast, a cone at a random raider, 5 s of Plague damage ticking each second, leaves pockets of Infested Ground. Costs 50 energy. First at about 45 s, second at about 90 s. Raid emote with broadcast 106119. |
| Rot | 203096 | Every 16 s on random raiders (1 target, 2 after the first breath). Plague damage to allies within 8 yd on application and every 3 s for 9 s. On expiry about 3 Infested Ground pools form where the target stands. Raid boss whisper to the target, broadcast 105376. |
| Volatile Rot | 204463 | Three casts per Phase 1 on the current tank, at about 20, 50 and 80 s. After 8 s it deals raid-wide Plague damage with falloff by distance, then forms several Infested Ground pools. Whisper to the target, broadcast 105969. |
| Infested Ground | 203045 | An area trigger, Plague damage every 1.5 s to anyone inside. Lasts until Phase 2. |

Melee: she auto-attacks her tank. Tail Lash was removed from the dungeon
journal and is not built.

**Phase 2, "Heart of the Swarm"**, starts when energy reaches 0:

- She casts Heart of the Swarm (203552): 3.7 s cast, then a channel of about
  20 s. She stops attacking and stays in place.
- The existing Infested Ground pools are pulled to her quickly. Each one
  keeps damaging whoever it touches on the way, and the pulled pools
  recharge her energy: it rises from 0 to 100 over the channel.
- A large number of Corrupted Vermin (111005) are summoned at random points
  in the room at the start of the channel. They are immobile and cannot be
  targeted or attacked.
- From about 10 s into the channel, one random vermin per second swells and
  casts Burst of Corruption (203646) three times over 3 s, then dies. Burst
  hits players within 8 yd and stacks a 25% damage-taken increase for 5 s.
- When the channel ends the energy is 100, the vermin are gone and Phase 1
  begins again with its timers restarted.

**Enrage:** Berserk (26662) at 10 minutes on Normal, 8 minutes on Heroic
and above (DBM; see Calibration).

Calibrate: breath times, Volatile Rot times, the Rot ramp, the number of
vermin, the pull-in speed, and the first Rot time after a phase starts
(initial value 8 s).

## Design

### Files

| File | Change |
|---|---|
| `src/server/scripts/BrokenIsles/EmeraldNightmare/boss_nythendra.cpp` | New: the boss, its spell and aura scripts, the Infested Ground area trigger AI, `AddSC_boss_nythendra`. |
| `.../emerald_nightmare.h` | Add `NPC_CORRUPTED_VERMIN = 111005` to `ENCreatureIds`. |
| `.../instance_emerald_nightmare.cpp` | Add `BossBoundaryData` for `DATA_NYTHENDRA` and `LoadBossBoundaries`. |
| `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp` | Declare and call `AddSC_boss_nythendra()`. |
| `sql/updates/world/master/2026_10_17_NN_world.sql` | Template fixes, creature text, spawn, spell script names, area trigger rows. Next free `NN` on the date. |

The script directory is globbed by CMake as `TrialOfValor` is, so no
CMake change is expected; the plan confirms this.

### Boss script (`boss_nythendra`)

A `BossAI` subclass registered with `RegisterEmeraldNightmareCreatureAI`,
following `boss_guarm`:

- `JustAppeared`: set max energy to 100 and the current energy to 100
  (`Creature::UpdateMaxPower` zeroes it otherwise, as Guarm's comment says).
- `JustEngagedWith`: `BossAI::JustEngagedWith`, send
  `ENCOUNTER_FRAME_ENGAGE`, start Phase 1 timers.
- Event ids: `EVENT_INFESTED_BREATH`, `EVENT_ROT`, `EVENT_VOLATILE_ROT`,
  `EVENT_HEART_OF_THE_SWARM`, `EVENT_SWELL_VERMIN`, `EVENT_END_CHANNEL`.
  Two event phases (`PHASE_ONE`, `PHASE_TWO`) so Phase 1 events are
  cancelled on the change.
- `UpdateAI`: `UpdateVictim` in Phase 1 only; in Phase 2 she is held in
  place and does not melee.
- Breath cost: after the breath cast completes, set energy to
  `GetPower - 50`. When it reaches 0 the next update starts Phase 2.
- Phase 2: root her, cast 203552, schedule the vermin summon at cast end,
  the first swell at 10 s, then one swell per second, and the end of the
  channel at 20 s. Energy is raised in steps each second so the client's
  bar fills. At the end, remove the root, set energy 100, clear the
  vermin and pools, reschedule Phase 1.
- `EnterEvadeMode`: send `ENCOUNTER_FRAME_DISENGAGE`, `summons.DespawnAll`,
  despawn the Infested Ground triggers, `_EnterEvadeMode`, `_DespawnAtEvade`.
- `JustDied`: disengage, despawn leftover vermin and triggers, `_JustDied`.
  Encounter credit comes from the existing `DungeonEncounterData`, so no
  `flags_extra` boss flag is needed (Guarm has none either).

### Spell and aura scripts

Registered through `spell_script_names`, using the pattern in
`boss_guarm.cpp`:

- A target selector on Rot so the number of targets is a script value
  (1, then 2), choosing raid members and not the tank.
- Rot's expiry hook (`AfterRemove` on the aura, expired removals only)
  creates three Infested Ground pools around the target.
- Volatile Rot: the detonation damage with distance falloff is read from
  the spell data; the script only adds the pool creation on expiry. If the
  spell data does not scale by distance on its own, the script scales it.
- Infested Breath: the cone is aimed at a random raider by the boss before
  the cast; the pockets of Infested Ground are created along the cone.
- Heart of the Swarm and Burst of Corruption need only the cast and the
  summon logic above; the stacking 25% effect is checked against the DB2
  data and, if it is not in the spell, added as a small aura script.

The first step of the plan dumps `SpellEffect`, `SpellMisc`,
`SpellDuration` and `AreaTrigger` data for 202977, 202978, 203045, 203096,
203097, 203552, 203646 and 204463 with `wdc1.py`. That shows which spells
already trigger which, and which of the scripts above are needed at all. The
design does not assume any spell data beyond the tooltips.

### Infested Ground

An `AreaTriggerEntityScript` with an `AreaTriggerAI`, as `at_guardians_breath`
in Guarm. It has two jobs:

1. Apply the 203045 aura to units inside, so the damage tick is the spell's.
2. Register itself with the boss on creation (instance data or a
   `GuidUnorderedSet` on the boss) so Phase 2 can find every pool.

Pull-in: at the start of the channel each registered pool is moved to the
boss with `AreaTrigger::InitSplines` over a short time (calibrate, about
4 s), then removed. The plan checks that `InitSplines` exists in this fork;
if it does not, a pool is removed at the start of the channel instead, and
the missing pull-in is recorded as a known gap in the plan, not hidden.
The recharge of energy does not depend on the pools arriving: it is a fixed
0-to-100 over the channel.

### Creature text

`creature_text` for 102672:

| Group | Type | Broadcast | Used for |
|---|---|---|---|
| 0 | 41 (raid boss emote) | 106119 | Infested Breath |
| 1 | 42 (raid boss whisper) | 105376 | Rot, sent to the target |
| 2 | 42 (raid boss whisper) | 105969 | Volatile Rot, sent to the target |

The `Text` column carries the English text of each broadcast so the table is
readable; the broadcast id is what the client shows. Called with
`Talk(group, target)`.

### Instance script

```cpp
BossBoundaryData const boundaries =
{
    { DATA_NYTHENDRA, new CircleBoundary(<centre>, <radius>) }
};
```

with `#include "AreaBoundary.h"` and `LoadBossBoundaries(boundaries)` in the
constructor. The centre and radius are the measured values below. No
`DoorData` and no `LoadDoorData`.

### SQL

One file `sql/updates/world/master/2026_10_17_NN_world.sql`, re-runnable
(`DELETE` then `INSERT`/`UPDATE` with a comment per block):

1. `creature_template` 102672: `faction` 14, `ScriptName`
   `boss_nythendra`. Nothing else on the template changes in v1: the
   unsniffed `speed_*`, `BaseAttackTime` 0 and the difficulty row are left,
   and any value that proves wrong in the test is a data-only fix recorded
   in the plan. (`BaseAttackTime` 0 is checked first: a boss with no melee
   timer may never swing. If so it is set to 2000 ms.)
2. `creature_template` 111005: `faction` 14, `unit_flags` for non-
   attackable and not selectable (`UNIT_FLAG_NON_ATTACKABLE |
   UNIT_FLAG_UNINTERACTIBLE`), `flags_extra` untargetable, speed 0, no
   script (the boss picks a vermin each second and makes that vermin cast
   Burst of Corruption three times, then despawns it).
3. `creature_text` for 102672, three rows.
4. `spell_script_names` for the scripts above.
5. `areatrigger_template` / `areatrigger_create_properties` rows for the
   Infested Ground trigger, only if the spell data shows the existing rows
   do not cover it (`areatrigger_create_properties` has 187 rows today).
   Ids follow the scaffold's habit of checking for free ids first.
6. `creature` spawn for 102672 on map 1520 at the measured position,
   `spawnDifficulties` `14`, respawn time 7 days (raid boss), no
   wander. The guid is the next free one after a check of the table.

### Measured inputs

The user supplies, from the retail layout, via `.gps` in game:

1. **Spawn.** Fly in with `.gm on` and `.gm fly on`, stand where Nythendra
   stands (Wowhead's pin, zone 8026, 55.3 / 50.1 of the map, says roughly
   where to look) and take `.gps`: x, y, z, orientation and map id.
2. **Room edge.** Take `.gps` at three or four points on the edge of the
   arena, spread round it, at ground height. The boundary is the circle
   through them; if the room is clearly not round, the plan uses a
   `RectangleBoundary` or `ZRangeBoundary` plus circle instead.
3. **Entrance check.** Where players enter the room from the entrance, so
   the boundary does not cut the path. Taking `.gps` at the entrance
   point is enough.

The plan's first code step leaves these three as named constants at the top
of `instance_emerald_nightmare.cpp` and in the SQL, filled in when the user
sends the readings; nothing else depends on them.

## Testing

Build in `~/arguscore` (rule 9, 10 of `docs/BRANCHING.md`): check the tree is
clean on `dev`, `pgrep -x docker-buildx` shows nothing, ask the "Priest class
hall engine work" session before building. Use `~/emerald-nightmare/build.sh`
and `boot_check.sh`; never `--no-cache`; no restart while a client is
connected (rule 13).

1. **Boot.** The server starts; `DBErrors.log` grows by no more than the
   baseline (13,422); no "script not found" for `boss_nythendra` or the
   spell scripts.
2. **Template.** `.gm on`, `.go instance emerald_nightmare`, walk to the
   room. `.npc info` on her shows faction 14, the script name and level
   110. She is visible at the measured spot and does not attack a player
   before the pull (aggro radius).
3. **Pull.** Hitting her engages: the encounter frame and her energy bar
   (100) show.
4. **Rot.** The first Rot lands on a raider (Gairy, solo) about 8 s after
   the pull and again every 16 s; the whisper text shows; 9 s later about
   three pools form at the spot; walking into a pool takes damage every
   1.5 s.
5. **Volatile Rot.** Gairy, as the tank, is hit three times in Phase 1
   (about 20, 50, 80 s); the detonation after 8 s damages by distance; pools
   form.
6. **Infested Breath.** The emote shows, the breath is a 3 s cast and a
   5 s cone, energy drops to 50 at about 45 s and to 0 at about 90 s.
7. **Phase 2.** At 0 energy she stops, casts Heart of the Swarm and holds
   still for about 20 s; the pools move to her; vermin are on the ground
   and cannot be targeted; from about 10 s one swells per second and
   Burst of Corruption hits three times then the vermin die.
8. **Loop.** After the channel her energy is 100, no vermin or pools are
   left, and Phase 1 restarts with its timers.
9. **Evade.** Leaving the boundary or dying (`.gm off`, then `.die`)
   resets her to full energy with no pools or vermin.
10. **Kill.** `.damage` her to death (GM): `.instance getbossstate 0` reports
    Done; leaving and re-entering shows the raid lock; Gairy is returned to
    her earlier state with `.instance unbind all` afterwards.

Timings are watched with a stopwatch or the combat log; if a real number
differs from the design by more than a few seconds, it is a calibrate item
and not a bug.

## Out of scope

- Heroic Infested stacks, Mythic Infested Mind, Raid Finder differences, the
  8-minute enrage on Heroic and above.
- Loot tables, trash, other bosses, graveyards.
- Doors (see Decisions).
- Solo-friendly tuning: GM commands only.

## Risks

- **Timers are estimates.** Calibrate against a retail video (section
  "Sources and what is uncertain").
- **Area trigger pull-in.** Depends on `InitSplines`; see "Infested Ground".
- **Spell data.** Some spells may already be driven by their own DB2
  triggers; the plan's first step dumps them before any script is written, so
  nothing is scripted that the data already does.
- **Boundary shape.** A circle may not fit the room; see "Measured inputs".
- **Template values.** Unsniffed rows can have more wrong values than faction
  (melee timer, speed, flags). They are found in the test and fixed in data.

## Amendment after the spell-data dump (Task 1, 2026-10-17)

The client's spell data already does more than the design assumed. Heart of
the Swarm (203552) carries its own periodic energize (+5 energy per second
for its 20 s), Burst of Corruption (203646) carries the 25% stacking effect,
Rot ticks through 203097, and the Volatile Rot detonation is 204470. So the
script does not ramp energy or add a Burst stacking aura script. Still
scripted: the -50 energy per breath, the Infested Ground pools on Rot expiry
and on Volatile Rot detonation. Infested Ground itself is spell 203044
(create-properties id 5887), which has no row in the world database yet; the
SQL adds it. Details: `nythendra-task1-spell-findings.md` in the session
notes folder.
