# Emerald Nightmare: Il'gynoth, The Heart of Corruption

Date: 2026-10-17
Branch: `dev` (Emerald Nightmare content goes on `dev` directly, rule 3 of
`docs/BRANCHING.md`)
Status: design approved by the user 2026-10-17, section by section. Written
spec awaiting review.

## Goal

Il'gynoth (creature 105393, encounter 1873, `DATA_ILGYNOTH`) can be fought
and killed on Normal difficulty in the Emerald Nightmare with his retail
Normal mechanics. In the outside phase the raid kills tentacles and a
Nightmare Horror, and their Nightmare Ichors are blown up at the Eye of
Il'gynoth. In the inside phase the raid damages the heart in its chamber
until Dark Reconstitution throws them out, and Final Torpor ends the second
attempt. Killing the heart sets the boss state to Done and records the raid
lock. Solo testing uses GM commands.

## Decisions

All made by the user on 2026-10-17.

- **Scope.** Retail-faithful mechanics, Normal only. Heroic (Nightmarish
  Fury stacks), Mythic (Death Blossom, Dispersed Spores, explosion DoT) and
  Raid Finder are later pieces. One spec per boss, content on `dev`.
- **Timers.** Timers come from DBM 7.3.31 `DBM-EmeraldNightmare/Ilgynoth.lua`
  (the standing rule: the script fits the addon). How many adds come in each
  wave comes from Icy Veins' Normal spawn pattern. Retail "auto-pacing"
  (waves arriving sooner when the raid kills faster) is **not** built. The
  schedule is fixed.
- **Architecture.** Approach A: the heart's `BossAI` runs the encounter.
  It owns the phases and the wave schedule and summons every add. The Eye and
  each add type get a small AI of their own. (Rejected: the instance script
  as controller; the Eye as boss in Phase 1 with a hand-off to the heart.)
- **The hollow.** The client's own "Eye of Il'gynoth's Forcefield"
  (gameobject 251555) is spawned at the hollow mouth. It is closed in
  Phase 1 and open in Phase 2. "Inside the chamber" is a shape the user
  measures in game.
- **Final Torpor.** The first heart phase casts Dark Reconstitution (50 s).
  The second heart phase casts Final Torpor (90 s on Normal) instead. This
  is the literal reading of Icy Veins' "you are given 2 Phase Two to kill
  Il'gynoth". It is a calibrate item.
