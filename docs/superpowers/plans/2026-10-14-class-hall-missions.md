# Class Hall Missions Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A character with a class hall can open the mission table, see the missions on offer with their rewards, start one with champions and troops for its cost, and on completion learn whether it succeeded and collect its rewards, with the server rolling against the chance the client showed.

**Architecture:** A new `ClassHallMissions` class in `src/server/game/ClassHall/`, owned by `ClassHall`, with its own table `character_class_hall_mission`; a follower's current mission is a new column on `character_class_hall_follower`. Offering runs at login and from the hall's update. Rewards come from a new world table read by `GarrisonMgr`. The success chance is the client's rule, recovered from the client's own numbers with a test add-on before it is written. The Warlords `Garrison` class is not changed.

**Tech Stack:** C++ (ArgusCore, a TrinityCore fork for 7.3.5 build 26972), MySQL update files, Docker image build on the host `172.30.1.185`, a throwaway Lua add-on in the user's client (`E:\Legion`).

**Spec:** `docs/superpowers/specs/2026-10-14-class-hall-missions-design.md`

## Global Constraints

- Engine commits contain core code and schema SQL only: no content, GM commands, debug code or TODO comments.
- New code goes in new files; shared files get small hook changes only.
- No new compiler warnings in touched files.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` by the server's UTC date, under `sql/updates/<db>/master/`.
- Commit messages end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Every image build comes from `~/arguscore`; never pass `--no-cache`; never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while a character has `online = 1` or a client socket is open on 8085; ask the user to close the game client before a restart.
- Do not use any leak-derived core as a source. Allowed: client DB2 data, client UI source, TrinityCore upstream, WowPacketParser, Wowhead.
- Test rows and the test add-on are planted by hand, never committed, listed in the ledger and removed at the end. The test character is put back exactly as it was.
- Never push `main`; never force-push; never open upstream pull requests or issues.
- Do not use `pkill -f` or `pgrep -f` over ssh with a pattern that appears in the command line itself.
- In-game steps go to the user a few at a time, in plain words, with no chat event printers.

## Review Focus

1. A refused start takes nothing: no Order Resources, followers still free, mission still offered. Pinned in Task 10 Step 1 (items 3 and 4).
2. A follower on a mission cannot be sent on a second one, dismissed, or set inactive. Pinned in Task 10 Step 1 (item 5) and Task 7 Step 2.
3. A complete request before the mission's time is up changes nothing. Pinned in Task 10 Step 1 (item 6).
4. Rewards are given once: a second bonus roll, or one for a failed mission, gives nothing. Pinned in Task 10 Step 3 (items 2 and 3).
5. An offer that has run out cannot be started. Pinned in Task 10 Step 3 (item 5).

Not covered by any in-game test, for the final reviewer to read with care: a Horde character; the character dump with missions in progress; an item reward with full bags (mailed); money near the gold cap; a mission whose followers were removed by hand while it ran.

## Facts the tasks rely on

Read from the client files and the code at `engine/class-hall-shipments` `99e6861f`:

- Table layouts (WoWDBDefs, 7.3.5), each with its meta in `DB2Metadata.h`:
  - `GarrMissionXEncounter` (meta line 4802): `OrderIndex` u8, `ID`, `GarrEncounterID` u32, `GarrEncounterSetID` u32, parent `GarrMissionID`.
  - `GarrEncounter` (4347): `Name`, `CreatureID`, `UiAnimScale` f, `UiAnimHeight` f, `PortraitFileDataID`, `ID`, `UiTextureKitID`.
  - `GarrEncounterXMechanic` (4391): `ID`, `GarrMechanicID` u8, `GarrMechanicSetID` u8, parent `GarrEncounterID` u16.
  - `GarrMechanic` (4655): `ID`, `Factor` f, `GarrMechanicTypeID` u8, `GarrAbilityID`.
  - `GarrMechanicType` (4695): `Name`, `Description`, `IconFileDataID`, `Category` u8, `ID`.
  - `GarrAbilityEffect` (4185): `CombatWeightBase` f, `CombatWeightMax` f, `ActionValueFlat` f, `ActionRecordID`, parent `GarrAbilityID` u16, `AbilityAction` u8, `AbilityTargetType` u8, `GarrMechanicTypeID` u8, `Flags` u8, `ActionRace` u8, `ActionHours` u8, `ID`.
  - `GarrEncounterSetXEncounter` (4371) and `GarrMechanicSetXMechanic` (4675): set member tables.
- Class hall missions for follower type 4: 752. Categories, costs, conditions and offer times as in the spec. Type ids: 70 campaign, 71 generic, 72 treasure, 73 special reward, 74 quest, 75 dungeon treasure, 76 raid treasure, 54 generic (7.0); not offered: 11, 51, 52, 57.
- Unconditioned generic missions costing Order Resources, for tests: 800 (3 h, 3 followers, cost 20), 803 (2 h, 3, 50), 808, 817 (3 h, 3, 20), 1047 "Scouting Faronaar" (8 h, 3, 150), 1048 (4 h, 3, 100), 1055 (4 h, 3, 100).
- Opcodes: `CMSG_OPEN_MISSION_NPC` 0x32D7, `CMSG_GARRISON_START_MISSION` 0x3300, `CMSG_GARRISON_COMPLETE_MISSION` 0x3301, `CMSG_GARRISON_MISSION_BONUS_ROLL` 0x3303, `CMSG_GARRISON_GET_MISSION_REWARD` 0x3334 (not handled here); `SMSG_GARRISON_ADD_MISSION_RESULT` 0x2906, `SMSG_GARRISON_START_MISSION_RESULT` 0x2907, `SMSG_GARRISON_COMPLETE_MISSION_RESULT` 0x2909, `SMSG_GARRISON_MISSION_BONUS_ROLL_RESULT` 0x290C, `SMSG_GARRISON_OPEN_MISSION_NPC` 0x2923. All `STATUS_UNHANDLED` / `Handle_NULL` today. 7.3.5 has no "delete expired missions" message in WowPacketParser's list.
- Layouts: the spec's "Packets" table (WowPacketParser `V7_0_3_22248` and `V6_0_2_19033` garrison parsers, copies in `~/class-hall-missions/ref/`).
- `WorldPackets::Garrison::GarrisonMission`, `GarrisonMissionReward` and their writers exist (`GarrisonPackets.h:101`, `.cpp:89`); `GarrisonInfo` has `Missions`, `MissionRewards`, `MissionOvermaxRewards`, `CanStartMission`, `ArchivedMissions`.
- `ConditionMgr::IsPlayerMeetingCondition(Player const*, uint32)` returns true for an id missing from `sPlayerConditionStore`, so the offering checks the store itself first.
- `Player::HasCurrency(id, amount)`, `Player::RemoveCurrency(id, amount, CurrencyDestroyReason::Garrison)`, `Player::AddCurrency(id, amount, CurrencyGainSource::GarrisonMissionReward)`, `Player::AddItem(itemId, count)` (returns false when it does not fit), `Player::SendItemRetrievalMail(itemId, count, context)`, `Player::ModifyMoney`.
- `UNIT_NPC_FLAG_2_GARRISON_MISSION_NPC` = 0x20 of the upper half: `npcflag` bit `1 << 37` (137438953472).
- `ClassHall::Update()` (`ClassHall.cpp:108`) runs from `Player::Update` (`Player.cpp:1051`) and calls only the talents.
- `ClassHallFollowers::Follower` is private; it holds `Entry` and `PacketInfo` (with `CurrentMissionID`, `FollowerLevel`, `ItemLevelWeapon`, `ItemLevelArmor`, `Quality`, `AbilityID`, `FollowerStatus`).
- `PlayerDump.cpp`: `character_class_hall`, `_follower` (`DTT_CLASS_HALL_FOLLOWER`), `_shipment`, `_talent`.
- Priest champions planted in the followers run: 857 Ishanah (abilities 352, 458), 870 Zabra Hexx (353, 456); troops 677 Acolyte, 679 Zealot. Plant files in `~/class-hall-followers/`.
- The followers run saw a hand-sent `SMSG_GARRISON_OPEN_MISSION_NPC` not open the table; whether a real reply to `CMSG_OPEN_MISSION_NPC` does is checked in login B.
- Baseline: DBErrors growth per boot 13,422.

## Builds and logins

| Build | Commits | Login |
|-------|---------|-------|
| A | Tasks 1, 2 | A: planted missions load and save |
| B | Tasks 3, 4, 5 | B: the table opens with missions and rewards; the probe add-on measures |
| C | Tasks 6 to 9 | C1: start, chances compared; C2: complete, rewards, refusals |

The packet log stays on for the whole run (`PacketLogFile = "missions.pkt"`).

---

### Task 0: Workspace, branches, backups, data checks

**Files:** on the host, `~/class-hall-missions/` (already holds `m0.py` to `m3.py` and `ref/` from the research).

- [ ] **Step 1: Preconditions**

```bash
ssh -o BatchMode=yes chris@172.30.1.185 'cd ~/arguscore && git status --short | head -3; git branch --show-current; pgrep -x docker-buildx | wc -l; docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT COUNT(*) FROM characters.characters WHERE online=1" 2>/dev/null'
```

Expected: no changes, `feature/priest-class-hall`, `0`, `0`.

- [ ] **Step 2: Branches**

In `~/arguscore-engine` (on `engine/class-hall-shipments`): `git switch -c engine/class-hall-missions`.
In `~/arguscore`: `git switch -c integration/class-hall-missions`.

- [ ] **Step 3: Helper scripts**

Copy `build.sh`, `boot_check.sh`, `pkt.py`, `rotate_opcode.py`, `dbdiff.py`, `edit_common.py`, `dberr_base.txt`, `srv_base.txt` from `~/class-hall-shipments/`, with the directory, the integration branch and the merged engine branch renamed for missions, and `GarrisonMgr|ClassHallFollowers` in the warning filter. Start `progress.md` with `# SDD ledger — plan: docs/superpowers/plans/2026-10-14-class-hall-missions.md` and a ruling that the 13,422 baseline is reused.

