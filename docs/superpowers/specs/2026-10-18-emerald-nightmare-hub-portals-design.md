# Emerald Nightmare hub portals: reaching the wings

Date: 2026-10-18
Branch: `dev` (content goes on `dev` directly, rule 3 of `docs/BRANCHING.md`)
Status: design approved by the user 2026-10-18, written spec awaiting review

## Goal

After Nythendra, a player walks into a portal in the hub above her room and
arrives in the matching wing of the raid (Un'goro, Thunder Bluff, Grizzly
Hills, The Emerald Dreamway). In each wing a return portal leads back to the
hub. Today none of this is spawned, so the only way into the Un'goro room is
`.go xyz`.

## Decisions taken while drafting

- **Gating (user chose option C, 2026-10-18).** Retail gating, plus a GM
  bypass. Per Icy Veins and Wowhead, Nythendra is the only gate: once she is
  dead every wing is reachable, in any order. Cenarius and Xavius are not
  behind hub portals and are out of scope. Before Nythendra dies a portal
  refuses everyone except a player in GM mode.
- **Mechanism (user chose option A).** Walk-in server-side areatriggers, as
  the scaffold's entrance and exit. Clicking a portal object was rejected: the
  portal objects have no spell (`Data0` 0) and would need one script each.
- **Return portals (user chose option A).** One per wing, ungated, plain
  teleport action. Without them Il'gynoth's room has no way out.
- Mechanics follow retail, Normal first, solo via GM commands (standing rules).

## What exists today

Checked 2026-10-18 on `dev`, the server world database and the client DB2
files for build 7.3.5.26972.

- Portal templates, all with zero spawns: `gameobject_template` 254270
  "Un'goro Crater", 254271 "Thunder Bluff", 254272 "Grizzly Hills", 254273
  "The Emerald Dreamway" (type 5, displays 36807-36810, size 4). Other "Portal"
  goobers (251995, 251997, 252004, 252006 on display 36949; 252003, 253174 on
  35705) are not used here.
- The scaffold's triggers: `areatrigger_template` / `areatrigger_create_properties`
  152001 and 152002, spawn 1520001 on map 1520, and `gameobject` guids
  1520001-1520003 are taken.
- Instance script `instance_emerald_nightmare` exposes `GetBossState(DATA_NYTHENDRA)`.
- `AreaTriggerAI::OnUnitEnter(Unit*)` exists; `RegisterAreaTriggerAI` registers
  a script by class name; `areatrigger_create_properties.ScriptName` and
  `areatrigger.ScriptName` attach it.
- `areatrigger_template_actions` action 2 (`AREATRIGGER_ACTION_TELEPORT`) with a
  WorldSafeLocs id teleports without a script. It cannot test boss state, so
  it only suits the ungated return triggers.

WorldSafeLocs on map 1520 (x, y, z), from `WorldSafeLocs.db2`:

| Id | Position | Likely use |
|----|----------|------------|
| 5681 | (1871.0, 1350.1, 597.9) | hub ring centre |
| 5677 | (1831.6, 1328.8, 597.5) | hub ring spot |
| 5678 | (1852.9, 1280.7, 598.5) | hub ring spot |
| 5679 | (1905.5, 1303.8, 601.9) | hub ring spot |
| 5680 | (1948.0, 1350.2, 581.0) | hub ring spot |
| 5447 | (1863.9, 1373.3, 598.1) | Nythendra-area, hub side |
| 5461, 5577 | (-13148.0, 12096.9, -256.3), (-13151.1, 12102.3, -255.7) | Un'goro arrival |
| 5575, 5688 | (11334.4, 11055.4, 13.7), (11316.6, 11054.4, 8.1) | a wing, to identify |
| 5576, 5689 | (11196.4, -12614.2, 549.7), (11199.8, -12619.1, 548.7) | a wing, to identify |
| 5574, 5691 | (-12334.5, -12761.4, 329.4), (-12372.2, -12957.0, 328.9) | a wing, to identify |
| 5523-5526, 5578, 5690 | around (521..686, -12750..-12910) | possibly the Dreamway |

Only Un'goro is identified (it matches DungeonMap floor 4). The other mappings
are guesses from coordinates and are confirmed in game (see Testing).

## Scope

In scope:

1. Four hub portal gameobject spawns (visual), one per wing.
2. Four gated hub triggers driven by a new `AreaTriggerAI` script.
3. Four ungated return triggers (action-2 teleport).
4. One SQL file, one script file, one loader line.

Out of scope: portal opening effects or a visual change after Nythendra dies;
Cenarius and Xavius routes; content inside the other wings (empty rooms until
their boss specs land); Heroic/Mythic/LFR differences.

## Design

### Script: `at_en_hub_portal`

