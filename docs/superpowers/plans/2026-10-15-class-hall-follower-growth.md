# Class Hall Follower Growth Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Class hall champions gain experience from successful missions and tomes, level from 100 to 110 and then rise in quality, raise their item level with armaments, and fill the equipment slots their quality unlocks; troops do not grow.

**Architecture:** The growth lives in `ClassHallFollowers` (`src/server/game/ClassHall/`), which already stores quality, level, item levels and experience. `ClassHallMissions::CompleteMission` calls it on a success; the spell effects 80, 231 and 244 call it for the champion named in the cast, after a static check in `Spell::CheckCast` like the shipments one. Equipment gets its own table `character_class_hall_follower_equipment`. The team's experience bonus, the cast layout and the quality/abilities messages are measured from the client before the code that depends on them is written.

**Tech Stack:** C++ (ArgusCore, a TrinityCore fork for 7.3.5 build 26972), MySQL update files, Docker image build on the host `172.30.1.185`, a throwaway Lua add-on in the user's client (`E:\Legion`).

**Spec:** `docs/superpowers/specs/2026-10-15-class-hall-follower-growth-design.md`

## Global Constraints

- Engine commits contain core code and schema SQL only: no content, GM commands, debug code or TODO comments.
- New code goes in new files where it is new; shared files get small hook changes only (spell effect handlers, `Spell::CheckCast`, the mission completion).
- No new compiler warnings in touched files.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` by the server's UTC date, under `sql/updates/<db>/master/`.
- Commit messages end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Every image build comes from `~/arguscore`; never pass `--no-cache`; never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while a character has `online = 1` or a client socket is open on 8085; ask the user to close the game client before a restart.
- Never read `opcode.txt` while `rotate_opcode.py` runs.
- Do not use any leak-derived core as a source. Allowed: client DB2 data, client UI source, TrinityCore upstream, WowPacketParser, Wowhead, measurements of the client.
- Test rows, items and the test add-on are planted by hand, never committed, listed in the ledger and removed at the end. The test character is put back exactly as it was.
- Never push `main`; never force-push; never open upstream pull requests or issues.
- Do not use `pkill -f` or `pgrep -f` over ssh with a pattern that appears in the command line itself.
- In-game steps go to the user a few at a time, in plain words, with no chat event printers.

## Review Focus

1. An item cast that would change nothing (cap reached, already higher, below 110, locked slot, duplicate trait, troop, champion on a mission) keeps the item. Pinned in Task 6 Step 5 and Task 7 Step 4.
2. Experience never overshoots: several levels from one gain carry the remainder; at the top quality it stops at 0. Pinned in Task 5 Step 4 (items 2 and 4).
3. A quality step re-sends the abilities so a newly unlocked equipment slot shows at once. Pinned in Task 5 Step 4 (item 3).
4. A failed mission gives no experience; troops gain none from a success. Pinned in Task 5 Step 4 (item 1).
5. Equipment survives a relog and the character dump. Pinned in Task 7 Step 4 (item 4) and Task 7 Step 5.

Not covered by any in-game test, for the final reviewer to read with care: a Horde character (faction-indexed abilities); an equipment row for a follower removed meanwhile; experience reaching a champion that is inactive; an item cast while the follower list is not loaded.

## Facts the tasks rely on

Read from the client files and the code at `engine/class-hall-missions` `22c0cfbd`:

- `GarrFollowerLevelXP` (meta `DB2Metadata.h:4502`, FileDataId 970979, ID not inline): `XpToNextLevel` u16, `ShipmentXP` u16, `FollowerLevel` u8, `GarrFollowerTypeID` u8. Type 4 rows 30-40: level 100 → 200, 101 → 400, 102 → 600, 103 → 800, 104 → 1000, 105 → 1500, 106 → 1750, 107 → 2000, 108 → 2700, 109 → 3000, 110 → 0.
- `GarrItemLevelUpgradeData` (meta `:4633`, FileDataId 1572314, ID inline at field 0): `ID`, `Operation`, `MinItemLevel`, `MaxItemLevel`, `FollowerTypeID`, all int32. Type 4 rows 12-24 as in the spec (operations 2/3 add up to MaxItemLevel 850 or 900; operations 0/1 raise to the amount).
- `GarrFollowerQualityEntry` (loaded): `XpToNextQuality`, `ShipmentXP`, `Quality`, `AbilityCount`, `TraitCount`, `GarrFollowerTypeID`, `ClassSpecID`. Type 4: q1 8000/2/0, q2 20000/3/0, q3 100000/3/1, q4 200000/3/2, q5 0/3/3, q6 0/3/3.
- Empty-slot traits (flags 0x401): 414, 415, 855, in that order in every champion's `GarrFollowerXAbility` list. `GARRISON_ABILITY_FLAG_TRAIT` = 0x1, `GARRISON_ABILITY_FLAG_IS_EMPTY_SLOT` = 0x400, single-mission duration = 0x40 (`Garrison.h:68-78`).
- Spells for tests: armaments 208142 (item 136412, +5, rows 12/13), 211997 (137208, +15, rows 12/13), 248384 (151841, raise to 850, rows 18/22), 248387 (151844, raise to 950, rows 20/21); tome 225517 (141028, 4000 XP); equipment 221914 (139792, ability 674) and 221915 (139808, ability 675).
- `ClassHallFollowers` (`ClassHallFollowers.h`): public `Follower { Entry; PacketInfo; IsActive(); }`, `GetFollower(uint64)`, `GetAllFollowers()`, `SetFollowerMission`, private `GetAbilities(GarrFollowerEntry const*, uint32 quality)`.
- The shipments cast check: `Spell.cpp:6629` calls `ClassHallShipments::CheckCreateShipmentCast(Player const*, uint32)` from `CheckCast`'s effect switch.
- `Spell::EffectAddGarrisonFollower` (`SpellEffects.cpp:5447`) is the pattern for a class hall effect handler; effects 80, 231, 244 are `EffectNULL` (lines 168, 319, 332).
- `PlayerDump.cpp:101`: `character_class_hall_follower` is `DTT_CLASS_HALL_FOLLOWER` with dbIds remapped.
- Opcodes: `SMSG_GARRISON_FOLLOWER_CHANGED_XP` 0x2912 (layout: int32 earned, int32 source, follower before, follower after; confirmed by the missions run), `..._CHANGED_ITEM_LEVEL` 0x2913, 0x2914 (core: `_CHANGED_ABILITIES`; WowPacketParser 7.3.5: `_CHANGED_QUALITY`), 0x2904 (core: `_CHANGED_DURABILITY`; WowPacketParser: `SMSG_GARRISON_UPDATE_FOLLOWER`). All `STATUS_UNHANDLED` today.
- The client fires `GARRISON_FOLLOWER_XP_CHANGED` for 0x2912 and computes `xpBonus` in `C_Garrison.GetPartyMissionInfo` (7th return).
- Baseline: DBErrors growth per boot 13,422.

## Builds and logins

| Build | Commits | Login |
|-------|---------|-------|
| — | none | P: the probe (casts, messages, xpBonus) |
| A | Tasks 2-5 | A: mission experience, levels, quality |
| B | Task 6 | B: tomes and armaments |
| C | Task 7 | C: equipment, relog |

The packet log stays on for the whole run (`PacketLogFile = "growth.pkt"`).

---

### Task 0: Workspace, branches, backups

**Files:** on the host, `~/class-hall-growth/`.

- [ ] **Step 1: Preconditions** — `~/arguscore` clean on `feature/priest-class-hall`, no build running, nobody online (same command as the missions plan's Task 0 Step 1). Expected: no changes, `feature/priest-class-hall`, `0`, `0`.
- [ ] **Step 2: Branches** — in `~/arguscore-engine` (on `engine/class-hall-missions`): `git switch -c engine/class-hall-follower-growth`. In `~/arguscore`: `git switch -c integration/class-hall-follower-growth`.
- [ ] **Step 3: Helpers** — copy `build.sh`, `boot_check.sh`, `pkt.py` (with the `PKTLEN` change), `rotate_opcode.py`, `dbdiff.py`, `edit_common.py`, `svparse.py`, `wdc1.py`, `rel.py` from `~/class-hall-missions/`, renamed for growth; add `ClassHallMissions` to the warning filter. Start `progress.md` with `# SDD ledger — plan: docs/superpowers/plans/2026-10-15-class-hall-follower-growth.md`.
- [ ] **Step 4: Back up** — `mysqldump ... characters > ~/class-hall-growth/characters_before.sql`. Ledger Gairy's currency rows and bag contents.
- [ ] **Step 5: Packet log on** — `PacketLogFile = "growth.pkt"`; ledger REVERT.