- [ ] **Step 4: Back up**

```bash
docker exec arguscore-db-1 mysqldump -uarguscore -parguscore --no-tablespaces characters > ~/class-hall-missions/characters_before.sql
```

Ledger: Gairy's level, `xp`, `money`, rewarded quest count, currency rows, hall row (expected none); the `npcflag` of creature 106450.

- [ ] **Step 5: Data checks**

With a `m4.py` on the WDC1 reader: print one row of each of the eight tables of Task 1 next to its WoWDBDefs layout. Expected: the field counts match the metas, and `GarrMechanic.Factor` reads as a float (900.0 or 650.0 for class hall threats).

- [ ] **Step 6: Packet log on**

`PacketLogFile = "missions.pkt"` in `~/arguscore-server/configs/worldserver.conf`. Ledger: REVERT at cleanup.

---

### Task 1: Load the mission encounter and threat tables

**Files:**
- Create: `sql/updates/hotfixes/master/<date>_00_hotfixes.sql`
- Modify: `src/server/game/DataStores/DB2Structure.h`, `DB2LoadInfo.h`, `DB2Stores.h`, `DB2Stores.cpp`
- Modify: `src/server/database/Database/Implementation/HotfixDatabase.h`, `HotfixDatabase.cpp`

**Interfaces:**
- Produces: `GarrMissionXEncounterEntry`, `GarrEncounterEntry`, `GarrEncounterXMechanicEntry`, `GarrMechanicEntry`, `GarrMechanicTypeEntry`, `GarrAbilityEffectEntry`, `GarrEncounterSetXEncounterEntry`, `GarrMechanicSetXMechanicEntry`, with fields named and typed as in "Facts" (`ID` first); stores `sGarrMissionXEncounterStore` and the others named the same way; statements `HOTFIX_SEL_<TABLE>` (+ `_MAX_ID`, and `_LOCALE` for the two with text).

