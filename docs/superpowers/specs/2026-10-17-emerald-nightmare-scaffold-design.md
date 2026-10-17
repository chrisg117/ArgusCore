# Emerald Nightmare scaffold: an empty raid you can walk into

Date: 2026-10-17
Branch: `dev` (content goes on `dev` directly while one session works on
it, rule 3 of `docs/BRANCHING.md`)
Status: approved by the user 2026-10-17

## Goal

A player can walk into the Emerald Nightmare (map 1520) through its portal
in Val'sharah, get an instance with a working `InstanceScript` and a raid
lock, and walk back out. The raid is empty: no bosses, trash or doors yet.

This is the first of a series. Each boss after it gets its own spec, branch
commits and in-game test, starting with Nythendra. The scaffold gives those
specs somewhere to plug in: an instance script that already knows all seven
encounters, so a boss spec only adds its own spawn, doors, boundary and
script.

## Decisions taken while drafting

- Emerald Nightmare is worked on before the unfinished Trial of Valor and
  Antorus bosses (user, 2026-10-17).
- Mechanics follow retail, Normal difficulty first. Making a boss soloable
  is left to GM commands for now, and Heroic and Mythic extras come later
  (user, 2026-10-17). This leaves the pinned scaling question where it is.
- The scaffold is its own piece. Nythendra is the next spec, not part of
  this one (user, 2026-10-17).
- Players enter through a walk-in portal, as on retail, not only by GM
  teleport or the dungeon finder (user approved the design, 2026-10-17).

## What exists today

Checked on 2026-10-17 against `dev` (tree `9c3cbaa`), the server's world
database and the client's DB2 files for build 7.3.5.26972.

Scripts: nothing. There is no `EmeraldNightmare` directory and no file
mentions map 1520. The only Legion raids with scripts are Trial of Valor
(Guarm) and Antorus (Garothi Worldbreaker).

World database:

- No `instance_template` row for 1520. Without it the instance never
  creates an `InstanceScript` (`InstanceMap::CreateInstanceData`), and
  `.go instance` cannot find the raid by name.
- No creature, gameobject or areatrigger spawns on map 1520.
- The boss templates exist but are unsniffed (faction 35, no script):
  Nythendra 102672, Il'gynoth 105393, Elerethe Renferal 106087, Ursoc
  100497, Ysondre 102679, Taerar 102681, Lethon 102682, Emeriss 102683,
  Cenarius 104636 (faction 14), Xavius 103769.
- `areatrigger_teleport` already has the two server-only ids added in commit
  `0bc88ac1`: `-27` "The Emerald Nightmare Entrance" (WorldSafeLocs 5533)
  and `-28` "The Emerald Nightmare Exit" (WorldSafeLocs 5687). Negative ids
  have no client trigger behind them; `.go instance` and the dungeon finder
  use them to find where to put a player.
- `hotfixes.broadcast_text` has Nythendra's raid emotes (105376 Rot, 105969
  Volatile Rot, 106119 Infested Breath). Not needed here; noted for the
  Nythendra spec.

Client data:

- `Map` 1520: directory `NightmareRaid`, instance type 2 (raid), expansion 6.
- `MapDifficulty` 3374-3377: difficulties 14 Normal, 15 Heroic (30
  players), 16 Mythic (20), 17 Raid Finder (30), lock ids 1-4.
- `WorldSafeLocs` 5533: map 1520, (1810.12, 1424.19, 355.17), facing 339.91
  degrees. This is the arrival point inside.
- `WorldSafeLocs` 5687: map 1220, (3588.28, 6483.40, 177.97), facing 218.88
  degrees. This is where a player leaving the raid lands in Val'sharah.
- `AreaTrigger.db2` has no trigger volumes on 1520, and none at the portal
  on 1220. The client data has no walk-in portal for any Legion raid, so
  the portal has to be a server-side trigger.
- The server already has the map's terrain: 227 map tiles, 171 mmap tiles
  and the vmaps for 1520.

`DungeonEncounter` (difficulty 0 rows, which apply to every difficulty):

| Encounter | Id | Bit | Journal order |
|-----------|------|-----|---------------|
| Nythendra | 1853 | 1 | 1 |
| Il'gynoth, The Heart of Corruption | 1873 | 4 | 2 |
| Elerethe Renferal | 1876 | 5 | 3 |
| Ursoc | 1841 | 0 | 4 |
| Dragons of Nightmare | 1854 | 2 | 5 |
| Cenarius | 1877 | 6 | 6 |
| Xavius | 1864 | 3 | 7 |

