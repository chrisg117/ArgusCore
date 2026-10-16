# Class Hall Mission Extras Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A class hall mission does what the client shows before Start: the chance counts every ability the team brings, threats and abilities change the cost and duration, and troops lose vitality per mission, die on an uncountered Lethal mission, can be healed, and are removed when they expire.

**Architecture:** `ClassHallMissions::GetSuccessChance` becomes `GetTeamEffects`, one pass over the team's abilities and the mission's threats that returns the chance, cost, duration and whether the troops die; start charges and stores its result, with the duration in a new column. `ClassHallFollowers` gets the vitality changes and the removal of an expired troop; `ClassHallMissions::CompleteMission` and the bonus roll call them. The stacking, rounding and countering rules, and the removal message, are measured from the client before the code that depends on them is written.

**Tech Stack:** C++ (ArgusCore, a TrinityCore fork for 7.3.5 build 26972), MySQL update files, Docker image build on the host `172.30.1.185`, a throwaway Lua add-on in the user's client (`E:\Legion`).

**Spec:** `docs/superpowers/specs/2026-10-16-class-hall-mission-extras-design.md`

## Global Constraints

- Engine commits contain core code and schema SQL only: no content, GM commands, debug code or TODO comments.
- New code goes where the code it extends lives (`ClassHallMissions`, `ClassHallFollowers`); shared files get small hook changes only.
- No new compiler warnings in touched files.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` by the server's UTC date, under `sql/updates/<db>/master/`.
- Commit messages end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Every image build comes from `~/arguscore`; never pass `--no-cache`; never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while a character has `online = 1` or a client socket is open on 8085; ask the user to close the game client before a restart.
- Do not use any leak-derived core as a source. Allowed: client DB2 data, client UI source, TrinityCore upstream, WowPacketParser, Wowhead and public guides, measurements of the client.
- Test rows, items and the test add-on are planted by hand, never committed, listed in the ledger and removed at the end. The test character is put back exactly as it was. Character rows are edited only while Gairy is offline.
- Never push `main`; never force-push; never open upstream pull requests or issues.
- Do not use `pkill -f` or `pgrep -f` over ssh with a pattern that appears in the command line itself.
- In-game steps go to the user a few at a time, in plain words, with no chat event printers.

## Review Focus

1. A team that the client shows as unaffordable (changed cost above the player's currency) is refused at start, and the currency is not touched. Pinned in Task 3 Step 5 (item 3).
2. A heal with no troop on the team does nothing, and an expired troop is not healed back (the loss comes first). Pinned in Task 6 Step 3 (item 2) and Task 5 Step 5 (item 5, no healer).
3. A successful mission's expired troop stays on the mission until the chest is opened, so the report can show it; a relog before the chest keeps it at 0 and the chest still removes it. Pinned in Task 5 Step 5 (items 2 and 3).
4. A mission row saved before this change (`missionDuration` 0) keeps its own duration and end time. Pinned in Task 2 Step 3.
5. A countered threat's cost, duration and Lethal effects do not apply, whatever the probe settles for the uncountered case. Pinned in Task 3 Step 5 (item 2) and Task 5 Step 5 (item 4).

Not covered by any in-game test, for the final reviewer to read with care: a Horde character (faction-indexed abilities); several heals on a team with one troop, and the cap at the kind's maximum (unreachable in game: the loss always comes first); a troop dismissed by the player while on a mission (refused today, check it stays refused); a mission whose currency is not the one an action 39 row names; the first-mission-of-the-day ability (Rallying Cry, flags 274), which this piece treats like the others.

## Facts the tasks rely on

Read from the client files and the code at `engine/class-hall-follower-growth` `b9217cb3`:

- `GarrAbilityEffect` fields: `CombatWeightBase` f, `CombatWeightMax` f, `ActionValueFlat` f, `ActionRecordID` i32, `GarrAbilityID`, `AbilityAction`, `AbilityTargetType`, `GarrMechanicTypeID`, `Flags`, `ActionRace`, `ActionHours`. The spec's table lists the rows for actions 6, 7, 17, 24, 25, 26, 27, 28, 33, 39, 41.
- `ClassHallMissions.cpp:48-54`: the `ABILITY_ACTION_*` enum (0, 2, 22, 23, 29); `GetSuccessChance` at `:331`; `CanStartMission` cost check at `:468`; start charges at `:493`, stores the chance at `:496`; `InitMission` sets `PacketInfo.MissionDuration` from the entry at `:95`; `CompleteMission`'s end time at `:527` uses the entry.
- `character_class_hall_mission`: `guid`, `missionId`, `dbId`, `offerTime`, `startTime`, `successChance`, `state`; dumped as `DTT_CHAR_TABLE` (`PlayerDump.cpp:103`).
- `ClassHallFollowers`: `PacketInfo.Durability` is the vitality (load `:100`, save `:150`, set from `GarrFollower.Vitality` at `:227`); `RemoveFollower(uint64)` at `:238` (refuses champions and troops on a mission, sends `GarrisonRemoveFollowerResult { FollowerDBID, GarrTypeID, Result, Destroyed }`); `SendUpdateFollower(uint64) const` at `:381`.
- Client UI (`Blizzard_GarrisonMissionTemplates.lua`, 7.3.5): `C_Garrison.GetPartyMissionInfo(id)` returns `totalTimeString, totalTimeSeconds, isMissionTimeImproved, successChance, partyBuffs, missionEffects, xpBonus, currencyMultipliers, goldMultiplier`; `C_Garrison.GetMissionCost(id)` returns `baseCost, cost`; the completion report shows a troop with `durability <= 0` as exhausted.
- Test champions (type 4) holding each ability: Flying Serpent Kick 606, Dreadsteed 619, Berserk 641, Soul Link 618, Commanding Shout 708, Enrage 714, Chenship 588 (needs Chen 596), To Me Squire! 1001 (needs 478, 479, 480 or 1000), Royal Bloodline 710, Hero of the Storm 607, Titan's Wrath 713, Agony 620, Gatekeeper 711, Leeching Poisons 891, Prayer of Mending 875.
- Equipment items: Hasty Pocketwatch 140572 (ability 754, shorter than 4 h), Potion of Energy 140571 (756), Elixir of Overwhelming Focus 140573 (755), Void-Touched Arinor Blossom 152440 (963, shorter than 8 h), Azurelight Sapphire 152936 (1000, longer than 8 h).
- Troop kinds: 622 (vitality 2), 623 (3), 625 (2), 629 (3); any other type 4 follower with `Vitality` > 0.
- Missions whose client draw (seeded with the dbId) holds the threats: Lethal 775 (dbId 2), 810 (1); Slowing 802 (1), 804 (1); Disorienting 777 (1), 786 (1); Lethal + Disorienting 1162 (1); Slowing + Lethal 907 (1). Use `rule.py`'s `mission_mechanics` to check any other choice.
- Baseline: DBErrors growth per boot 13,422.

## Builds and logins

| Build | Commits | Login |
|-------|---------|-------|
| — | none | P1, P2: the probe (chance, cost, duration; removal message) |
| A | Tasks 2-3 | A: cost and duration, stored duration |
| B | Task 4 | B: the chance actions |
| C | Tasks 5-6 | C: troops |

The packet log stays on for the whole run (`PacketLogFile = "extras.pkt"`).

---

### Task 0: Workspace, branches, backups

**Files:** on the host, `~/class-hall-extras/`.

- [ ] **Step 1: Preconditions** — `~/arguscore` clean on `feature/priest-class-hall`, no build running, nobody online. Expected: no changes, `feature/priest-class-hall`, `0`, `0`.
- [ ] **Step 2: Branches** — in `~/arguscore-engine` (on `engine/class-hall-follower-growth`): `git switch -c engine/class-hall-mission-extras`. In `~/arguscore`: `git switch -c integration/class-hall-mission-extras`.
- [ ] **Step 3: Helpers** — copy `build.sh`, `boot_check.sh`, `pkt.py`, `rotate_opcode.py`, `dbdiff.py`, `edit_common.py`, `svparse.py`, `wdc1.py`, `rel.py`, `rule.py` from `~/class-hall-growth/`, paths renamed; keep the warning filter. Start `progress.md` with `# SDD ledger — plan: docs/superpowers/plans/2026-10-16-class-hall-mission-extras.md`.
- [ ] **Step 4: Back up** — `mysqldump ... characters > ~/class-hall-extras/characters_before.sql`.
- [ ] **Step 5: Packet log on** — `PacketLogFile = "extras.pkt"`; ledger REVERT.