- [ ] **Step 1: Hotfixes update** — one table per store (plus `_locale` for `garr_encounter` and `garr_mechanic_type`), columns in struct order with `ID` first and `VerifiedBuild`, as `char_shipment` was written.
- [ ] **Step 2: Stores** — modelled on `CharShipment` (parent field) and `CharShipmentContainer` (text). Signs and order from the metas.
- [ ] **Step 3: Commit** `feat(engine,sql): load the mission encounter and threat tables`. No build; Task 2 shares build A.

---

### Task 2: Store class hall missions

**Files:**
- Create: `sql/updates/characters/master/<date>_00_characters.sql`
- Create: `src/server/game/ClassHall/ClassHallMissions.h`, `ClassHallMissions.cpp`
- Modify: `src/server/game/ClassHall/ClassHall.h`, `ClassHall.cpp`, `ClassHallFollowers.h`, `ClassHallFollowers.cpp`
- Modify: `CharacterDatabase.h`, `CharacterDatabase.cpp`; `Player.h` (login query slot), `Player.cpp` (load call), `CharacterHandler.cpp` (login query)
- Modify: `src/server/game/Tools/PlayerDump.cpp` (table list)

**Interfaces:**
- Consumes: `sGarrMissionStore`; `ClassHallFollowers`.
- Produces:

```cpp
class TC_GAME_API ClassHallMissions
{
public:
    ClassHallMissions(Player* owner, ClassHall* classHall);

    void LoadFromDB(PreparedQueryResult missions);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);
    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

private:
    struct Mission
    {
        GarrMissionEntry const* Entry = nullptr;
        WorldPackets::Garrison::GarrisonMission PacketInfo;
    };

    Player* _owner;
    ClassHall* _classHall;
    std::map<uint32 /*garrMissionId*/, Mission> _missions;
    uint64 _nextMissionDbId = 1;
};

// ClassHallFollowers, made public with what missions need:
struct Follower { /* as today */ };
Follower const* GetFollower(uint64 dbId) const;
void SetFollowerMission(uint64 dbId, uint32 garrMissionId);
```

- `ClassHall::LoadFromDB(classHall, talents, followers, shipments, missions)`, `ClassHall::GetMissions()` (both constnesses).
- Statements: `CHAR_SEL_CHARACTER_CLASS_HALL_MISSIONS`, `CHAR_INS_CHARACTER_CLASS_HALL_MISSION`, `CHAR_DEL_CHARACTER_CLASS_HALL_MISSIONS`; the follower select and insert gain `missionId`. Login slot `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL_MISSIONS`.

- [ ] **Step 1: Characters update** — table `character_class_hall_mission` (`guid`, `missionId`, `dbId`, `offerTime` bigint, `startTime` bigint, `successChance` int, `state` tinyint unsigned; primary key `(guid, missionId)`; no foreign key, comment as the earlier tables), and `ALTER TABLE character_class_hall_follower ADD COLUMN missionId INT UNSIGNED NOT NULL DEFAULT 0 AFTER status` (guarded so a re-run does nothing).
- [ ] **Step 2: Load, save, delete.** A row whose mission is not a class hall mission for follower type 4 is skipped with one error containing "class hall mission" (the next save drops it). `PacketInfo` holds the saved times and state; `OfferDuration`, `TravelDuration`, `MissionDuration` come from the mission row. `_nextMissionDbId` is one above the highest loaded `dbId`. After loading, every follower whose `CurrentMissionID` names no held mission in progress gets 0 and one error containing "class hall follower on a mission".
- [ ] **Step 3: `ClassHall`** owns `_missions`, forwards load (after followers), save, delete and `BuildInfo` (missions and empty reward lists, one `CanStartMission` entry each, no archived missions).
- [ ] **Step 4: Dump** — `character_class_hall_mission` as `DTT_CHAR_TABLE` after `_follower`.
- [ ] **Step 5: Commit** `feat(engine,sql): store class hall missions`.
- [ ] **Step 6: Build A** — `./build.sh > build.out 2>&1; tail -4 build.out`. Expected: `none`, `BUILD OK`.
- [ ] **Step 7: Plant and boot** — with every character offline: Gairy's hall row (1, 560, 1); followers 857 and 870 (champions, quality 2 and 3, level 110, item level 760 and 800) and two each of 677 and 679 (troops), with `dbId` above the current maximum; missions 800 (offered, now) and 1047 (in progress, started now, chance 75); follower 857 with `missionId` 1047, 870 with `missionId` 999 (stale); a mission row with `missionId` 999999. `./boot_check.sh`. Expected: READY, the characters update applied, growth 13,422.
- [ ] **Step 8: [user] Login A** — log in, `.save`, log out (three lines for the user).

