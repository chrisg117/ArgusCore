# Emerald Nightmare: Elerethe Renferal

Date: 2026-10-19
Branch: `dev` (Emerald Nightmare content goes on `dev` directly, rule 3 of
`docs/BRANCHING.md`)
Status: design approved by the user 2026-10-19, section by section. Written
spec awaiting review.

## Goal

Elerethe Renferal (creature 106087, encounter 1876, `DATA_ELERETHE_RENFERAL`)
can be fought and killed on Normal difficulty in the Emerald Nightmare with her
retail Normal mechanics. She alternates between a Spider form and a Roc form.
Each Roc form, she flies from the platform she is on to the next of three, and
the raid follows over the web bridges through a Dark Storm. Killing her sets
the boss state to Done and records the raid lock. Solo testing uses GM
commands.

## Decisions

All made by the user on 2026-10-19.

- **Scope.** Retail-faithful mechanics, Normal only. Heroic's Venomous Pool
  dispersal by tornadoes, Mythic's Tangled Webs, Violent Winds and Nightmare
  Spawn, and Raid Finder are later pieces. One spec per boss, content on `dev`.
- **Platform movement: the full cycle.** That covers three platforms, her
  flight to the next one each Roc form, and Dark Storm on the platform she
  leaves, with an Eye of the Storm safe zone on the new one. It also covers
  Web Crawlers spiderlings on the bridges, egg sacs with Skittering
  Spiderlings, and Shimmering Feathers with the jump boost and the spiderling
  crush.
- **Form changes.** She has a visible energy bar (`unit_class` 4). The script
  fills it at a fixed rate so that she changes form at 100 on DBM's times.
  The bar does not use power regeneration.