---

### Task 1: The probe (logins P1 and P2, no code)

**Files:** `E:\Legion\Interface\AddOns\ExtrasProbe\` (toc, lua; SavedVariables `ExtrasProbeDB`), never committed.

- [ ] **Step 1: Plant** — Gairy's hall (site level 560), the mission NPC flag on 106450, Order Resources 5000; P1 champions 606, 619, 641, 620, 607 (five active, level 110, item level 850) and troops 622, 623, 625 at full vitality and one 622 at 1; offered missions 802, 777, 775, 907, 1162 at the dbIds in Facts, plus three planted missions without threats: one under 4 h, one between 4 and 8 h, one over 8 h. Ledger every row.
- [ ] **Step 2: The add-on** — on `/xprobe`: for each offered mission and each team of one to three of the active followers (all troop mixes included), `C_Garrison.AddFollowerToMission`, record `GetPartyMissionInfo` (all nine returns), `GetMissionCost` (both), `C_Garrison.GetMissionInfo` duration and cost, the team's abilities and each troop's durability; then remove the team. Also log every `GARRISON_FOLLOWER_*` event with its arguments.
- [ ] **Step 3: Login P1** — the user opens the table and runs `/xprobe`; then equips Hasty Pocketwatch (`.additem 140572`) on 620 and Azurelight Sapphire (`.additem 152936`) on 607, and runs `/xprobe` again.
- [ ] **Step 4: Login P2** — swap the five active champions for 618, 714, 588, 596 (Chen, for Chenship) and 710 (DB edit while offline); same missions; `/xprobe`.
- [ ] **Step 5: The removal message** — with the table open, hand-send `SMSG_GARRISON_REMOVE_FOLLOWER_RESULT` for a planted troop with `Destroyed` 0 and then 1 (with `rotate_opcode.py`); the add-on logs `GARRISON_FOLLOWER_REMOVED` and the follower list after each. Ledger which value the client takes.
- [ ] **Step 6: The rules** — an `xrule.py` oracle over `ExtrasProbeDB` and `rule.py`: fit the cost, duration and new chance actions on half the teams and check them on the other half. Ledger each answer as a ruling: countering a threat removes its effects or not; multipliers multiply or add; rounding; actions 6/7 against the own or changed duration; how action 24 counts vitality; 25, 26, 33, 41 as the spec words them or otherwise. Expected: every held-out team matches. If no rule fits, stop and ask.

---

### Task 2: Store a mission's duration

**Files:**
- Create: `sql/updates/characters/master/<date>_00_characters.sql` (`ALTER TABLE character_class_hall_mission ADD COLUMN missionDuration int unsigned NOT NULL DEFAULT '0' AFTER successChance`).
- Modify: `CharacterDatabase.cpp` (the mission select and insert), `ClassHallMissions.cpp` (load, save, end time).

**Interfaces:**
- Produces: `PacketInfo.MissionDuration` holds the stored duration for a started mission; `CompleteMission` and every message use it.

- [ ] **Step 1: Implement** — load: 0 keeps `InitMission`'s duration from the entry; save writes `PacketInfo.MissionDuration` in seconds; the end time is `StartTime + TravelDuration + MissionDuration` from `PacketInfo`.
- [ ] **Step 2: Commit** `feat(engine,sql): store a class hall mission's duration`. No build; build A covers Tasks 2-3.
- [ ] **Step 3: Check in login A** (after Task 3) — a planted in-progress row with `missionDuration` 0 shows its own duration and completes at `startTime` + the entry's travel and mission duration, not earlier.