Expected afterwards (database): missions 800 and 1047 unchanged; 999999 gone; Server.log has the "class hall mission" error once and the "class hall follower on a mission" error once (for 870); 870's `missionId` is 0, 857's is 1047. In the packet log, the info reply lists the two missions and 857 with `CurrentMissionID` 1047.

---

### Task 3: Add the mission packets

**Files:** `src/server/game/Server/Packets/GarrisonPackets.h`, `GarrisonPackets.cpp`, `Opcodes.cpp` (the five replies to `STATUS_NEVER`), `WorldSession.h` (forward declarations as handlers are added).

**Interfaces:**
- Produces, layouts as in the spec: `OpenMissionNpc` (CMSG: `NpcGUID`, `int32 GarrFollowerTypeID`), `GarrisonOpenMissionNpc` (SMSG, unless the followers piece already has it), `GarrisonAddMissionResult` (`GarrTypeID`, `Result`, `uint8 State`, `GarrisonMission Mission`, `std::vector<GarrisonMissionReward> Rewards`, `OvermaxRewards`, `bool CanStart`), `GarrisonStartMission` (CMSG: `NpcGUID`, `int32 GarrMissionID`, `std::vector<uint64> FollowerDBIDs`), `GarrisonStartMissionResult` (`Result`, `uint16 SessionMissionCount`, `Mission`, `std::vector<uint64> FollowerDBIDs`), `GarrisonCompleteMission` (CMSG: `NpcGUID`, `int32 GarrMissionID`), `GarrisonCompleteMissionResult` (`Result`, `Mission`, `int32 GarrMissionID`, followers as `{uint64 DBID; uint32 Flags;}`, `bool Succeeded`), `GarrisonMissionBonusRoll` (CMSG: `NpcGUID`, `uint32 GarrMissionID`), `GarrisonMissionBonusRollResult` (`Mission`, `int32 GarrMissionID`, `int32 Result`).

- [ ] **Step 1: Write the classes** — the mission entry writer and reward writer already exist; reuse them.
- [ ] **Step 2: Commit** `feat(engine): add the mission packets`. No build; build B.

---

### Task 4: Offer class hall missions

**Files:**
- Create: `sql/updates/world/master/<date>_00_world.sql` (table `class_hall_mission_reward`, empty)
- Modify: `ClassHallMissions.h/.cpp`, `ClassHall.cpp` (`Update`, login offering), `src/server/game/Garrison/GarrisonMgr.h/.cpp` (load and look up rewards)

**Interfaces:**
- Produces:

```cpp
// ClassHallMissions
// how many rotating missions the table holds at once; no source I may use states the retail number
static constexpr uint32 ROTATING_MISSIONS_ON_OFFER = 10;
void Update();               // offering, at most once a minute
void OfferMissions(bool notify);
static bool IsOfferable(GarrMissionEntry const* mission); // class hall, follower type 4, not types 11/51/52/57

// GarrisonMgr
std::vector<WorldPackets::Garrison::GarrisonMissionReward> const* GetClassHallMissionRewards(uint32 garrMissionId) const;
```