New file `src/server/scripts/BrokenIsles/EmeraldNightmare/areatrigger_emerald_nightmare.cpp`,
registered by its own `AddSC_areatrigger_emerald_nightmare()`, declared and
called from `broken_isles_script_loader.cpp` in the Emerald Nightmare block.

- `struct at_en_hub_portal : AreaTriggerAI`, with `OnUnitEnter(Unit* unit)`.
- Ignore anything that is not a player.
- Allowed if `player->IsGameMaster()` or the instance's
  `GetBossState(DATA_NYTHENDRA) == DONE`.
- If allowed, `player->TeleportTo` the destination from a table keyed by the
  create-properties id (position read from the WorldSafeLocs entry by id, so
  the Id lives in one table in the script, not hard-coded coordinates).
- If refused, send the player a short system message ("The portal does not
  respond.") and throttle it so standing in the trigger does not spam. A
  per-player timestamp in the AI is enough.
- Destination wing is determined by `at->GetCreateProperties()->Id`, not by
  trigger position, so the SQL and script share one key.

### Database

One file: `sql/updates/world/master/2026_10_18_NN_world.sql` (next free
number), safe to re-run (`DELETE` then `INSERT`, comment on each block).

Ids, free on 2026-10-18 and rechecked when the SQL is written (other branches
may have taken them):

| Row | Ids |
|-----|-----|
| Hub triggers, template + create properties | 152003-152006 (Un'goro, Thunder Bluff, Grizzly Hills, Emerald Dreamway) |
| Return triggers, template + create properties | 152007-152010, same wing order |
| `areatrigger` spawns, hub | 1520002-1520005 |
| `areatrigger` spawns, return | 1520006-1520009 |
| `gameobject` spawns, portals | 1520004-1520007 for 254270-254273 |

- Hub triggers: `areatrigger_create_properties` with `ScriptName
  'at_en_hub_portal'`, sphere (shape 0) radius 4, `IsCustom = 1`; a matching
  `areatrigger_template` (`IsCustom = 1`, `Flags = 1`) with no actions;
  `areatrigger` spawns on map 1520 with `SpawnDifficulties '14,15,16,17'` and
  `IsCustom = 1`.
- Return triggers: same shape, but `areatrigger_template_actions` with
  `ActionType = 2`, `ActionParam` = the hub arrival WorldSafeLocs id for that
  wing, `TargetType = 5`. Spawned on map 1520 inside each wing room.
- Portal visuals: `gameobject` rows for 254270-254273 on map 1520, difficulties
  `14,15,16,17`, rotation facing the hub centre, placed under each hub trigger.
- Positions are placeholders until measured in game. The SQL is first written
  with the ring points above (hub) and arrival points (return), then adjusted
  with `.gps`. The arrival points sit outside the triggers by design, so a
  player is never teleported straight back.

### Return trip destinations

Each return trigger sends the player to the hub spot of the wing it leaves
(the ring spot nearest that wing's hub portal), not the ring centre, so the
player lands beside the portal they used. If this puts the player inside the
hub trigger, move the arrival point (data-only fix).

## Testing

Build in `~/arguscore` after asking the session that holds it (rules 8-10 of
`BRANCHING.md`); no `--no-cache`; no restart while a client is connected. SQL
needs an image rebuild (`~/emerald-nightmare/build.sh`, then `boot_check.sh`;
`DBErrors` baseline 13422).

1. **Boot.** No "script not found" for `at_en_hub_portal`; `DBErrors.log` growth
   within baseline.
2. **Hub visuals.** Gairy sees the four portal objects at the hub ring.
3. **Locked.** `.gm off` with Nythendra not dead: each portal refuses with the
   message and does not teleport.
4. **GM bypass.** `.gm on`: each portal teleports to its wing.
5. **Open.** `.gm off`, Nythendra dead (killed, or `.instance setbossstate 0 3`):
   each portal teleports.
6. **Wing identification.** On arrival at each wing, `.gps` and note which
   room it is; fix the Thunder Bluff / Grizzly Hills / Dreamway mapping if
   the guesses in the table above are wrong.
7. **Return.** Each return trigger lands at the hub and does not bounce.
8. **Cleanup.** `.instance unbind all` afterwards unless the user wants the
   lock kept.

## Risks and open points

- **Wing mapping.** Only Un'goro is verified; the other three arrival points are
  guesses until step 6. A wrong guess is a data-only fix.
- **Position by eye.** Portal and trigger positions come from `.gps` in game
  (the user's job); the radius-4 sphere may need adjusting.
- **Shared spawn on difficulties.** The portal objects are visible in all four
  difficulties, matching the scaffold's triggers. Heroic/Mythic/LFR behaviour
  is not tested here.
- **Id collisions.** Other feature branches may have taken the listed ids; the
  SQL step rechecks them.