The journal instance is 768. The dungeon finder lists the raid as 1348
(Normal), 1349 (Heroic) and 1350 (Mythic), plus the Raid Finder wings
1287 Darkbough, 1288 Tormented Guardians and 1289 Rift of Aln.

## Scope

In scope:

1. The script files for the instance (header, instance script, loader
   lines).
2. The `instance_template` row.
3. A walk-in entrance portal on map 1220 and an exit trigger inside the
   raid, both server-side areatriggers using the core's built-in teleport
   action.

Out of scope, each for a later spec:

- Any boss, trash or door spawn, and any boss script. Nythendra is next.
- Boss boundaries and door data. Each boss spec adds its own.
- The order in which bosses unlock.
- Heroic, Mythic and Raid Finder differences, and tuning for solo play.
- The dungeon finder. It may well work once `instance_template` exists, but
  it is not tested here.
- Graveyards and corpse runs inside the raid.

## Script files

New directory `src/server/scripts/BrokenIsles/EmeraldNightmare/`, laid out
like `TrialOfValor/`.

### `emerald_nightmare.h`

- `DataHeader "EmeraldNightmare"` and
  `ENScriptName "instance_emerald_nightmare"`.
- `EncounterCount = 7`.
- `ENDataTypes`: one `DATA_*` per encounter, in journal order:
  `DATA_NYTHENDRA = 0`, `DATA_ILGYNOTH`, `DATA_ELERETHE_RENFERAL`,
  `DATA_URSOC`, `DATA_DRAGONS_OF_NIGHTMARE`, `DATA_CENARIUS`,
  `DATA_XAVIUS`. Journal order is what the boss specs will be written in;
  the encounter bit comes from `DungeonEncounter`, so the enum does not
  need to follow it.
- `ENCreatureIds`: the boss entries listed above, named `BOSS_*`. The four
  dragons get one constant each.
- `GetEmeraldNightmareAI` and `RegisterEmeraldNightmareCreatureAI`, as in
  `trial_of_valor.h`.
- No gameobject ids yet. The first boss spec that needs a door adds an
  `ENGameObjectIds` enum.

### `instance_emerald_nightmare.cpp`

- `ObjectData creatureData[]` for the six single-creature encounters:
  Nythendra, Il'gynoth, Elerethe, Ursoc, Cenarius and Xavius. The Dragons
  of Nightmare encounter has four creatures; how they are tracked is left
  to that boss's spec.
- `DungeonEncounterData encounters[]` mapping each `DATA_*` to its id from
  the table above.
- No `DoorData` and no `BossBoundaryData` yet.
- `InstanceMapScript(ENScriptName, 1520)`; the constructor calls
  `SetHeaders`, `SetBossNumber(EncounterCount)`, `LoadObjectData` and
  `LoadDungeonEncounterData`.

### Loader

`broken_isles_script_loader.cpp` gets a `// Emerald Nightmare` block with
`AddSC_instance_emerald_nightmare()` declared and called. There is no boss
`AddSC_` until Nythendra's spec.

## Database

One file, `sql/updates/world/master/2026_10_17_00_world.sql` (use the next
free number on that date if another branch has taken `_00`), written to be
re-run safely: `DELETE` then `INSERT`, with a comment on each block.

### Instance template

`instance_template`: `(map 1520, parent 0, script 'instance_emerald_nightmare')`.

### Entrance and exit triggers

The pattern is the one the database already uses for "The Everbloom - Exit"
(spawn 203) and "The Seat of the Triumvirate - Enter" (spawn 204), each
with three rows:

- `areatrigger_template` (`IsCustom = 1`, `Flags = 1`).
- `areatrigger_template_actions` with `ActionType = 2`
  (`AREATRIGGER_ACTION_TELEPORT`), `ActionParam` = a WorldSafeLocs id, and
  `TargetType = 5`.
- `areatrigger_create_properties` (`IsCustom = 1`, pointing at the
  template, with a shape).
- An `areatrigger` spawn (`IsCustom = 1`).

