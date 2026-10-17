# Class Hall Mission Rewards Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A successful class hall mission over 100% rolls for its bonus rewards, a champion can carry a temporary ability for one mission, and a successful mission can bring back an expired troop.

**Architecture:** The bonus rewards are content in `class_hall_mission_reward` (new `Overmax` key column), loaded by `GarrisonMgr` and sent wherever the normal rewards are; `CompleteMission` rolls the bonus and keeps it in a new `bonusWon` mission column, `BonusRoll` (the chest) gives it. A temporary ability is one column on the follower row, granted through effect 244 and dropped at completion. Expired troops go into a new character table when `RemoveExpiredTroop` runs; the chest's revive rolls add a troop of the latest kind through `AddFollower`. How the client learns of a won bonus, and whether a champion can hold two temporary abilities, are measured before the code that depends on them.

**Tech Stack:** C++ (ArgusCore, a TrinityCore fork for 7.3.5 build 26972), MySQL update files, Docker image build on the host `172.30.1.185`, a throwaway Lua add-on in the user's client (`E:\Legion`).

**Spec:** `docs/superpowers/specs/2026-10-16-class-hall-mission-rewards-design.md`

## Global Constraints

- Engine commits contain core code and schema SQL only: no content, GM commands, debug code or TODO comments.
- New code goes where the code it extends lives (`ClassHallMissions`, `ClassHallFollowers`, `GarrisonMgr`'s class hall loader); shared files get small hook changes only.
- No new compiler warnings in touched files.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` by the server's UTC date, under `sql/updates/<db>/master/`.
- Commit messages end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`; write them to a file and `git commit -F` (apostrophes break the ssh quoting).
- Every image build comes from `~/arguscore`; never pass `--no-cache`; never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while a character has `online = 1` or a client socket is open on 8085; ask the user to close the game client first.
- Do not use any leak-derived core as a source. Allowed: client DB2 data, client UI source, TrinityCore upstream, WowPacketParser, Wowhead and public guides, measurements of the client.
- Test rows, items and the test add-on are planted by hand, never committed, listed in the ledger and removed at the end. Gairy is put back exactly as she was. Character rows are edited only while Gairy is offline.
- Never push `main`; never force-push; never open upstream pull requests or issues.
- No `pkill -f` / `pgrep -f` over ssh with a pattern that appears in the command line itself.
- In-game steps go to the user a few at a time, in plain words; the client serves completed missions in its own order, so no step may depend on that order.

## Review Focus

1. A mission with no bonus rewards never rolls and never sets `bonusWon`; a failed mission never has `bonusWon` set (it goes back on offer). Pinned in Task 3 Step 5 (items 2 and 3).
2. A temporary ability on a champion whose mission fails is still used up, and one granted to a champion not on any mission survives logouts. Pinned in Task 4 Step 6 (items 3 and 4).
3. A revive when the troop kind is at its limit keeps the record and adds nothing; a revive with no record does nothing. Pinned in Task 6 Step 5 (items 2 and 3).
4. A character dump and load keeps the expired-troop records and the temporary ability. Pinned in Task 6 Step 5 (item 4).
5. A bonus reward row for an unknown item or a non-class-hall mission is skipped at load with one error, and the normal rewards of that mission still load. Pinned in Task 2 Step 4.

Not covered in game, for the final reviewer: a revive of a troop kind of another class (refused by `CanAddFollower`, record kept); more than 10 expired troops (the oldest dropped); a revive and a troop expiring on the same mission (the expired one is recorded first, then may be revived); a temporary ability whose item is used while the champion is inactive; the bonus roll of a mission whose chance is over 200 (capped by piece 3).

## Facts the tasks rely on

Read from the code at `engine/class-hall-mission-extras` `239b4681` and the client files:

- `GarrisonMgr::LoadClassHallMissionRewards` (`GarrisonMgr.cpp` ~496): `SELECT MissionID, ItemID, ItemQuantity, CurrencyID, CurrencyQuantity FROM class_hall_mission_reward ORDER BY MissionID, Idx`, into `_classHallMissionRewards`; `GetClassHallMissionRewards(uint32) const` in `GarrisonMgr.h:86`. Table created in a `sql/updates/world/master/2026_10_14_*` file with key (`MissionID`, `Idx`).
- `ClassHallMissions::BuildInfo` pushes an empty `MissionOvermaxRewards` entry per mission; `SendAddMission` fills `Rewards` only (`GarrisonAddMissionResult` already has `OvermaxRewards`, written after the rewards).
- `GarrisonMission` (the packet's mission) has `MissionState`, `SuccessChance`, `Flags` (from `GarrMission.Flags` in `InitMission`).
- `ClassHallMissions::CompleteMission` decides `Succeeded`, applies troop loss and heals, sets `CLASS_HALL_MISSION_STATE_COMPLETED`; `BonusRoll` calls `GiveRewards(garrMissionId)`, frees the team, removes expired troops through `RemoveExpiredTroop(dbId, true)`, erases the mission.
- `Spell::EffectLearnFollowerAbility` (`SpellEffects.cpp`, effect 244) handles traits only; `ClassHallFollowers::CheckItemCast` refuses a non-trait 244 ability ("abilities for a single mission are not handled").
- `ClassHallFollowers::GetAbilities(entry, quality, equipment)` builds `PacketInfo.AbilityID`; `AddFollower(uint32) -> GarrisonError` checks `CanAddFollower` (class, unique, troop-kind limit) and sends the add message.
- `PlayerDump.cpp:100-105`: class hall tables; the follower table is `DTT_CLASS_HALL_FOLLOWER` (dbId remapped), the others `DTT_CHAR_TABLE`.
- `GARRISON_ABILITY_FLAG_SINGLE_MISSION_DURATION = 0x40` (`Garrison.h:74`). Temporary ability items: 139419 (662), 139428 (663), 140156 (748), 140749 (765), 140760 (776), 143605 (808), 143852 (821).
- Revive abilities (action 35, flat 50, max 1): 538 Rebirth, 566 Absolution, 624 Arise My Champion, 646 Reincarnation, 653 Resurrection. Champions holding them: 645 (538), 757 (566), 839 (624), 871 (653, a priest champion); no type 4 champion holds 646.
- Client UI: `overmaxSuccess = Clamp(successChance - 100, 0, 100)`; `C_Garrison.GetCompleteMissions(4)` entries carry `overmaxSucceeded`; `C_Garrison.CanSpellTargetFollowerIDWithAddAbility(followerID)`, `C_Garrison.TargetSpellHasFollowerTemporaryAbility()`.
- Test troops must be priest kinds (e.g. Zealot 679); a monk troop would be refused on revive.
- Planting: opening a chest deletes the mission row; a rotating mission whose `offerTime` is older than its offer duration is dropped. Baseline DBErrors growth per boot: 13,422.

## Builds and logins

| Build | Commits | Login |
|-------|---------|-------|
| — | none | P: the probe |
| A | Tasks 2-3 | A: bonus rewards and roll |
| B | Task 4 | B: temporary abilities |
| C | Tasks 5-6 | C: expired troops and revive, dump test |

The packet log stays on for the whole run (`PacketLogFile = "rewards.pkt"`).

---

### Task 0: Workspace, branches, backups

- [ ] **Step 1: Preconditions** — `~/arguscore` clean on `feature/priest-class-hall`, no build running, nobody online. Expected: no changes, the branch, `0`, `0`.
- [ ] **Step 2: Branches** — in `~/arguscore-engine` (on `engine/class-hall-mission-extras`): `git switch -c engine/class-hall-mission-rewards`. In `~/arguscore`: `git switch -c integration/class-hall-mission-rewards`.
- [ ] **Step 3: Helpers** — `~/class-hall-rewards/` with `build.sh`, `boot_check.sh` (paths and branch names renamed), `pkt.py`, `rotate_opcode.py`, `dbdiff.py`, `edit_common.py`, `svparse.py`, `wdc1.py`, `rel.py`, `rule.py`, `dberr_base.txt`, `srv_base.txt` from `~/class-hall-extras/`. Ledger `progress.md` starting `# SDD ledger — plan: docs/superpowers/plans/2026-10-16-class-hall-mission-rewards.md`.
- [ ] **Step 4: Back up** — `mysqldump characters > characters_before.sql`; `mysqldump world class_hall_mission_reward > reward_before.sql`.
- [ ] **Step 5: Packet log on** — `PacketLogFile = "rewards.pkt"` (`configs/worldserver.conf:569`); ledger REVERT.

---

### Task 1: The probe (login P, no code)

**Files:** `E:\Legion\Interface\AddOns\RewardsProbe\` (toc, lua; SavedVariables `RewardsProbeDB`), never committed.

- [ ] **Step 1: The add-on** — on `/rprobe`: record `C_Garrison.GetCompleteMissions(4)` and `GetInProgressMissions(4)` (every field); for each champion `CanSpellTargetFollowerIDWithAddAbility` and its ability list. Log every `GARRISON_*` event with its arguments (no chat output except a done line).
- [ ] **Step 2: Plant** — Gairy's hall, the mission NPC flag on 106450, three priest champions (one at 150% on a planted mission) and completed missions (state 2, start two days ago) with `MissionState` 2 on one and, through hand-sent messages, the variants of Step 3.
- [ ] **Step 3: Login P, the bonus flag** — with the table open, the user runs `/rprobe` once; then `rotate_opcode.py` sends `SMSG_GARRISON_ADD_MISSION_RESULT` for a planted completed mission with `State` 2 and, one per send, each `Mission.Flags` bit and `MissionState` values 3-6, the user running `/rprobe` after each. Ledger which, if any, makes `overmaxSucceeded` true.
- [ ] **Step 4: Login P, a second temporary ability** — hand-send `SMSG_GARRISON_UPDATE_FOLLOWER` (0x2904) for a champion with ability 662 added to its list; the user runs `/rprobe` (does `CanSpellTargetFollowerIDWithAddAbility` say yes), then uses item 139428 (`.additem 139428`) on that champion. Ledger whether the client asks the confirm question and sends the cast (packet log), or refuses by itself.
- [ ] **Step 5: Rulings** — ledger: the bonus signal (field and value, or "none: the animation stays missing, noted on issue #1"); one or several temporary abilities. If several, Task 4 uses the table form from the spec.

---

### Task 2: Load and send bonus rewards

**Files:**
- Create: `sql/updates/world/master/<date>_00_world.sql` — `ALTER TABLE class_hall_mission_reward ADD COLUMN Overmax tinyint unsigned NOT NULL DEFAULT '0' AFTER MissionID, DROP PRIMARY KEY, ADD PRIMARY KEY (MissionID, Overmax, Idx)`.
- Modify: `GarrisonMgr.h/.cpp` (loader, getter), `ClassHallMissions.cpp` (`BuildInfo`, `SendAddMission`).

**Interfaces:**
- Produces: `std::vector<WorldPackets::Garrison::GarrisonMissionReward> const* GarrisonMgr::GetClassHallMissionOvermaxRewards(uint32 garrMissionId) const` (nullptr when none), filled from rows with `Overmax = 1`.

- [ ] **Step 1: Implement** — the loader selects `Overmax` and fills `_classHallMissionOvermaxRewards`; same checks and error texts, naming the bonus reward; the load line counts both. `BuildInfo` and `SendAddMission` put the bonus rewards in the overmax lists.
- [ ] **Step 2: Commit** `feat(engine,sql): load and send class hall mission bonus rewards`. No build; build A covers Tasks 2-3.
- [ ] **Step 3: Check in login A** — Task 3 Step 5 item 1.
- [ ] **Step 4: Check at boot A** — planted rows: one bonus reward of item 0 + currency 1220 for a class hall mission, one for unknown item 1 on a class hall mission, one for a Warlords mission. Expected: two errors (bonus reward, unknown item; not a class hall mission), one bonus reward loaded, the normal rewards unchanged.

---

### Task 3: Roll for the bonus

**Files:**
- Create: `sql/updates/characters/master/<date>_00_characters.sql` — `ALTER TABLE character_class_hall_mission ADD COLUMN bonusWon tinyint unsigned NOT NULL DEFAULT '0' AFTER missionDuration`.
- Modify: `CharacterDatabase.cpp` (mission select/insert), `ClassHallMissions.h/.cpp`.

**Interfaces:**
- Consumes: `GetClassHallMissionOvermaxRewards`; Task 1's bonus signal ruling.
- Produces: `bool ClassHallMissions::Mission::BonusWon`; `void ClassHallMissions::GiveRewards(std::vector<WorldPackets::Garrison::GarrisonMissionReward> const& rewards)` (the existing body over a given list).

- [ ] **Step 1: Implement** — on a success with bonus rewards, `BonusWon = roll_chance_i(std::clamp(int32(SuccessChance) - 100, 0, 100))`; saved and loaded with the mission; the client is told as ruled (or not). At the chest the normal rewards are given, then the bonus rewards when won.
- [ ] **Step 2: Commit** `feat(engine,sql): roll for a class hall mission's bonus rewards`.
- [ ] **Step 3: Build A** — `build.sh`, `boot_check.sh`. Expected: warnings in touched files none, BUILD OK, READY, the two updates applied, growth 13,422 + the two Task 2 errors, no other new Server.log lines.
- [ ] **Step 4: Plant** — five missions in progress and due (start two days ago), each with one champion: two at chance 200 with bonus rows (bonus always), one at 100 with bonus rows (never), one at 200 without bonus rows, one at 0 with bonus rows (fails). Bonus rows give currency 1220 x 77, so a won bonus is easy to see; an offered mission with bonus rows for item 1.
- [ ] **Step 5: Login A** —
  1. An offered mission with bonus rows shows the bonus item beside "+X% bonus".
  2. The two at 200 give 77 Order Resources more than their normal reward; the one at 100 and the one without bonus rows give none; `bonusWon` is 1 only on the first two (checked in the database before the chest, after a relog).
  3. The failing one goes back on offer with `bonusWon` 0.
  4. The bonus signal, if one was ruled: the chest shows the bonus animation.

---

### Task 4: Temporary abilities

**Files:**
- Create: `sql/updates/characters/master/<date>_01_characters.sql` — `ALTER TABLE character_class_hall_follower ADD COLUMN temporaryAbilityId int unsigned NOT NULL DEFAULT '0' AFTER missionId` (or, if Task 1 ruled several, `CREATE TABLE character_class_hall_follower_temporary_ability (guid, dbId, abilityId, PRIMARY KEY (dbId, abilityId), KEY idx_guid (guid))` and a `DTT_CLASS_HALL_FOLLOWER` dump entry).
- Modify: `CharacterDatabase.cpp`, `ClassHallFollowers.h/.cpp`, `SpellEffects.cpp` (effect 244 hook), `ClassHallMissions.cpp` (`CompleteMission`).

**Interfaces:**
- Consumes: Task 1's one-or-several ruling.
- Produces: `uint32 ClassHallFollowers::Follower::TemporaryAbility` (0 none); `void ClassHallFollowers::AddTemporaryAbility(uint64 dbId, GarrAbilityEntry const* ability)`; `void ClassHallFollowers::RemoveTemporaryAbility(uint64 dbId)` (both rebuild `AbilityID` and send the whole follower).

- [ ] **Step 1: Cast check** — `CheckItemCast` accepts a 244 ability with `GARRISON_ABILITY_FLAG_SINGLE_MISSION_DURATION` on an active champion not on a mission that does not hold that ability; refuses a second temporary ability unless Task 1 ruled several (`SPELL_FAILED_GARRISON_FOLLOWER_HAS_ABILITY`).
- [ ] **Step 2: Effect and abilities** — `EffectLearnFollowerAbility` calls `AddTemporaryAbility` for such an ability; `GetAbilities` appends it after the others; save, load (drop with one error a value that is not a single-mission ability).
- [ ] **Step 3: Used up** — in `CompleteMission`, after the troop effects, each champion on the team with a temporary ability has it removed, success and failure.
- [ ] **Step 4: Commit** `feat(engine,sql): give class hall champions temporary abilities`. Build B as Task 3 Step 3 (one update applied).
- [ ] **Step 5: Plant** — two priest champions free, one planted due mission (fail) and one planted due mission (success) each needing a champion; items 139419, 139428 added by the user.
- [ ] **Step 6: Login B** —
  1. Golden Banana on champion X: the confirm question, the item is used, X shows the ability; a second Golden Banana on X is refused, the item kept.
  2. Starting a mission with X: the chance shown and stored includes the bonus (xrule check from piece 3 not needed: shown = stored).
  3. When it completes (failure), X no longer has the ability.
  4. Master Plan on champion Y (not on a mission); relog; Y still has it.

---

### Task 5: Remember expired troops

**Files:**
- Create: `sql/updates/characters/master/<date>_02_characters.sql` — `character_class_hall_expired_troop` (`guid` bigint unsigned, `followerId` int unsigned, `expireTime` bigint, PRIMARY KEY (`guid`, `expireTime`, `followerId`)).
- Modify: `CharacterDatabase.h/.cpp` (select, insert, delete-by-guid), `PlayerDump.cpp` (one `DTT_CHAR_TABLE` row), `ClassHall.cpp` (load query passed through, delete), `ClassHallFollowers.h/.cpp`.

**Interfaces:**
- Produces: `static constexpr uint32 ClassHallFollowers::MAX_EXPIRED_TROOPS = 10` (a chosen value; the retail number is not known); `std::vector<std::pair<uint32 /*garrFollowerId*/, time_t>> _expiredTroops` (oldest first); `RemoveExpiredTroop` appends to it and drops the oldest past the limit; `bool ClassHallFollowers::ReviveExpiredTroop(uint32 vitality)` (Task 6).

- [ ] **Step 1: Implement** — record on removal (at load too, when a troop at 0 is dropped); save (delete + insert), load (unknown kinds dropped with one error), delete with the class hall.
- [ ] **Step 2: Commit** `feat(engine,sql): remember expired class hall troops`. No build; build C covers Tasks 5-6.

---

### Task 6: Revive expired troops

**Files:** Modify `ClassHallFollowers.cpp` (`ReviveExpiredTroop`), `ClassHallMissions.cpp` (`BonusRoll`).

**Interfaces:**
- Consumes: `_expiredTroops`, `AddFollower`.
- Produces: `ABILITY_ACTION_REVIVE_TROOP = 35`.

- [ ] **Step 1: Implement** — `ReviveExpiredTroop(vitality)`: none recorded → false; `AddFollower(latest kind)` refused → false, record kept; taken → the new troop's vitality set (capped by its kind), the record removed, true. In `BonusRoll`, after the expired troops are removed, each action 35 effect on the team rolls `ActionValueFlat` and revives with `CombatWeightMax`.
- [ ] **Step 2: Commit** `feat(engine): revive expired class hall troops`.
- [ ] **Step 3: Build C** — as Task 3 Step 3 (one update applied).
- [ ] **Step 4: Plant** — the four champions that hold a revive ability (Facts), each on its own due mission at chance 200 with a priest troop at 1 on it (so a troop expires and is recorded on each); the revive is a 50% roll the test cannot force, so expect about half to revive (all four missing is 1 in 16: re-plant once). For item 3: a separate due mission with the revive champion after the troop kind is filled to its `FollowerClassLimit` by planted troops; for "no record": one with the records table empty and no troop on the team.
- [ ] **Step 5: Login C** —
  1. A troop expiring on a successful mission is recorded (database after logout).
  2. A revive on a success with a record brings back a troop of that kind with 1 vitality (the add message, the troop list).
  3. With that troop kind at its limit, a revive hit adds nothing and the record stays; with no record, nothing happens and no error.
  4. Console `pdump write` / `pdump load` of Gairy keeps the expired-troop records and a temporary ability (the copy erased afterwards with `character erase`).

---

### Task 7: Land it

- [ ] **Step 1: Final review** — one fresh reviewer on the most capable model over `engine/class-hall-mission-extras..engine/class-hall-mission-rewards`, with this plan, the spec, the Review Focus and the ledger's rulings. Critical and Important findings get one fix pass, each reproduced first; Minor ones are ledgered as deferred.
- [ ] **Step 2: Clean up** — planted rows and items; `class_hall_mission_reward` back to `reward_before.sql`; the NPC flag; `PacketLogFile` empty; `rewards.pkt`, `opcode.txt` removed; the add-on and its SavedVariables removed; Gairy restored with `dbdiff.py`, only client settings differing; scratch database dropped; the dumped copy erased.
- [ ] **Step 3: Merge** — `dev` (worktree), docs commit, then `dev` into the artifact branch and that into `feature/priest-class-hall` (worktree); `~/arguscore` back on `feature/priest-class-hall`; delete the integration branch; rebuild and boot the clean image.
- [ ] **Step 4: Docs and push** — on `dev`: the spec's status and "What the tests showed", BRANCHING.md, the README table (mission rewards "In `dev`"; the roadmap's first item done). Push `engine/class-hall-mission-rewards`, `dev`, `feature/priest-artifact-weapon`, `feature/priest-class-hall`.
- [ ] **Step 5: Report** — checks passed and failed, every `Ruling:` with its cost, deferred minors, the answers to the spec's open points; update the class hall memory file.