- [ ] **Step 1: World update** — `class_hall_mission_reward` (`MissionID`, `Idx`, `ItemID`, `ItemQuantity`, `CurrencyID`, `CurrencyQuantity`, `VerifiedBuild`; primary key `(MissionID, Idx)`).
- [ ] **Step 2: Load the rewards** in `GarrisonMgr::Initialize`: a row with an unknown mission, item or currency is skipped with an error naming the table. Currency 0 is money in copper.
- [ ] **Step 3: Offering**, as spec section 4: expire offered missions past `OfferTime + OfferDuration` (removed, no packet); add the no-offer-time missions whose condition is met; fill the rotation to `ROTATING_MISSIONS_ON_OFFER` with random eligible missions. A condition is met only if `sPlayerConditionStore` has it and `ConditionMgr::IsPlayerMeetingCondition` says yes. With `notify`, each addition sends `GarrisonAddMissionResult` (state 0, rewards, `CanStart` true). Login offers without notify, before the info reply is built; `Update` offers with notify. Mission entries are sent with their rewards in `BuildInfo` too.
- [ ] **Step 4: Commit** `feat(engine,sql): offer class hall missions`.

---

### Task 5: Open the mission table, and probe the client

**Files:** `ClassHallMissions.h/.cpp`, `GarrisonHandler.cpp`, `Opcodes.cpp`, `WorldSession.h`; the test add-on `E:\Legion\Interface\AddOns\MissionProbe\` (never committed).

**Interfaces:**
- Produces: `void ClassHallMissions::OpenNpc(ObjectGuid npcGuid)`; private `ObjectGuid _openNpc`; `WorldSession::HandleOpenMissionNpc` (checks `GetNPCIfCanInteractWith(guid, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG_2_GARRISON_MISSION_NPC)` and a class hall).

- [ ] **Step 1: Handler and reply** as spec section 5. Commit `feat(engine): open the mission table`.
- [ ] **Step 2: Build B**, then with everyone offline: Grand Anchorite Gesslar's (106450) `npcflag` gets `| 137438953472`; reward rows for 800 (item 2589 x5), 803 (currency 1220 x100), 808 (currency 0 x100000), and offered mission rows for 803 and 808 (800 is already offered) so the rewarded missions are on the table; `boot_check.sh`. Expected: growth 13,422 (the reward rows name a real item and currencies).
- [ ] **Step 3: Write the probe add-on** (`MissionProbe.toc`, `MissionProbe.lua`, `## SavedVariables: MissionProbeDB`). `/mprobe` does, for follower type 4: for every available mission, record `C_Garrison.GetMissionInfo`, `GetBasicMissionInfo` and the enemies with their mechanics; for every team in the list below, add each follower with `C_Garrison.AddFollowerToMission`, record `GetMissionSuccessChance` and `GetPartyMissionInfo`, then remove them. Teams: every single follower, every pair, and every triple of the planted followers (6 followers: 41 teams per mission). `/mprobe` prints one line when done. Ledger: REVERT the add-on folder and `WTF\...\SavedVariables\MissionProbe.lua` at cleanup.
- [ ] **Step 4: [user] Login B** (four short steps for the user):
  1. `.npc add temp 106450`, then click him. Expected: the mission table opens with missions and the planted rewards.
  2. Note how many missions are listed and send a screenshot.
  3. `/mprobe`, wait for "done".
  4. `/reload`, then log out.

Expected afterwards: `MissionProbe.lua` in the client's SavedVariables holds results for every listed mission; the packet log has `CMSG_OPEN_MISSION_NPC` and the reply; whether the client sent 0x3334 is recorded.

- [ ] **Step 5: Read the probe** — copy `MissionProbe.lua` to the host. Ledger: (a) whether set-based missions show encounters, and whether two characters' (or two offers') encounters differ; (b) whether a team smaller than `MaxFollowers` gets a chance at all (the slot rule); (c) the measured table. If (a) shows the client draws the sets itself in a way the server cannot reproduce, stop and ask the user (a plan-breaking finding).

---

### Task 6: Work out a mission's success chance

**Files:** `ClassHallMissions.h/.cpp`; on the host `~/class-hall-missions/rule.py` (the oracle, never committed).