---

### Task 3: Change a mission's cost and duration by its team

**Files:** Modify `ClassHallMissions.h/.cpp`.

**Interfaces:**
- Consumes: the rulings of Task 1 Step 6 (countering, stacking, rounding).
- Produces: `struct ClassHallMissions::TeamEffects { uint32 SuccessChance = 0; uint32 Cost = 0; uint32 MissionDuration = 0; bool Lethal = false; };` and `TeamEffects GetTeamEffects(Mission const& mission, std::vector<ClassHallFollowers::Follower const*> const& team) const` replacing `GetSuccessChance`; `ABILITY_ACTION_MISSION_DURATION = 17`, `ABILITY_ACTION_MISSION_COST = 39`, `ABILITY_ACTION_KILL_TROOPS = 27`.

- [ ] **Step 1: Implement** — `GetTeamEffects` keeps the chance rule; adds duration (17) and cost (39, only rows whose `ActionRecordID` is the mission's currency) from the team's abilities and the mission's threats, threats as ruled in Task 1; sets `Lethal` for an uncountered action 27 threat as ruled.
- [ ] **Step 2: Start** — `CanStartMission` checks `GetTeamEffects(...).Cost` (it already has the team); start charges that cost and stores the chance and `MissionDuration`.
- [ ] **Step 3: Commit** `feat(engine): change a class hall mission's cost and duration by its team`.
- [ ] **Step 4: Build A** — `build.sh`, `boot_check.sh`. Expected: warnings in touched files none, BUILD OK, READY, growth 13422, no new Server.log lines.
- [ ] **Step 5: Login A** —
  1. Start 802 (Slowing) with and without a countering champion, and with Dreadsteed's 619: the currency taken and the time shown on the started mission equal the table's.
  2. Start 777 (Disorienting) countered and uncountered: the cost taken equals the table's.
  3. With Order Resources set below the doubled cost (but above the base cost): Start is refused, nothing taken.
  4. Relog: the started missions keep their changed durations; then Task 2 Step 3.

---

### Task 4: Count the rest of a team's abilities in the chance

**Files:** Modify `ClassHallMissions.cpp`.

**Interfaces:**
- Consumes: `GetTeamEffects`; the rulings of Task 1 Step 6 for actions 6, 7, 24, 25, 26, 33, 41.
- Produces: `ABILITY_ACTION_SUCCESS_CHANCE_LONG_MISSION = 6`, `_SHORT_MISSION = 7`, `_PER_TROOP_VITALITY = 24`, `_WITH_EXHAUSTED_TROOP = 25`, `_WITH_FOLLOWER = 26`, `_WITH_MIXED_TROOPS = 33`, `_ALONE = 41`.

- [ ] **Step 1: Implement** — each action adds `ActionValueFlat` under its ruled condition (spec, "The team-effects pass").
- [ ] **Step 2: Oracle** — `xrule.py` with the server's rule written in Python against all P1/P2 teams. Expected: every team matches the client's `successChance`.
- [ ] **Step 3: Commit** `feat(engine): count the rest of a team's abilities in the success chance`. Build B as Task 3 Step 4.
- [ ] **Step 4: Login B** — the user starts one mission per new action with a team the probe recorded; after logout, each stored `successChance` equals the chance the table showed.

---

### Task 5: Wear out troops on missions

**Files:** Modify `ClassHallFollowers.h/.cpp`, `ClassHallMissions.cpp` (`CompleteMission`, the bonus roll).

**Interfaces:**
- Consumes: `TeamEffects::Lethal` (recomputed at completion from the team still on the mission); the removal ruling of Task 1 Step 5.
- Produces: `void ClassHallFollowers::SetVitality(uint64 dbId, uint32 vitality)` (clamped to `[0, GarrFollower.Vitality]`, sends the whole follower); `void ClassHallFollowers::RemoveExpiredTroop(uint64 dbId)` (erases, sends the remove message as ruled); `bool ClassHallFollowers::IsExpired(Follower const&)` (a troop at 0).

- [ ] **Step 1: Implement** — at the completion roll, success or failure, each troop on the team loses 1, or all when `Lethal`; where the mission frees its followers (at once on a failure, on the bonus roll on a success), each expired troop is removed instead of being freed. Loading accepts an expired troop that is still on a completed mission and removes one that is not, with one error.
- [ ] **Step 2: Lethal at completion** — `Lethal` comes from `GetTeamEffects` over the team on the mission, so a counter on the team saves the troops.
- [ ] **Step 3: Commit** `feat(engine): wear out class hall troops on missions`.
- [ ] **Step 4** — No build yet; build C covers Tasks 5-6.
- [ ] **Step 5: Login C, troops** — planted due missions:
  1. A failure with troop 622 at 2: it ends at 1, free.
  2. A success with 622 at 1: shown exhausted in the report, still on the mission; the chest removes it.
  3. Same as 2 with a relog before the chest: still on the mission at 0; the chest removes it.
  4. Lethal 775 uncountered with 623 at 3: removed; countered: 623 at 2.
  5. A failure with 622 at 1: removed at once.

---

### Task 6: Heal troops on missions

**Files:** Modify `ClassHallMissions.cpp` (`CompleteMission`).

**Interfaces:**
- Consumes: `SetVitality`; `ABILITY_ACTION_HEAL_TROOP = 28`.

- [ ] **Step 1: Implement** — after the loss, each action 28 effect on the team with `CombatWeightBase` = `CombatWeightMax` > 0 (Light of Valor, Leeching Poisons) gives one random troop on the team that much back, capped; Pain Suppression and Combustion rows (base ≠ max) do nothing.
- [ ] **Step 2: Commit** `feat(engine): heal class hall troops on missions`. Build C as Task 3 Step 4.
- [ ] **Step 3: Login C, heals** (after Task 5 Step 5) —
  1. Leeching Poisons' 891 with troop 622 at 2: after the mission 622 is at 2 (lost 1, healed 1).
  2. 891 alone (no troop): nothing changes, no error.
  3. Relog: every troop's vitality kept.

---

### Task 7: Land it

- [ ] **Step 1: Final review** — one fresh reviewer on the most capable model over `engine/class-hall-follower-growth..engine/class-hall-mission-extras`, with this plan, the spec, the Review Focus and the ledger's rulings. Critical and Important findings get one fix pass, rebuilt and rechecked in game; Minor ones are ledgered as deferred.
- [ ] **Step 2: Clean up** — every planted row and item; the NPC flag; `PacketLogFile` empty; `extras.pkt`, `opcode.txt` removed; the add-on and its SavedVariables removed from `E:\Legion`; Gairy restored with `dbdiff.py`, only client settings differing; scratch database dropped; clean restart.
- [ ] **Step 3: Merge** — `dev` (worktree), docs commit, then `dev` into the artifact branch and `feature/priest-class-hall` (worktree); `~/arguscore` back on `feature/priest-class-hall`; delete the integration branch; rebuild and boot the clean image.
- [ ] **Step 4: Docs and push** — on `dev`: the spec's status and "What the tests showed" (the rules in words, the removal message), BRANCHING.md, the README table (mission extras "In `dev`", a row for piece 4 "Next"). Push `engine/class-hall-mission-extras`, `dev`, `feature/priest-artifact-weapon`, `feature/priest-class-hall`.
- [ ] **Step 5: Report** — checks passed and failed, every `Ruling:` with its cost, deferred minors, the answers to the spec's open points; update the class hall memory file.