`AreaTrigger::DoActions` uses the player who walks in as the caster when
the trigger is a static spawn, so the teleport needs no script.
`TargetType` is ignored for static spawns.

| | Entrance (Val'sharah) | Exit (inside) |
|---|---|---|
| Template / create properties id | 152001 | 152002 |
| Teleports to | WorldSafeLocs 5533 (inside) | WorldSafeLocs 5687 (Val'sharah) |
| Spawn id | 1220005 | 1520001 |
| Map, difficulties | 1220, `0` | 1520, `14,15,16,17` |
| Shape | sphere (0), radius 4 | sphere (0), radius 4 |
| Comment | `The Emerald Nightmare - Entrance` | `The Emerald Nightmare - Exit` |

The ids follow the fork's habit of spawn id = map id x 1000 + n. 152001,
152002 and 1520001 were unused on 2026-10-17. 1220001-1220004 are taken by
the priest content, so the entrance takes 1220005. Check all of them
again when the SQL is written, since other feature branches may have
added rows.

The entrance portal is not in the client's trigger data. Its position is
found in game:

1. Go to WorldSafeLocs 5687 in Val'sharah. Players land there facing away
   from the portal, so the portal is behind them, roughly towards a
   bearing of 38.9 degrees.
2. Stand in the middle of the portal's visual and take `.gps`. That gives
   the spawn position.

The exit trigger goes just behind the arrival point inside: a few yards
from 5533, against the facing of 339.91 degrees. Players arriving at 5533
must not land inside it, or they would bounce straight back out. Its exact
spot is also checked in game.

The arrival points sit outside the triggers by design, so a player is never
teleported into the other trigger. If the radius-4 sphere does not match
the portal's visual, change the radius during testing. A box (shape 1)
would only be worth it for an oddly shaped portal.

## Testing

Build in `~/arguscore`, which stays on `dev`. Rule 10 of
`docs/BRANCHING.md` applies: one session owns the folder at a time, so
before building check the tree is clean, that no build is running, and
that no other session is mid-change there.

1. **Boot.** The world server starts and `DBErrors.log` grows by no more
   than the current baseline (13,423 lines on the last check). The log
   shows the instance script loading with no "script not found" error for
   `instance_emerald_nightmare`.
2. **Walk in.** With `.gm on`, Gairy walks into the Val'sharah portal and
   arrives at the raid entrance. This shows the entrance trigger and the
   instance both work. With `.gm off` and no raid group, the same portal
   refuses her with the "need a raid group" message. That shows the
   trigger goes through the normal entry checks.
3. **Instance and lock.** Inside, `.instance stats` names
   `instance_emerald_nightmare`, and `.instance getbossstate 0` answers
   with a state rather than an error. This shows the script attached and
   knows the bosses.
4. **Walk out.** Walking into the exit trigger puts Gairy back in
   Val'sharah at 5687.
5. **GM teleport works.** `.go instance emerald_nightmare` takes Gairy to
   the raid through trigger `-27`. The command matches instance script
   names, not map ids, so it only finds the raid once `instance_template`
   exists. Its "Could not find exit portal" line is expected: every Legion
   map has no corpse map in the client data, and
   `ObjectMgr::GetGoBackTrigger` gives up on such maps (corrected
   2026-10-17, during Task 1).

Afterwards Gairy is left as she was: the raid lock from the test is
removed (`.instance unbind all`) unless the user wants to keep it.

## Risks and open points

- **Portal position.** Picking the spot by eye can put it slightly off the
  visual. A radius-4 sphere gives some room, and it is a data-only fix.
- **Missing portal visual.** If the portal model in Val'sharah turns out to
  be a gameobject the database lacks, rather than part of the terrain,
  the trigger still works but nothing shows where it is. If so, it is
  added in this piece once its display is found (the listfile method in
  the Broken Shore notes).
- **Raid group.** The core does enforce one. `Map::PlayerCannotEnter`
  refuses a raid of the current expansion to a player outside a raid group
  (`TRANSFER_ABORT_NEED_GROUP`) unless the player is in GM mode or
  `Instance.IgnoreRaid` is on, and the server has it off. So a solo Gairy
  outside GM mode is turned away at the portal, which is retail behaviour
  and correct. Testing covers both cases. Whether to set
  `Instance.IgnoreRaid = 1` for solo play is the user's call, and it is
  not part of this piece.