- **Berserk at 540 s on Normal too** (DBM's value). Platform 3 is the last
  stop: a third Roc form never comes, so she flies only twice.
- **Counts no source gives** are starting guesses and calibrate items:
  - Venomous Spiderlings per Feeding Time;
  - the spiderlings and tornado left on a bridge after a flight;
  - Skittering Spiderlings per egg sac;
  - the Feathers per flight.
- **Yells.** Only lines the client has are used. Plan Task 1 looks up her
  lines in the client's `BroadcastText.db2`. Lines that are not there are a
  known gap; no custom text is added.
- **Architecture.** Approach A: one `BossAI` on 106087 runs the whole fight.
  The Roc is the client's own transform aura, so she stays one unit with one
  health pool. Rejected: the instance script as controller, and swapping in
  creature 105604 for the Roc form (it breaks DBM, which watches 106087, and
  the encounter frame).
- **Positions.** Every position comes from the user's `.gps` readings in
  game. They are a gate in the plan, not inputs to this spec (see "Measured
  inputs").
- **Doors.** None. The client data has no `NightmareBoss3*` door object.
- **No leak-derived sources.** Mechanics come from DBM, the Legion-era guides
  and the client's DB2 data.

## Sources and what is uncertain

- DBM 7.3.31 `DBM-EmeraldNightmare/Renferal.lua` (mod 1744, creature 106087,
  encounter 1876), installed in `E:\Legion\Interface\AddOns`.
- Icy Veins "Elerethe Renferal Abilities" and "Strategy/Tactics for
  Normal/Heroic" (both 23 Sep 2016).
- warcraft.wiki.gg "Elerethe Renferal (tactics)" (journal text and quotes).
- The client's DB2 files (build 26972), dumped with `/tmp/stage1/spellinfo.py`
  into `~/emerald-nightmare/elerethe_spells.txt` and `elerethe_spells2.txt`.
  That tool prints radii in yards, not as indexes.

Research notes: `elerethe-research.md` in the session notes folder.

Calibrate items. Nothing beyond these is invented:

1. **Feeding Time spiderlings:** 5 per cast.
2. **Bridge drop after a flight:** 2 Venomous Spiderlings and 1 tornado.
3. **Skittering Spiderlings per egg sac:** 4. Icy Veins says 4-5.
4. **Shimmering Feathers per flight:** 3.
5. **Eye of the Storm radius:** 15 yd.
6. **Flight time:** about 8 s.
7. **Feeding Time:** a 15 yd rise, and about 10 s in the web before Vile
   Ambush.
8. **Web of Pain:** every 30 s. DBM has no timer for it.
9. **Necrotic Venom target count:** 1 solo, 2 in a group.
10. **Pool area trigger 7078:** radius 3 (from the spell data, 225519) and
    the decal, picked by eye.
11. **Egg sac pop range:** 3 yd. **Feather crush range:** 3 yd to start;
    the spell's own radius is 9 yd (221994).

## What exists today

Checked 2026-10-19 on `dev` (`929005e9`) and the server's databases.

- **Scripts.**
  - `emerald_nightmare.h` has `DATA_ELERETHE_RENFERAL = 2` and
    `BOSS_ELERETHE_RENFERAL = 106087`.
  - `instance_emerald_nightmare.cpp` has her in the creature `ObjectData`,
    with encounter 1876, and boundaries for Nythendra and Il'gynoth only.
- **Templates.** None of these has a script. Each has `BaseAttackTime` 0 and
  faction 35, and a difficulty-0 row only:

  | Entry | Name | HealthModifier | Display | Role |
  |---|---|---|---|---|
  | 106087 | Elerethe Renferal | 342 | 69474 (spider) | the boss |
  | 105604 | Bird Transform | 260 | 70237 (roc) | display source of the transform aura (226039, 210293) |
  | 106311 | Venomous Spiderling | 4.5 | 67790 | summoned by 210644 |
  | 106350 | Twisting Shadows | 5 | 169 / 11686 | summoned by 210877 |
  | 108542 | Skittering Spiderling | 1 | 67323 | from egg sacs |
  | 108540 / 109855 / 113154 | Surging Egg Sac | 5 | 70606 / 42349 / 70606 | which one is EN is settled in Task 1 |

  Not used: 106539 (her redeemed form, "Archdruid of the Valley"), 111000,
  113604, 113609. Also unused are the other Venomous Spiderling and Twisting
  Shadows entries (107459, 111721, 111933, 113626; 107767, 109557, 111439)
  and 112078 Pulsing Egg Sac.
- **Spawns.** Map 1520 has Nythendra (1520101) and the Il'gynoth heart
  (1520102) only. The gameobjects are 1520001-1520003. The next free creature
  guid is 1520103.
- **Broadcast text** in `hotfixes.broadcast_text`:
  - 90971: "Scream for me!"
  - 111635: the raid emote "%s calls for Feeding Time".
  - 111411: the raid emote "Elerethe begins Gathering Clouds".
  - 112999 and 113000: Malfurion's death lines.

  Her other wiki lines are not in the table.
- **Boundary support.** `AreaBoundary.h` has `BoundaryUnionBoundary`.
  Several `BossBoundaryData` entries for one boss are an intersection, not a
  union, so the union class is what this boss needs.
- **Area triggers.** 7041 (Gathering Clouds), 7078 (pools) and 6833
  (tornado) are in `AreaTrigger.db2` only as unrelated static triggers on
  other continents, the same id overlap as Il'gynoth's. They have no
  world-DB rows, so their shapes are calibrate values.

## Spells (Normal = difficulty 14, from the client data)

| Spell | Id | Data facts | Script needed |
|---|---|---|---|
| Arachnid Form | 210326 → 226055 | 3 s cast; 226055 transforms into 106087's display (DBM key) | cast at energy 100 in Roc form |
| Darkwing Form | 210308 → 226039 | 3 s cast; 226039 transforms into 105604's display (DBM key) | cast at energy 100 in Spider form |
| Necrotic Venom | 215443 → 215452 → 215449 → 215460 | 10 s warning, then 6 s of 195,000/s; 7 yd ally splash 215461 (47,499); pools through 225519 (AT 7078) | target filter: count, ranged first |
| Feeding Time | 212364 | dummy (DBM key, raid emote 111635) | rise, spiderling drop, Vile Ambush |
| Vile Ambush | 214339 → 214346 → 214348 | jump to dest; 2,983,199 Physical, falls off over 300 yd; knockback within 15 yd | target choice |
| Web of Pain | 215288 → 215300 / 215307 | 1.4 s cast; 90 s link; Pain Lash 215306 on crossing the beam | pairing of two players |
| Web Crawlers | 210327 → 210644 | a spiderling every 4 s from a player on the web | applied by the bridge area triggers |
| Dripping Fangs | 210229 → 210228 | stacking DoT on the spiderling's target | passive aura |
| Venomous Pool | 215489 / 213123 / 213124 | 215489: spiderling corpse pool (5 s, stacks); 213124: 87,538/s | Task 1 picks which one the corpse casts |
| Swarm | 214450 → 214456 | +25% damage and haste per spiderling within 10 yd | none (passive aura) |
| Twisting Shadows | 210864 → 210863 → 210877 → 106350 | 1.3 s cast (DBM key), 10 s mark, then a tornado for 2 min; 210850 deals 67,899/s for 4 s | Task 1: who applies 210850, and whether 221877 moves it |
| Gathering Clouds | 212707 | 2.5 s cast, 8 s channel, 1 s ticks of 212709 (385,823), knockback 212708 | cast schedule |
| Dark Storm | 210948, 211122 → 213936, 211137 | 4 s cast; 1 s ticks of 27,536; +5%/s for 45 s | tick only players without Eye of the Storm |
| Eye of the Storm | 211127 | the "safe from the storm" aura | area trigger applies and removes it |
| Raking Talons | 215582, 222037 | 2 s cast, 1,359,999, +100% per stack for 10 s; 0.7 s quick repeat | one cast plus one repeat |
| Razor Wing | 210547 → 210553 / 212853 | 4.5 s cast; 100 yd cone; 203,999 and knockback | cast schedule |
| Shimmering Feather | 212993 (→ 212986), 221993 → 221994 | 25 s: jump height, safe fall, proc; 221994 kills spiderlings within 9 yd | pick-up area trigger; crush per Task 1 |

Plan Task 1 dumps these again with target types, `SpellTargetRestrictions`,
proc data and visuals. Anything the data already does is not scripted.

## The encounter

### Pull

- She waits on platform 1 (P1) in Spider form and is hostile (faction 14).
  Aggro or damage pulls her.
- `DoZoneInCombat`, `ENCOUNTER_FRAME_ENGAGE`, Berserk at 540 s.
- Swarm (214450) is on her from the pull.
- Energy 0. The script sets energy every second:
  - +100/90 per second in the first Spider form;
  - +100/92 in later Spider forms;
  - +100/132 in Roc form.

  Each form ends at 100.

### Spider form

Timers start from the pull, or from Spider Transform 226055.

1. **Necrotic Venom** at 12.2 s, then every 21.8 s, 4 casts.
   - Targets: 1 solo, 2 in a group (calibrate), ranged players first.
   - The data chain does the rest.
2. **Feeding Time** at 15.5 s and again 50 s later.
   - She casts 212364. The raid emote 111635 goes out.
   - She becomes unselectable and rises 15 yd (calibrate).
   - 5 Venomous Spiderlings drop around the players (calibrate).
   - About 10 s later she casts **Vile Ambush** at a random player's spot.
     She marks it, jumps there, deals damage with falloff and knockback,
     becomes selectable again and fights on.
3. **Web of Pain** every 30 s (calibrate) on two random players, with the
   yell 90971. It is skipped when there is one player.

### Roc form

At energy 100 in Spider form, she casts Darkwing Form (210308), then Bird
Transform (226039). Timers from that cast:

1. **Twisting Shadows** at 6.6 s, then +40, +21.5 and +32.5 s (4 casts). The
   tornado forms 10 s after each mark.
2. **Gathering Clouds** at 15.8 s, with the raid emote 111411.
3. **Dark Storm** at 26 s, followed by **the flight** (below). It happens
   only in the first two Roc forms.
4. **Raking Talons** at 52 s, then every 32.5 s (3 casts). Each is one
   215582 and one quick 222037: 2 hits, per Icy Veins' "2 on Normal".
5. **Razor Wing** at 59 s, then every 32.5 s (3 casts).
6. At energy 100 she casts Arachnid Form (210326), then Spider Transform
   (226055), and is back in Spider form.

A cast that falls due during the flight waits until she lands. The timers
keep counting through the flight, since DBM counts from Bird Transform.

### The flight

After Dark Storm's cast, in the first and second Roc forms:

1. **Feathers.** She drops 3 Shimmering Feathers (calibrate) where she
   stands. Each is an area trigger that applies 212993 to the first player
   who touches it, then disappears.
2. **She flies.** She becomes unselectable, sets fly mode, and flies a spline
   through the measured waypoints over the bridge to the next landing spot.
   The trip takes about 8 s (calibrate).
3. **Dark Storm.**
   - A custom aura on her (from 211122) ticks 213936 every 1 s on every
     player inside the boundary who does not have Eye of the Storm (211127).
   - Foul Winds (211137) adds 5% a second.
   - It runs for 45 s (Foul Winds' duration) whoever it hits. A player who
     leaves the Eye inside that time is hit again.
4. **Eye of the Storm** is an area trigger summoned at the new landing spot
   (radius 15, calibrate). Entering it applies 211127; leaving it removes the
   aura.
5. **Bridge drop.** She leaves 2 Venomous Spiderlings and 1 tornado at the
   midpoint of the bridge she crossed (calibrate).
6. **Landing.** She lands, becomes selectable and fights on.

### The bridges

- **Web Crawlers.** A static area trigger along each bridge (a box fitted to
  the readings) applies 210327 while a player is on the web. That spawns a
  Venomous Spiderling every 4 s per player. Leaving the web removes the aura.
- **Egg sacs** are DB spawns: 3-4 per bridge and 2-3 per platform, at
  measured spots.
  - A player within 3 yd pops a sac. It dies and releases 4 Skittering
    Spiderlings (calibrate), which chase that player.
  - Sacs stay dead for the attempt and come back when the encounter resets.
- **The Feather crush.** A player with 212993 who lands from a jump within
  3 yd of a spiderling kills it (221994).
  - Task 1 checks whether the data's own proc (212993 aura 42 → 212986) does
    this. If it does not, an aura script checks on landing.
  - Jump height and safe fall come from the aura's own effects. Whether the
    client honours aura 409 is checked in game.

### Adds

- **Venomous Spiderling** (106311).
  - Tankable melee that chases the nearest player.
  - Dripping Fangs (210229) as a passive aura.
  - When it dies it leaves a Venomous Pool. Task 1 picks 215489 or the
    213123 area trigger.
  - Swarm on Elerethe counts the spiderlings near her by itself.
- **Skittering Spiderling** (108542). Weak, fast melee with no abilities.
- **Egg sac** (entry from Task 1). Cannot be attacked or targeted and does
  not move. It is popped by proximity (above).
- **Twisting Shadows tornado** (106350).
  - It carries its 4 s pull-and-damage aura 210850: from the summon if the
    data applies it, otherwise cast by its AI on spawn.
  - It follows a player slowly if 221877 (fixate) says so; otherwise it stays
    put.
  - It despawns when the aura ends.

### Pools

Area trigger 7078 (Necrotic Venom's 225519 and 213123) gets
`areatrigger_create_properties`:

- a sphere of radius 3;
- it applies 213124 while a player is inside;
- a decal picked by eye (31, Nythendra's green, first).

Gathering Clouds' 7041 is cosmetic and gets a row only if its absence logs
errors. Pools stay through form changes. They despawn on evade and on death.

### End

- **Berserk** at 540 s. Task 1 finds the spell (Nythendra used 26662).
- **Death.**
  - Boss state Done, `ENCOUNTER_FRAME_DISENGAGE`, and the summons and pools
    are despawned.
  - Malfurion's lines 112999 and 113000 are used only if Task 1 finds a
    speaker in the room; otherwise they are a known gap.
- **Evade.**
  - Despawn the summons, pools, Feathers and the Eye, and clear the
    transform aura and energy.
  - She returns to her P1 spawn in Spider form.
  - The egg sacs respawn.

## Design

### Files

| File | Change |
|---|---|
| `src/server/scripts/BrokenIsles/EmeraldNightmare/boss_elerethe_renferal.cpp` | New: `boss_elerethe_renferal`; the Venomous Spiderling, Skittering Spiderling, egg sac and tornado AIs; the spell scripts (Necrotic Venom target filter, Dark Storm tick filter, Web of Pain pairing, the Feather crush if it is needed); the area trigger AIs (Eye of the Storm, Feather, bridge web); `AddSC_boss_elerethe_renferal` |
| `.../emerald_nightmare.h` | Elerethe's add and egg-sac ids |
| `.../instance_emerald_nightmare.cpp` | The `DATA_ELERETHE_RENFERAL` boundary; egg-sac respawn when the encounter goes back to Not Started |
| `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp` | Declare and call `AddSC_boss_elerethe_renferal()` under `// Emerald Nightmare` |
| `sql/updates/world/master/<date>_NN_world.sql` | Template fixes, spawns, text, spell script names and area trigger rows. Next free `NN` on that date |

### Boss (`boss_elerethe_renferal`, `BossAI`)

- **State.** Event phases `PHASE_SPIDER` and `PHASE_ROC`. It also keeps the
  platform index (1-3), a form counter (for the 90 s and 92 s fill rates and
  the "flies only twice" rule), and an `_inFlight` flag.
- **Energy.** A 1 s repeating event adds the rate for the form. At 100 it
  schedules the form change and resets energy to 0.
- **Events.** One `EventMap` with phase masks. Timers are cited to DBM in a
  comment. Casts that come due during `_inFlight` or Feeding Time are pushed
  back until she is down again.
- **Flight.** `MovementInform` / `PointMovement` with fly mode, as a spline
  through the measured waypoints. On arrival she lands and clears
  `_inFlight`, and the Roc events resume.
- **Summons.** Everything she makes goes through `summons`, so evade and
  death clear it.

### Boundary

`BossBoundaryData` gets one `DATA_ELERETHE_RENFERAL` entry: a
`BoundaryUnionBoundary` of three `CircleBoundary` objects (one per platform,
each centred at that platform's floor z, since the check is 3D) and two
bridge boxes. A bridge box is a `RectangleBoundary` intersected with a
`ZRangeBoundary`, or a `PolygonBoundary` with a z range, whichever the plan
finds simplest. The way in from Il'gynoth's side must be inside it.

### Script names

Script names are checked against every existing script before they are
chosen. Il'gynoth collided with Ulduar's `npc_corruptor_tentacle`, and the
tree has spider scripts (Maexxna, Mother Smolderweb, Anub'arak). All new
names use the prefixes `npc_en_`, `spell_en_` and `at_en_`.

### SQL

One re-runnable file (`DELETE` then `INSERT`/`UPDATE`, a comment per block):

1. **`creature_template`.**
   - `faction` 14 and a `ScriptName` on 106087, 106311, 108542, 106350 and
     the egg sac.
   - `BaseAttackTime` 2000 where it is 0.
   - `unit_class` 4 on 106087.
   - The flags that keep the egg sac and tornado from being targeted or
     tanked.
   - 105604 is not changed.
2. **`creature_template_difficulty`.** Level 110 on all of them (Il'gynoth's
   templates came in at level 1).
3. **`creature` spawns** on map 1520 with `spawnDifficulties` `'14'`,
   `spawntimesecs` 604800 and `wander_distance` 0, from guid 1520103:
   - Elerethe at the P1 landing spot and facing;
   - the egg sacs at their readings.
4. **`creature_text`** for 106087:
   - yell 90971 (type 14);
   - emotes 111635 and 111411 (type 41);
   - any further lines Task 1 finds.
5. **`spell_script_names`** for the spell scripts.
6. **Area triggers.**
   - `areatrigger_template` and `areatrigger_create_properties` for 7078.
   - Custom ids for the Eye of the Storm, the Feather and the bridge web.
   - A static `areatrigger` spawn for each bridge web.
   - Visuals use `DecalPropertiesId`, since the spell-visual path does not
     render here (Nythendra lesson).

### Plan Task 1 (data dump) settles

- the Berserk spell;
- which egg-sac entry is the Emerald Nightmare one (by display and model
  file name);
- who applies 210850, and whether 221877 moves the tornado;
- the spiderling's death pool;
- whether 212993's proc does the crush;
- Malfurion's speaker;
- her lines in `BroadcastText.db2`.

Each finding goes into an amendment at the end of this spec.

## Measured inputs (gate in the plan)

The user flies the room with `.gm on` and `.gm fly on` and takes a `.gps` at
each point. The readings come from the `.gps` command lines in `GM.log`,
which carry x, y and z. The plan's skeleton task waits for:

1. **P1, P2, P3.** For each:
   - her landing spot and facing;
   - the platform centre;
   - 3 edge points at floor height;
   - the Gathering Clouds cubby (for reference only).
2. **Bridges P1→P2 and P2→P3:** both ends and the midpoint, on the web.
3. **Flight path:** 1-2 points in the air above each bridge.
4. **Egg sacs:** 3-4 spots per bridge and 2-3 per platform.
5. **The way in** from Il'gynoth's side.
6. **Whether the webs are walkable** on this server (vmaps and mmaps). If
   they are not, the bridges are a blocker, raised with the user before any
   code.

## Testing

Build in `~/arguscore` by `docs/BRANCHING.md` rules 9, 10 and 13:

- The tree is clean on `dev` and `pgrep -x docker-buildx` prints nothing.
- Ask the session that holds the folder before building.
- Use `~/emerald-nightmare/build.sh` and `boot_check.sh`.
- Never use `--no-cache`, and never restart while a client is connected.

In game, solo GM (Gairy), with DBM 7.3.31's bars as the reference:

1. **Boot.** `DBErrors.log` growth is no more than the baseline (13,422).
   There is no "script not found", and the server boots (no script-name
   collision).
2. **Spawns.** `.go instance emerald_nightmare` and walk to the room.
   - `.npc info` on her shows faction 14, the script and level 110.
   - The egg sacs stand at their spots.
3. **Pull.**
   - The encounter frame shows, with an energy bar.
   - DBM starts Necrotic Venom (12.2 s), Feeding Time (15.5 s) and Roc Form
     (90 s).
   - Berserk shows 540 s.
4. **Spider form.**
   - Necrotic Venom marks Gairy, ticks after 10 s and leaves pools.
   - Feeding Time lifts her, and spiderlings drop. Vile Ambush lands on the
     marked spot with a knockback.
   - Spiderlings stack Dripping Fangs and leave pools when they die. Swarm
     stacks on her while they are near her.
5. **Roc form at energy 100.**
   - The transform shows the roc.
   - Twisting Shadows marks Gairy, and a tornado forms 10 s later.
   - Gathering Clouds pushes Gairy back.
   - Dark Storm is cast, then the flight.
6. **The crossing.**
   - Feathers lie on the old platform. With one, Gairy jumps higher, takes no
     fall damage and crushes spiderlings by landing on them.
   - Walking on the web spawns a spiderling every 4 s.
   - An egg sac pops into Skittering Spiderlings.
   - The storm ramps until Gairy reaches the Eye, and stops hitting there.
     Stepping out of the Eye within 45 s brings the ticks back.
7. **Tank casts.** Raking Talons (2 hits) and Razor Wing on DBM's times.
8. **Back to Spider form** at 132 s, on P2. The second flight goes to P3.
9. **Berserk** at 540 s if she is alive.
10. **Evade.** `.die` in each form and on each platform:
    - everything despawns;
    - she is back on P1 in Spider form with 0 energy;
    - the egg sacs are back;
    - the next pull starts fresh.
11. **Kill.** `.damage` her to death:
    - `.instance getbossstate 2` reports Done;
    - the raid lock shows;
    - afterwards Gairy's state is reset with `.instance unbind all`.

Timings more than a few seconds off DBM are calibrate items, not bugs.

## Needs a group

Mechanics that solo GM testing cannot confirm. The in-game ability pass moves
items here rather than marking them OK or failed:

- Web of Pain pairing, the damage reflection, and Pain Lash.
- Necrotic Venom's ally splash and its group target count.
- Gathering Clouds' damage by distance across a raid.
- Swarm with several spiderlings spread across tanks.
- The Raking Talons tank swap.
- The Feather count against the raid size.

## Out of scope

- Heroic Venomous Pool dispersal by tornadoes (213781-213790), Mythic
  Tangled Webs, Violent Winds, Wind Burn and Nightmare Spawn, Raid Finder
  differences, and Mythic timers.
- A third flight (Berserk ends the fight first).
- Custom yell text for lines the client does not have.
- Loot, trash between Il'gynoth and Elerethe, graveyards and the hub portal.
- Solo-friendly tuning (GM commands only).

## Risks

- **Bridges.** If the webs are not walkable on this server's maps, the
  crossing cannot work as designed. This is checked first, in the measured
  inputs.
- **The flight.** A fly-mode spline over the bridge may clip the scenery.
  The air waypoints are measured to avoid that.
- **The Feather.** The jump-height aura and the crush proc depend on client
  and engine support. The fallback is an aura script that checks on landing.
- **Energy display.** The bar shows only if `unit_class` 4 gives her an
  energy power. The Il'gynoth Horror showed that it does.
- **Unsniffed templates.** More values than faction and level may be wrong
  (speed, flags, model scale). They are found in the test and fixed in data.