**Interfaces:**
- Produces: `uint32 ClassHallMissions::GetSuccessChance(GarrMissionEntry const* mission, std::vector<ClassHallFollowers::Follower const*> const& team) const;` and whatever private helpers the rule needs.

- [ ] **Step 1: Write the oracle first** — `rule.py` reads the probe file and the client tables and must reproduce every measured chance of half the teams (alternate teams). Run: `python3 rule.py fit`. Expected: `mismatches: 0` for the fitted half. Ledger the rule in words.
- [ ] **Step 2: Check the oracle** — `python3 rule.py check` on the other half. Expected: `mismatches: 0`. A mismatch sends the rule back to Step 1; it is never patched for one team.
- [ ] **Step 3: Port the rule to C++** — same inputs: the mission's threats, each follower's abilities, level, item levels, quality and troop status, the mission's target level and item level, base chance.
- [ ] **Step 4: Commit** `feat(engine): work out a mission's success chance`. No build; build C. The C++ is checked against the client in login C1.

---

### Task 7: Start a class hall mission

**Files:** `ClassHallMissions.h/.cpp`, `ClassHallFollowers.cpp`, `GarrisonHandler.cpp`, `Opcodes.cpp`, `WorldSession.h`.

**Interfaces:**
- Produces: `void ClassHallMissions::StartMission(ObjectGuid npcGuid, uint32 garrMissionId, std::vector<uint64> const& followerDbIds);` private `GarrisonError CanStartMission(Mission const& mission, std::vector<ClassHallFollowers::Follower const*> const& team) const;` and `uint16 _sessionMissionCount`.

- [ ] **Step 1: Start**, as spec section 6, with the slot rule from Task 5 Step 5. On success: `RemoveCurrency(mission cost, CurrencyDestroyReason::Garrison)`; chance from Task 6 stored in `PacketInfo.SuccessChance`; state 1; `StartTime` now; `SetFollowerMission` for each; reply result 0 with the mission and follower ids. On a refusal: the reply with the error and nothing changed.
- [ ] **Step 2: Followers on a mission** — `ClassHallFollowers::RemoveFollower` and `SetFollowerInactive` refuse a follower whose `CurrentMissionID` is not 0, with `GARRISON_ERROR_FOLLOWER_ALREADY_ON_MISSION`, in the reply each already sends.
- [ ] **Step 3: Commit** `feat(engine): start a class hall mission`.

---

### Task 8: Complete a class hall mission

**Files:** `ClassHallMissions.h/.cpp`, `GarrisonHandler.cpp`, `Opcodes.cpp`, `WorldSession.h`.

**Interfaces:**
- Produces: `void ClassHallMissions::CompleteMission(ObjectGuid npcGuid, uint32 garrMissionId);`

- [ ] **Step 1: Complete**, as spec section 8: only in progress and past `StartTime + TravelDuration + MissionDuration`; otherwise the reply with `GARRISON_ERROR_INVALID_MISSION_TIME` and nothing changed. Roll: `roll_chance_i(SuccessChance)` with 100 and above certain. Followers back to mission 0 (their flags 0 in the reply). Failure: the mission is removed. Success: state 2.
- [ ] **Step 2: Commit** `feat(engine): complete a class hall mission`.

---

### Task 9: Reward a class hall mission

**Files:** `ClassHallMissions.h/.cpp`, `GarrisonHandler.cpp`, `Opcodes.cpp`, `WorldSession.h`.

**Interfaces:**
- Produces: `void ClassHallMissions::BonusRoll(ObjectGuid npcGuid, uint32 garrMissionId);` private `void GiveRewards(uint32 garrMissionId);`

- [ ] **Step 1: Bonus roll** — only for a held mission in state 2: give each reward (item with `AddItem`, mailed with `SendItemRetrievalMail` when it does not fit; currency with `AddCurrency(..., CurrencyGainSource::GarrisonMissionReward)`; currency 0 with `ModifyMoney`), remove the mission, reply result 0. Anything else: nothing given; the reply with `GARRISON_ERROR_INVALID_MISSION` if the mission is held, nothing if not.
- [ ] **Step 2: Commit** `feat(engine,sql): reward a class hall mission` if it touches SQL, `feat(engine)` otherwise.
- [ ] **Step 3: Build C** and boot, everyone offline.