---

### Task 1: The probe (login P, no code)

**Files:** `E:\Legion\Interface\AddOns\GrowthProbe\` (toc, lua; SavedVariables `GrowthProbeDB`), never committed.

- [ ] **Step 1: Plant** — Gairy's hall (site level 560), champions 857 (q3, level 110, item level 850/850), 870 (q2, 105), 872 (q4, 110, 760/760), troops 677 x2; a creature flagged as mission NPC (`npcflag` of 106450 as in the missions run); Order Resources 1000. Ledger every row.
- [ ] **Step 2: The add-on** — on `ADVENTURE_MAP_OPEN` and `/gprobe`: for each offered mission and each team of one to three of the planted followers, `C_Garrison.AddFollowerToMission` the team and record `select(7, C_Garrison.GetPartyMissionInfo(id))` (xpBonus) with the team's abilities (`C_Garrison.GetFollowerAbilities`), then remove the team. Also record `C_Garrison.GetMissionInfo(id)` xp (2nd return) next to `GarrMission.BaseFollowerXP`.
- [ ] **Step 3: Login P** — the user opens the table and runs `/gprobe`; then, with items added by `.additem 136412 1`, `.additem 141028 1`, `.additem 139792 1`: uses each on champion 857 (the equipment one on its first slot), and tells what the client showed.
- [ ] **Step 4: Read** — the casts in `growth.pkt` (`CMSG_CAST_SPELL` or `CMSG_USE_ITEM`, values from `Opcodes.h`): where the follower dbId and the slot or ability are carried (`SpellCastRequest::Misc[2]`, the target, or elsewhere). Ledger the layout as a ruling for Task 6 and Task 7.
- [ ] **Step 5: Messages** — with `rotate_opcode.py`, hand-send 0x2914 and 0x2904 for follower 857 with quality 4, and again with a changed ability list; the add-on logs every `GARRISON_FOLLOWER_*` event and `C_Garrison.GetFollowerInfo` of 857 after each send. Ledger which opcode and layout the client takes for quality and for abilities.
- [ ] **Step 6: The bonus rule** — a `xpb.py` oracle over `GrowthProbeDB`: fit the rule on half the teams and check it on the other half (as `rule.py` did). Expected: every team matches; the rule written in the ledger in words. If no rule fits all teams, stop and ask.

---

### Task 2: Load the follower level and item level upgrade tables

**Files:**
- Create: `sql/updates/hotfixes/master/<date>_00_hotfixes.sql`
- Modify: `DB2Structure.h`, `DB2LoadInfo.h`, `DB2Stores.h`, `DB2Stores.cpp`, `HotfixDatabase.h`, `HotfixDatabase.cpp`

**Interfaces:**
- Produces: `GarrFollowerLevelXPEntry { uint32 ID; uint16 XpToNextLevel; uint16 ShipmentXP; uint8 FollowerLevel; uint8 GarrFollowerTypeID; }`, `GarrItemLevelUpgradeDataEntry { int32 ID; int32 Operation; int32 MinItemLevel; int32 MaxItemLevel; int32 FollowerTypeID; }`; `sGarrFollowerLevelXPStore`, `sGarrItemLevelUpgradeDataStore`; `HOTFIX_SEL_GARR_FOLLOWER_LEVEL_XP(_MAX_ID)`, `HOTFIX_SEL_GARR_ITEM_LEVEL_UPGRADE_DATA(_MAX_ID)`.

- [ ] **Step 1: Hotfixes update** — two tables, columns in struct order with `VerifiedBuild`, `CREATE TABLE IF NOT EXISTS` as the earlier class hall files.
- [ ] **Step 2: Stores** — modelled on `GarrFollowerQuality`.
- [ ] **Step 3: Commit** `feat(engine,sql): load the follower level and item level upgrade tables`. No build; build A covers Tasks 2-5.

---

### Task 3: Add the follower change messages

**Files:** Modify `GarrisonPackets.h/.cpp`, `Opcodes.cpp`, and `Opcodes.h` only if the probe showed a name is wrong.

**Interfaces:**
- Produces: `WorldPackets::Garrison::GarrisonFollowerChangedXp { int32 XPEarned; int32 XPSource; GarrisonFollower Before; GarrisonFollower After; }`, `GarrisonFollowerChangedItemLevel { GarrisonFollower Follower; }`, and the quality and abilities messages in the layout ledgered in Task 1 Step 5 (names follow WowPacketParser's 7.3.5 list where the probe confirmed it).

- [ ] **Step 1** — the classes and writers; `STATUS_NEVER` for the opcodes used.
- [ ] **Step 2: Commit** `feat(engine): add the follower change messages`.

---

### Task 4: Give class hall followers experience

**Files:** Modify `ClassHallFollowers.h/.cpp`.

**Interfaces:**
- Consumes: Task 2 stores, Task 3 messages.
- Produces: `void ClassHallFollowers::AddExperience(uint64 dbId, uint32 amount, uint32 source)`; `static uint32 GetLevelCap(uint32 garrFollowerTypeId)` (the `FollowerLevel` of the type's row with `XpToNextLevel` 0); `GarrFollowerQualityEntry const* GetQuality(uint32 garrFollowerTypeId, uint32 quality)` (moved out of `GetAbilities`, which now uses it).

- [ ] **Step 1: Implement** — per the spec's section 2: troops and unknown dbIds ignored; level steps then quality steps, each carrying the remainder; at a quality with no next step the experience is 0; abilities re-derived after a quality step; one `GarrisonFollowerChangedXp` with before and after, plus the quality message if Task 1 showed the client needs it. `source` is 0 for missions and 1 for items unless the client data or the UI source names values (ledger the choice).
- [ ] **Step 2: Commit** `feat(engine): give class hall followers experience`.

---

### Task 5: Give experience for a successful mission

**Files:** Modify `ClassHallMissions.h/.cpp`.

**Interfaces:**
- Consumes: `AddExperience`; the bonus rule of Task 1 Step 6.
- Produces: `uint32 ClassHallMissions::GetExperienceBonus(Mission const&, std::vector<ClassHallFollowers::Follower const*> const& team) const`.

- [ ] **Step 1: Implement** — in `CompleteMission`, on a success only, after the roll and before the reply: each champion on the mission gains `BaseFollowerXP` + `GetExperienceBonus`.
- [ ] **Step 2: Commit** `feat(engine): give experience for a successful mission`.
- [ ] **Step 3: Build A** — `build.sh`, `boot_check.sh`. Expected: warnings in touched files none, BUILD OK, READY, growth 13422, no new Server.log lines.
- [ ] **Step 4: Login A** — planted due missions (as the missions run: `startTime` back, `successChance` 100 or 0):
  1. A success with champions 870 and a troop, and a failure with 872: 870 gains base + bonus as the table showed (0x2912 in the log, XP shown in the follower tab); the troop and 872 gain nothing.
  2. 870 at level 105 with planted `xp` near the step gains enough for two levels: level 107, remainder kept.
  3. 857 (q3, 110) with planted `xp` 99,000 gains past 100,000: quality 4, a second equipment slot shown at once.
  4. A champion at quality 5 gains experience: it stays 0.
  5. Relog: level, quality and experience kept.

---

### Task 6: Use items on class hall champions (targeting, tomes, armaments)

**Files:** Modify `ClassHallFollowers.h/.cpp`, `Spell.cpp` (`CheckCast` switch), `SpellEffects.cpp` (effects 80, 231), `Spell.h` only if the target must be kept on the spell.

**Interfaces:**
- Consumes: the cast layout ruling of Task 1 Step 4; `AddExperience`.
- Produces: `static SpellCastResult ClassHallFollowers::CheckItemCast(Player const* caster, SpellEffectInfo const& effect, uint64 followerDbId, uint32 slot)`; `bool ClassHallFollowers::UpgradeItemLevel(uint64 dbId, std::vector<std::pair<GarrItemLevelUpgradeDataEntry const*, int32>> const& upgrades)` (true when a value changed); the follower dbId (and slot) available to the effect handlers.

- [ ] **Step 1: Targeting** — read the follower (and slot) where the probe found them; `CheckCast` calls `CheckItemCast` for effects 80, 231, 244 with `GarrFollowerTypeID` 4 in MiscValue2 or an upgrade row of type 4: own class hall follower, a champion, active, not on a mission, else `SPELL_FAILED_BAD_TARGETS`.
- [ ] **Step 2: Tomes** — effect 231 calls `AddExperience(dbId, amount, source)`.
- [ ] **Step 3: Armaments** — effect 80: `CheckItemCast` also refuses below the level cap and when no effect of the spell would change a value (`SPELL_FAILED_BAD_TARGETS`); the handler applies each effect by its row (operations 2/3 add up to MaxItemLevel, 0/1 raise to the amount, never lower; weapon and armor as ledgered in Task 1) and sends `GarrisonFollowerChangedItemLevel`.
- [ ] **Step 4: Commit** `feat(engine): use items on class hall champions`. Build B as Task 5 Step 3.
- [ ] **Step 5: Login B** — items planted with `.additem`:
  1. Tome on 870: +4000 XP, levels as Task 4's rule says.
  2. +5 set on 857 at 850 (cap 850): refused, item kept.
  3. +15 set on 872 at 760: 775/775.
  4. Raise-to-950 set on 872: 950/950; then the raise-to-850 set on 872: refused, item kept.
  5. A set on 870 (below 110), on a troop, on a champion on a mission: refused, items kept.

---

### Task 7: Equip class hall champions

**Files:**
- Create: `sql/updates/characters/master/<date>_00_characters.sql` (`character_class_hall_follower_equipment`: `guid` BIGINT UNSIGNED, `dbId` BIGINT UNSIGNED, `slot` TINYINT UNSIGNED, `abilityId` INT UNSIGNED; primary key `dbId`, `slot`; `IF NOT EXISTS`).
- Modify: `ClassHallFollowers.h/.cpp`, `CharacterDatabase.h/.cpp` (`CHAR_SEL/INS/DEL_CHARACTER_CLASS_HALL_FOLLOWER_EQUIPMENT`, `CHAR_DEL_CHARACTER_CLASS_HALL_FOLLOWER_EQUIPMENT_BY_FOLLOWER`), `Player.cpp`/`Player.h` (a login query slot), `PlayerDump.cpp` (a dump type remapping `dbId` like `DTT_CLASS_HALL_FOLLOWER`), `SpellEffects.cpp` (effect 244), `ClassHall.cpp` (load, delete).

**Interfaces:**
- Consumes: `CheckItemCast`, the slot from Task 1 Step 4, the abilities message of Task 3.
- Produces: `Follower::Equipment` (`std::array<uint32, 3>`, 0 = empty); `void ClassHallFollowers::LoadEquipmentFromDB(PreparedQueryResult)`; `bool ClassHallFollowers::Equip(uint64 dbId, uint32 slot, GarrAbilityEntry const* trait)`.

- [ ] **Step 1: Storage** — load (skipping rows for followers not held, slot > 2, or a non-trait ability, with one error), save with the followers, delete with the character and in `RemoveFollower`, the dump type.
- [ ] **Step 2: Abilities** — `GetAbilities` sends, for each unlocked slot (the quality's `TraitCount`), the equipped trait or that slot's empty-slot trait.
- [ ] **Step 3: Effect 244** — `CheckItemCast` refuses a single-mission ability (flag 0x40), a non-trait, a locked slot, a trait already held; the handler equips and sends the abilities message. Commit `feat(engine,sql): equip class hall champions`. Build C as Task 5 Step 3.
- [ ] **Step 4: Login C** —
  1. Equipment 139792 on 857's first slot (empty): the trait shows in the slot.
  2. 139808 on the same slot: replaces it.
  3. 139792 on 870 (q2, no slot): refused, item kept; 139808 again on 857: refused (duplicate).
  4. Relog: the equipment is kept.
- [ ] **Step 5: Dump** — console `pdump write` / `pdump load` of Gairy into a new name (as the dump fix run): the copy keeps the equipment with its follower's new dbId; then `character erase` the copy.

---

### Task 8: Land it

- [ ] **Step 1: Final review** — one fresh reviewer on the most capable model over `engine/class-hall-missions..engine/class-hall-follower-growth`, with this plan, the spec, the Review Focus and the ledger's rulings. Critical and Important findings get one fix pass, rebuilt and rechecked in game; Minor ones are ledgered as deferred.
- [ ] **Step 2: Clean up** — every planted row and item; Gesslar's `npcflag`; `PacketLogFile` empty; `growth.pkt`, `opcode.txt` removed; the add-on and its SavedVariables removed from `E:\Legion`; Gairy restored with `dbdiff.py`, only client settings differing; scratch database dropped; clean restart.
- [ ] **Step 3: Merge** — as the missions run: `dev` (worktree, `--no-ff`), the artifact branch (worktree), `feature/priest-class-hall` in `~/arguscore`; `git diff --stat integration/class-hall-follower-growth feature/priest-class-hall -- src sql` prints nothing; delete the integration branch; rebuild and boot the clean image.
- [ ] **Step 4: Docs and push** — on `dev`: the spec's status and "What the tests showed" (the bonus rule in words, the cast layout, the messages), BRANCHING.md, the README table (follower growth "In `dev`", mission extras "Next"). Merge `dev` into both priest branches. Push `engine/class-hall-follower-growth`, `dev`, `feature/priest-artifact-weapon`, `feature/priest-class-hall`.
- [ ] **Step 5: Report** — checks passed and failed, every `Ruling:` with its cost, deferred minors, the answers to the spec's open points, which branch `~/arguscore` is on; update the class hall memory file.