- **Positions.** Every position comes from the user's `.gps` readings in
  game. They are a gate in the plan, not inputs to this spec (see "Measured
  inputs").
- **Doors.** The two `NightmareBoss2DungeonDoor` templates (254849, 254850)
  are not spawned. Neither the client data nor Wowhead places them; it is the
  same ruling as Nythendra.
- **No leak-derived sources.** Mechanics come from DBM, the Legion-era
  guides and the client's DB2 data.

## Sources and what is uncertain

- DBM 7.3.31 `Ilgynoth.lua` (host copy `~/emerald-nightmare/dbm_ilgynoth.lua`).
- Icy Veins "Il'gynoth Abilities" (18 Sep 2016) and "Strategy/Tactics for
  Normal/Heroic" (4 Oct 2016).
- Wowhead NPC 105393 (journal text and spell list).
- warcraft.wiki.gg "Il'gynoth" (journal text and quotes).
- The client's DB2 files, dumped with `~/emerald-nightmare/dump_ilg2.py`
  (output in `ilgynoth_spells.txt` and `ilgynoth_spells2.txt`).

Research notes: `ilgynoth-research.md` in the session notes folder.

Calibrate items. Nothing beyond these is invented:

1. **Wave counts** after the second wave of each type. DBM times them, but
   Icy Veins only gives counts up to 180 s.
2. **Spew Corruption.** How often it is recast; DBM does not time it.
3. **Nightmare Gaze.** Its rate: Icy Veins says every 2-3 s, and DBM calls
   it spam.
4. **Cursed Blood.** The target count per raid size: 2-6 per Icy Veins,
   and 1 solo.
5. **Nightmare Explosion.** The range to the Eye (13 yd, from the spell's
   radius) and the 1 HP / 2.5 s cast behaviour.
6. **Final Torpor.** Which heart phase casts it (see Decisions).
7. **Violent Bloodburst** (215971). The guides do not say when it fires.
   Plan Task 1 reads its data and either builds it or records it as a known
   gap.
8. **The random tentacle** every 60 s after a phase's list runs out.
9. **The low-health yell** (109143) at 30%: retail gives no threshold.

## What exists today

Checked 2026-10-17 on `dev` (`2e52703e`) and the server's databases.

- **Scripts.** `emerald_nightmare.h` has `DATA_ILGYNOTH = 1` and
  `BOSS_ILGYNOTH = 105393`. `instance_emerald_nightmare.cpp` has the
  creature `ObjectData` for 105393, encounter 1873, and a boundary for
  Nythendra only. There is no gameobject `ObjectData` and no `DoorData`.
- **Templates.** None of these has a script, and their `BaseAttackTime` is
  0:

  | Entry | Name | Faction | HealthModifier | Display |
  |---|---|---|---|---|
  | 105393 | Il'gynoth `<The Heart of Corruption>` | 35 | 66 | 69115 |
  | 105906 | Eye of Il'gynoth | 35 | 900 | 69879 |
  | 105304 | Dominator Tentacle | 35 | 18 | 69882 |
  | 105322 | Deathglare Tentacle | 35 | 8 | 69881 |
  | 105383 | Corruptor Tentacle | 35 | 3 | 69880 |
  | 105591 | Nightmare Horror | 35 | 52 | 70958 |
  | 105721 | Nightmare Ichor | 35 | 0.9 | 69925 |

  Each has a difficulty-0 row only. Not used: 110808 (a second "Il'gynoth"),
  112578 "Heart of Corruption", 112307 and 112371 "Cosmetic Nightmare
  Tentacle", 104592 "Nightmare Tentacle".
- **Gameobjects.** 251555 "Eye of Il'gynoth's Forcefield" (type 0, display
  6391, size 2.37) and 254849 and 254850 `NightmareBoss2DungeonDoor`. None
  is spawned.
- **Spawns.** Map 1520 has only Nythendra (guid 1520101). Wowhead has only
  the journal pin (zone 8026, floor 4, 48.3 / 48.2) for the boss and all
  adds.
- **Broadcast text** in `hotfixes.broadcast_text`:
  - 109119: "Come, little ones. Savor your fear as we feast upon it!"
  - 109141: "Every blow struck at our heart will be repaid a thousand fold!"
  - 109142: "Out! Out! You are not yet worthy!"
  - 109143: "Still you fight? Still you refuse the gift?"
  - 109144: "N'zoth... I journey... to Ny'alotha..."
  - 111379: the emote "The Eye of Il'gynoth is creating a Nightmare Horror!"
  - 109470: the emote "Dark Reconstitution almost completed".
  - 122666: the emote "Final Torpor almost completed".
  - 109189: the whisper "A Dominator Tentacle faces you and readies Ground
    Slam!"
- **No other rows.** There is no `creature_text`, no `spell_script_names`
  row for these spells, and no `areatrigger_create_properties` row for
  6514 or 7424.

## Spells (Normal, from the client data)

| Spell | Id | Data facts | Script needed |
|---|---|---|---|
| Stuff of Nightmares | 209915 | Eye; permanent; -99% damage taken | none (cast on spawn and respawn) |
| Nightmare Gaze | 210931 | 1.5 s cast; single target | cast schedule |
| Nightmare Ichor (summon) | 209966 | SUMMON 105721 at the caster | cast N times on an add's death |
| Fixate | 210099 | aura on the Ichor | target choice, chase |
| Touch of Corruption | 209467 → 209469 | proc on melee; 40 s DoT that stacks | none (passive aura) |
| Nightmare Explosion | 209471 | 2.5 s cast; damage to players in its radius | 5% of the Eye's max health if within range |
| Eye of Fate | 210984 | instant; line; 29 s debuff | cast schedule |
| Seeping Corruption | 209387 | periodic every 5 s → 212855 → 208931 | none (passive aura) |
| Nightmare Corruption (pool) | 208931 → AT 6514, aura 212886 | 90 s pool; damage, pacify and silence | `areatrigger_*` rows |
| Mind Flay | 208697 | 3 s cast; 8 s; interruptible | cast schedule |
| Spew Corruption | 208929 | 10 s aura with a periodic dummy every 2 s | dummy tick casts 208931 at the target |
| Nightmarish Fury | 215234 | 6 s; triggers 215233 every 750 ms | cast schedule |
| Ground Slam | 208689 | 2 s cast; line; knockback | target choice, whisper |
| Rupturing Roar | 208685 | 2 s cast; raid-wide | cast while nobody is in melee |
| Summon Horror visual | 210289 | 4 s | cast on the Horror's arrival (DBM key) |
| Dark Reconstitution | 210781 | 50 s cast; instakill, 88 yd | restrict kills to "inside" |
| Final Torpor | 223121 | 90 s cast on Normal; instakill, 88 yd | none beyond the cast |
| Cursed Blood | 215128 | 8 s DoT on enemies within 88 yd | target cap; burst 215143 on expiry |
| Reabsorption | 212942 | raid damage | cast when an Ichor enters the chamber |
| Violent Bloodburst | 215971 → 215975 → AT 7424 | 15 s pool | per calibrate item 7 |

Plan Task 1 dumps every spell in the table again with radii, target
types, `SpellTargetRestrictions` and area-trigger shapes. Anything the data
already does is not scripted.

## The encounter

### Phase 1, "The Ruined Ground"

1. **Heart.** It is in the chamber and cannot be selected (`UNIT_FLAG_UNINTERACTIBLE`).
   It runs the timers and casts nothing.
2. **Eye.** It stands at the mouth, rooted, with Stuff of Nightmares. It
   casts Nightmare Gaze on a random player every 2-3 s.
3. **Waves.** The heart spawns these as its summons (times from DBM, counts
   from Icy Veins):

   | | First Phase 1 (from the pull) | After a heart phase |
   |---|---|---|
   | Dominator | 2 at 0 s, measured spots | 2 at 0 s, same spots |
   | Deathglare | 21.5 s ×1, 83.5 s ×2, 168.5 s ×1, 223.5 s ×1 | 21.5 s ×1, 111.5 s ×2, 241.5 s ×1 |
   | Corruptor | 79 s ×2, 174 s ×2, 209 s ×1 | 45 s ×2, 140 s ×2, 175 s ×1, 260 s ×1, 300 s ×1 |
   | Horror | 60 s, then every 280 s | 95 s, then every 280 s |
   | Afterwards | one random tentacle (Deathglare or Corruptor) every 60 s | same |

   - **Spawn points.** Tentacles spawn at random points on the measured
     tentacle ring around the Eye. The Horror spawns at its measured spot.
     The Eye announces it with emote 111379, and the Horror casts 210289.
   - **Ichor count.** In the first Phase 1 the waves up to 174 s drop
     exactly 20 Ichors (2×2 + 1×2 + 4 + 2×1 + 2×2 + 1×2 + 2×1), the number
     the Eye needs.
4. **Ichors.** When an add dies it casts 209966 to summon its Ichors at its
   body:
   - Dominator 2, Deathglare 2, Corruptor 1, Horror 4.
   - Each Ichor fixates a random player (210099) and chases them. It does
     not change target unless its target dies.
5. **Ichor death.** An Ichor cannot drop below 1 HP. At 1 HP it stops,
   casts Nightmare Explosion (2.5 s), then dies.
   - If the Eye is within range when the cast ends (13 yd, calibrate), the
     Eye loses 5% of its maximum health, ignoring Stuff of Nightmares.
   - The Eye dies after 20 such explosions.
6. **Adds' abilities:**
   - **Dominator.** Rooted; tanked.
     - Nightmarish Fury on its victim, first at 6 s, then every 10.9 s.
     - Ground Slam at a random player, first at 12 s, then every 20.5 s,
       with whisper 109189 to that player.
     - If no player is in melee range, it casts Rupturing Roar back to back
       until one is.
   - **Deathglare.** Rooted. It casts Mind Flay at a random player and
     recasts it as soon as the last cast ends or is interrupted. It melees
     a player in reach.
   - **Corruptor.** Rooted. It casts Spew Corruption at the players near it,
     back to back (calibrate). Each dummy tick drops a Nightmare Corruption
     pool under the target.
   - **Horror.** It moves and is tanked. Eye of Fate on its victim, first at
     14 s, then every 10 s. Seeping Corruption is a passive aura.

### Phase 2, "The Heart of Corruption"

Starts when the Eye dies.

1. **Phase 1 stops.** Its events are cancelled (the next Phase 1 starts
   from the after-heart lists). Adds still
   alive keep fighting outside.
2. **The heart opens up.** The forcefield opens and the heart becomes
   selectable and attackable. It yells 109141.
3. **Cursed Blood** every 15 s:
   - Targets: 1 solo, otherwise `clamp(players / 5, 2, 6)`, chosen among
     players inside.
   - On expiry the target bursts with 215143.
4. **The timed cast:**
   - Heart phase 1: Dark Reconstitution (50 s).
   - Heart phase 2: Final Torpor (90 s).
   - A few seconds before the end, the heart sends the matching emote,
     109470 or 122666.
5. **Reabsorption.** An Ichor that comes inside the chamber area makes the
   heart cast Reabsorption (212942) and is removed.
6. **Low health.** At 30% health the heart yells 109143, once per fight.
   The threshold is a calibrate value; retail gives none.

### End of a heart phase

- **Dark Reconstitution ends.** Every player inside the chamber area dies.
  Outside players are untouched.
  - The heart yells 109142, closes the forcefield and becomes unselectable
    again.
  - The Eye respawns at full health with Stuff of Nightmares.
  - Phase 1 restarts on the after-heart timers.
  - Adds still alive stay. The heart keeps its health.
- **Final Torpor ends.** Every player in the encounter area dies (the
  spell's own 88 yd instakill). The raid wipes and the encounter evades.
- **Heart dies.** It yells 109144 and the boss state goes to Done. The
  forcefield opens and stays open. Remaining adds and pools are despawned.

### Evade and reset

At any point, `EnterEvadeMode` on the heart does the following:

- Despawns all summons (adds and Ichors) and the pools.
- Closes the forcefield and respawns the Eye.
- Resets the heart's health and the heart-phase count, and sends
  `ENCOUNTER_FRAME_DISENGAGE` for every framed unit.

The Eye and the adds evade through the heart.

### Encounter frames

The heart, the Eye and each Dominator are sent with
`ENCOUNTER_FRAME_ENGAGE` (DBM watches Dominators through
`INSTANCE_ENCOUNTER_ENGAGE_UNIT`).

### Pull

The Eye is the pull. It is hostile (faction 14) with its normal aggro
radius, so damaging it or coming into that radius engages it:

- Its `JustEngagedWith` engages the heart through the instance
  (`GetCreature(DATA_ILGYNOTH)`).
- The heart calls `DoZoneInCombat` and spawns the first two Dominators.

Before the pull no adds exist.

## Design

### Files

| File | Change |
|---|---|
| `src/server/scripts/BrokenIsles/EmeraldNightmare/boss_ilgynoth.cpp` | New: `boss_ilgynoth` (heart), `npc_eye_of_ilgynoth`, the five add AIs, the spell and aura scripts, the pool area-trigger AI if needed, and `AddSC_boss_ilgynoth`. |
| `.../emerald_nightmare.h` | Add the Il'gynoth creature ids (Eye, four adds, Ichor), `GO_EYE_OF_ILGYNOTH_FORCEFIELD = 251555`, and `DATA_EYE_OF_ILGYNOTH` and `DATA_ILGYNOTH_FORCEFIELD` as extra data ids after the encounters. |
| `.../instance_emerald_nightmare.cpp` | Add the Eye to the creature `ObjectData`, a gameobject `ObjectData` with the forcefield, and the Il'gynoth boundary. In `OnGameObjectCreate`, the forcefield is opened if `DATA_ILGYNOTH` is Done. |
| `src/server/scripts/BrokenIsles/broken_isles_script_loader.cpp` | Declare and call `AddSC_boss_ilgynoth()` under `// Emerald Nightmare`. |
| `sql/updates/world/master/2026_10_17_NN_world.sql` (or the date of the work) | Template fixes, spawns, text, spell script names, area trigger rows. Next free `NN` on that date. |

### Heart (`boss_ilgynoth`, `BossAI`)

- **Phases.** Event phases `PHASE_OUTSIDE` and `PHASE_HEART`. Plus a counter
  `_heartPhases` (0, 1, 2) and the wave cursor for each add type: index
  into its time list and its count list, with separate lists for the first
  Phase 1 and for later ones.
- **Waves.** One event per add type, rescheduled from its list after each
  spawn. Once a list is used up, a single `EVENT_RANDOM_TENTACLE` runs every
  60 s.
- **Phase change.** `DoAction(ACTION_EYE_DIED)` from the Eye starts the
  heart phase. `DoAction(ACTION_ICHOR_INSIDE)` makes the heart cast
  Reabsorption.
- **Ichor drops.** `JustSummoned` and `SummonedCreatureDies` keep the
  summon list. An add's death casts its Ichors through the add's own AI, so
  Ichors are owned by the heart's summon list through the add's summoner.
  The plan checks this ownership; if summons of summons are not owned by
  the heart, the add summons with the heart as summoner.
- **End of the timed cast.** On Dark Reconstitution's successful cast
  (`OnSuccessfulSpellCast` / `SpellHit` on self, as the plan finds), the
  heart kills the players inside (see "Inside the chamber") and resets the
  outside phase.
- **Final Torpor** needs nothing extra: its instakill covers the room, and
  the wipe evades the encounter.

### Eye (`npc_eye_of_ilgynoth`, `ScriptedAI`)

- **Setup.** It is rooted. In `JustAppeared` it casts Stuff of Nightmares
  on itself.
- **Pull.** `JustEngagedWith` pulls the heart.
- **Gaze.** A 2-3 s Nightmare Gaze timer.
- **The 5% hit.** The Eye does nothing itself. The Ichor applies the 5%
  (see "Adds"). All other damage is cut 99% by the aura, and there is no
  other way to hurt the Eye.
- **Death.** `JustDied` calls `ACTION_EYE_DIED` on the heart. The Eye is
  respawned by the heart with `Respawn(true)` or a fresh summon at its spot.
  The plan picks one: `Respawn` if the spawn stays a DB spawn, and the
  plan checks that a dead DB creature in an instance can be respawned at
  once.

### Adds

`ScriptedAI` per type, with `TaskScheduler` or `EventMap` as Guarm does:

- **Shared.** Rooted tentacles. Each tentacle calls `ENCOUNTER_FRAME_ENGAGE`
  if it is a Dominator. Each has an `_ichors` count and casts 209966 that
  many times in `JustDied`.
- **Ichor.** `npc_nightmare_ichor`:
  - On summon: fixate (210099) on a random player and chase.
  - `DamageTaken` caps damage at 1 HP. At 1 HP it casts 209471. At the
    cast's end it checks the distance to the Eye. If the Eye is in range,
    it lowers the Eye's health by 5% of its maximum. It sets the health
    directly so the aura cannot reduce it; at 0 the Eye is killed with
    `Unit::Kill` from the Ichor. Then the Ichor kills itself. This AI is
    the only owner of the 5%.
  - Each update it checks whether it is inside the chamber area during
    the heart phase; if so it tells the heart (Reabsorption).

### Spell and aura scripts

Through `spell_script_names`:

- **Spew Corruption** (208929): the periodic dummy tick casts 208931 at the
  aura's target.
- **Cursed Blood** (215128): a target filter caps the target count. Its
  `AfterRemove` (expired removals only) casts 215143 from the target.
- **Dark Reconstitution** (210781): a target filter keeps only players
  inside the chamber area. If the filter cannot do this, because the
  instakill is a self-centred area that the data fixes, the heart kills the
  inside players itself and the spell's effect is filtered to no targets.
- Nightmare Explosion (209471) needs no spell script: the Ichor AI owns
  the 5%.

### Inside the chamber

The chamber area is a measured shape (circle or polygon through the
chamber-edge readings, with the floor z), stored as constants in
`boss_ilgynoth.cpp`. It answers "is this unit inside?" for:

- Dark Reconstitution's kills.
- Reabsorption.
- Cursed Blood's target choice.

### Instance script

- **Boundary.** `BossBoundaryData` gets a `DATA_ILGYNOTH` entry fitted to
  the room-edge readings. It is 3D, so the floor z is needed. It must
  contain the chamber, the Eye, the tentacle ring and the doorway.
- **Forcefield.** Gameobject `ObjectData` holds `{ GO_EYE_OF_ILGYNOTH_FORCEFIELD,
  DATA_ILGYNOTH_FORCEFIELD }`. The heart opens and closes it with
  `instance->HandleGameObject(guid, open)`. It is not `DoorData`, because
  the barrier changes inside the fight.

### SQL

One re-runnable file (`DELETE` then `INSERT`/`UPDATE`, a comment per
block):

1. **`creature_template`.** `faction` 14 and `ScriptName` on 105393,
   105906, 105304, 105322, 105383, 105591 and 105721. Rows whose load-time
   default is not enough get `BaseAttackTime` 2000. On the Eye: the
   `flags_extra` values that block threat and knockback, whichever this
   fork names. The heart and Eye are rooted by script (the DB refuses
   `speed_*` 0).
2. **`creature` spawns** on map 1520 with `spawnDifficulties` `'14'`,
   `spawntimesecs` 604800, `wander_distance` 0 and free guids (Nythendra
   took 1520101):
   - the heart at its reading;
   - the Eye at its reading and facing.
3. **`gameobject` spawn** of 251555 at the hollow mouth, state closed.
4. **`creature_text`** for 105393: the five yells (type 14), emotes 109470
   and 122666 (type 41). For 105906: emote 111379 (type 41). For 105304:
   whisper 109189 (type 42).
5. **`spell_script_names`** for the scripts above.
6. **Area triggers.** `areatrigger_template` and
   `areatrigger_create_properties` for 6514 (Nightmare Corruption), plus
   7424 if Violent Bloodburst is built. Shapes come from the client's
   `AreaTrigger.db2`. Visuals use `DecalPropertiesId`, because the
   spell-visual path does not render here (Nythendra lesson).

## Measured inputs (gate in the plan)

The user flies the room with `.gm on` and `.gm fly on` and takes a `.gps`
at each point. The readings are read from the `.gps` command lines in
`GM.log`, which carry x, y and z. The plan's skeleton task waits for:

1. The Eye: its spot and the facing (the way it looks out of the hollow).
2. The heart: where it sits in the chamber.
3. Dominator A and Dominator B: one on each side of the Eye.
4. The Horror's spawn spot.
5. The hollow mouth, where the forcefield goes.
6. Chamber edge: 3-4 points at floor height. These define "inside".
7. Room edge: 3-4 points at floor height, which give the boundary.
8. The doorway the raid enters through from Nythendra's side.
9. Tentacle ring: 2-3 spots where tentacles should appear around the Eye.
   They give the ring's centre and radius.

The Wowhead journal pin (zone 8026, floor 4, 48.3 / 48.2) shows roughly
where to look.

## Testing

Build in `~/arguscore` by `docs/BRANCHING.md` rules 9, 10 and 13:

- The tree is clean on `dev` and `pgrep -x docker-buildx` prints nothing.
- Ask the session that holds the folder before building.
- Use `~/emerald-nightmare/build.sh` and `boot_check.sh`.
- Never use `--no-cache`, and never restart while a client is connected.

In game, solo GM (Gairy). DBM 7.3.31 is installed so its bars can be
compared:

1. **Boot.** `DBErrors.log` growth is no more than the baseline (13,422). No
   "script not found" for the new script names.
2. **Spawns.** `.go instance emerald_nightmare` and walk to the room.
   `.npc info` on the heart and the Eye shows faction 14, the script and
   level 110. The forcefield is closed, and the heart cannot be selected.
3. **Pull.** Hitting the Eye engages the encounter:
   - Frames show for the heart, the Eye and the two Dominators.
   - DBM starts its Fury (6 s), Slam (12 s) and Deathglare (21.5 s) bars.
   - The Dominators cast Fury and Slam on time, and the Slam whisper
     shows.
4. **Rupturing Roar.** Walk out of a Dominator's melee range and it roars
   on repeat.
5. **Waves.** Each wave appears within 2 s of DBM's bar: Deathglare,
   Horror (with the emote), Corruptor.
   - Deathglares Mind Flay, and an interrupt works.
   - Spew Corruption drops pools every 2 s under its target.
   - The Horror casts Eye of Fate and leaves pools.
6. **Ichors.** Killing each add type gives the right number of Ichors. Each
   fixates Gairy.
   - At 1 HP an Ichor casts Nightmare Explosion and dies.
   - Near the Eye, the Eye's health drops 5%. Far from it, nothing.
7. **The Eye.** After 20 near explosions (or GM help: a debug
   `.damage` path the plan picks), the Eye dies:
   - The forcefield opens and the heart can be selected and damaged.
   - The heart yells and Cursed Blood lands every 15 s and bursts.
   - The Dark Reconstitution bar shows 50 s.
8. **Reconstitution, outside.** Leave the chamber before the end: you live.
   The forcefield closes and the Eye is back at full health.
   - Adds left alive are still there, and the heart's health is unchanged.
   - Phase 1 restarts on the after-heart timers (Deathglare 21.5 s,
     Corruptor 45 s, Horror 95 s).
9. **Reconstitution, inside.** Repeat, but stay inside: you die.
10. **Reabsorption.** Walk a fixated Ichor into the chamber in a heart
    phase: it is reabsorbed, with raid damage.
11. **Final Torpor.** The second heart phase shows a 90 s Final Torpor bar.
    At its end everyone dies and the encounter resets.
12. **Evade.** `.die` in each phase. Everything despawns, the forcefield is
    closed, the Eye is back, and the next pull starts fresh.
13. **Kill.** `.damage` the heart to death in a heart phase:
    - `.instance getbossstate 1` reports Done.
    - The forcefield stays open after a re-login.
    - The raid lock shows.
    - Afterwards Gairy's state is reset with `.instance unbind all`.

Timings more than a few seconds off DBM are calibrate items, not bugs.

## Out of scope

- Heroic Nightmarish Fury stacks, Mythic Death Blossom and Dispersed
  Spores, the Mythic explosion DoT, Raid Finder differences, and Mythic
  timers.
- Retail add auto-pacing.
- Loot, trash on the way from Nythendra, and graveyards.
- The `NightmareBoss2DungeonDoor` objects.
- Solo-friendly tuning (GM commands only).

## Risks

- **Add timings.** Wave counts and late timers are partly unverified (see
  "Sources and what is uncertain").
- **Dark Reconstitution targeting.** If the spell's 88 yd instakill cannot
  be filtered to the chamber, the heart kills the players inside itself. The
  plan's Task 1 dump settles which.
- **Eye damage.** The 5% is set directly on the Eye's health, so no
  combat-log damage event is sent for it. DBM does not key on that, so
  nothing is lost.
- **Summon ownership.** Ichors summoned by adds must despawn on evade.
- **Forcefield model.** Display 6391 is the client's own forcefield; how it
  looks and whether it blocks movement is checked in game. If it does not
  block, the Eye's position and a server-side push-back are discussed then.
- **Unsniffed templates.** More values than faction may be wrong (speed,
  flags, model scale). They are found in the test and fixed in data.

## Amendment after the data dump (plan Task 1, 2026-10-18)

The "88" and "13" above are radius indexes, not yards. Corrections from the
client data (`~/emerald-nightmare/ilgynoth_task1.txt`, notes
`ilgynoth-task1-findings.md`):

- **Nightmare Explosion.** Its radius is 10 yd, so the Eye takes the 5%
  from Ichors within 10 yd, not 13.
- **Dark Reconstitution, Final Torpor and Cursed Blood.** They reach 300 yd.
  - Dark Reconstitution's instakill is an area-enemy target
    (`TARGET_UNIT_DEST_AREA_ENEMY`), so a target filter on effect 0 cuts it
    down to the chamber.
  - Final Torpor needs no filter: 300 yd covers the whole floor.
- **Spew Corruption.** It has a 10 s cast on Normal and hits one enemy
  target, not an area. "Back to back" means a new cast after each one ends.
- **Pools.** Area triggers 6514 and 7424 have no client shape. Pools are
  spheres of radius 5 (calibrate), using decal 31, the Nythendra green.
- **Cursed Blood burst** (215143). It is 11 yd, as the journal says.
- **Violent Bloodburst** is not built. Nothing in the client data, the world
  database or DBM casts 215971. Known gap.
- **Forcefield 251555.** Its model is `collisionwallpvp01.m2`, a thin
  invisible collision wall. It blocks movement but shows nothing.