---

### Task 10: Test the loop in game

State before login C1: Gairy's hall, the planted followers all free (missions 800 and 1047 rows removed), Order Resources 500, the mission NPC flag and reward rows of Task 5. Order Resources are set by SQL while she is offline, not with `.modify currency` (it adds).

- [ ] **Step 1: [user] Login C1** (a few steps at a time):
  1. Open the table at Gesslar. For three missions, put a team on, note the chance the table shows, and start it. (Three screenshots.)
  2. Expected: Order Resources fall by each cost; the followers show "On Mission".
  3. Try to start a fourth mission with a follower already out. Expected: refused, nothing taken.
  4. Spend Order Resources down below the next mission's cost (`.modify currency 1220 -<n>` with the number I give), try to start it. Expected: refused, nothing taken.
  5. Try to dismiss a follower that is on a mission. Expected: refused.
  6. On a started mission, run `/run C_Garrison.MarkMissionComplete(<id>)` (I give the id). Expected: nothing changes.
  7. Log out.

Expected afterwards: in the packet log, each start reply's `SuccessChance` equals the chance the user saw; the rows in `character_class_hall_mission` and the followers' `missionId` match.

- [ ] **Step 2: Age the missions** — with Gairy offline, set `startTime` back by the longest mission time; set one mission's `successChance` to 0 and one to 100; set one offered mission's `offerTime` back past its offer time.

- [ ] **Step 3: [user] Login C2:**
  1. Open the table: the three missions are complete. Click through each. Expected: the 100 one succeeds and its chest gives the planted reward; the 0 one fails and gives nothing; followers are free afterwards.
  2. After the chest, run `/run C_Garrison.MissionBonusRoll(<id>)` again (id given). Expected: nothing more is given.
  3. `/run C_Garrison.MissionBonusRoll(<failed id>)`. Expected: nothing.
  4. A full-bag case is not tested in game (Review Focus note).
  5. The expired offer is gone; `/run C_Garrison.StartMission(<expired id>)` after adding a follower changes nothing.
  6. Log out, close the client.

---

### Task 11: Land it

- [ ] **Step 1: Final review** — one fresh reviewer on the most capable model over `engine/class-hall-shipments..engine/class-hall-missions`, with this plan, the spec, the Review Focus and the ledger's rulings. Critical and Important findings get one fix pass, each rebuilt and rechecked in game; Minor ones are ledgered as deferred.
- [ ] **Step 2: Clean up** — every planted row (Gairy's hall, followers, missions; Gesslar's `npcflag`; the reward rows), `PacketLogFile` back to empty, `missions.pkt` and `opcode.txt` removed, the add-on folder and its SavedVariables file removed from `E:\Legion`; Gairy restored from the backup with `dbdiff.py`, only client settings and the server-wide rotation tables differing; scratch database dropped; clean restart.
- [ ] **Step 3: Merge** — as the shipments run: `dev` (worktree), the artifact branch (worktree), `feature/priest-class-hall` in `~/arguscore`; `git diff --stat integration/class-hall-missions feature/priest-class-hall -- src sql` prints nothing; delete the integration branch.
- [ ] **Step 4: Docs and push** — on `dev`: the spec's status and "What the tests showed" (with the success rule in words and the probe's answers), BRANCHING.md's branch list, and the README's class hall table (missions "In `dev`", follower growth "Next"). Merge `dev` into both priest branches. Push `engine/class-hall-missions`, `dev`, `feature/priest-artifact-weapon`, `feature/priest-class-hall`.
- [ ] **Step 5: Report** — checks passed and failed, every `Ruling:` with its cost, deferred minors, the answers to the spec's open points, which branch `~/arguscore` is on; update the class hall memory file.
