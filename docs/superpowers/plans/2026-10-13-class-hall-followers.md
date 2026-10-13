# Class Hall Followers Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A character with a class hall can gain champions and troops, keep them across logins, see them in the client's follower list, set a champion active or inactive, and dismiss a follower.

**Architecture:** A new `ClassHallFollowers` class in `src/server/game/ClassHall/`, owned by `ClassHall`, with its own table `character_class_hall_follower`. The grant spell (effect 220) is routed to it for class hall followers. Three client requests get handlers in `GarrisonHandler.cpp`. The Warlords `Garrison` class is not changed.

**Tech Stack:** C++ (ArgusCore, a TrinityCore fork for 7.3.5 build 26972), MySQL update files, Docker image build on the host `172.30.1.185`.

**Spec:** `docs/superpowers/specs/2026-10-13-class-hall-followers-design.md`

## Global Constraints

- Engine commits contain core code and schema SQL only: no content, GM commands, debug code or TODO comments.
- New code goes in new files; shared files get small hook changes only.
- No new compiler warnings in touched files.
- SQL files are named `YYYY_MM_DD_NN_<db>.sql` by the server's UTC date, under `sql/updates/<db>/master/`.
- Commit messages end with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- Every image build comes from `~/arguscore`; never pass `--no-cache`; never start a build while `pgrep -x docker-buildx` prints a process.
- Never restart the world server while a character has `online = 1`; ask the user to close the game client before a restart.
- Do not use LegionCore or any other leak-derived core as a source. Allowed: client DB2 data, client UI source, TrinityCore upstream, WowPacketParser, Wowhead.
- Test rows are planted by hand, never committed, listed in the ledger and removed at the end. The test character is put back exactly as it was.
- Never push `main`; never force-push; never open upstream pull requests or issues.
- Do not use `pkill -f` or `pgrep -f` over ssh with a pattern that appears in the command line itself.

## Review Focus

1. A refused grant, activation or dismissal changes nothing: no row, no gold taken, no activation used. Pinned in Task 4 Step 5 and Task 5 Step 5.
2. A request naming a follower id the character does not own (0, an unknown id) is refused and does not crash. Pinned in Task 5 Step 5 and Task 6 Step 3.
3. Saved followers whose hall row is missing do not load and do not block login. Pinned in Task 2 Step 7.
4. Two troops with the same follower id coexist, each with its own id, and dismissing one leaves the other. Pinned in Task 2 Step 5 (planted pair) and Task 6 Step 2.
5. After a restart the id generator never hands out an id that either follower table already holds. Pinned in Task 4 Step 4 (the new row's `dbId` is above every planted one).

Not covered by any in-game test, for the final reviewer to read with care: the Horde ability rows (faction index 0), the unchanged Warlords path of effect 220, and the daily reset of activations (needs a day change).

## Facts the tasks rely on

Read from the client files and the code at `engine/class-hall-talents` `e87f7ab8`:

- Priest followers: champions 856 (Calia, spell 219756), 857 (Ishanah, spell 219757), 870 to 875, 1002; Acolytes (class spec 91, limit 3) 677, 678, 921 to 926; Zealots (class spec 92, limit 2) 679, 920, 927 to 932, 943; Hooded Priests (class spec 155, limit 2) 1039 to 1042.
- `GarrFollower`: champions have `Vitality` 0 and `Flags` 3; troops `Vitality` 2 or 3 and `Flags` 2. `GARRISON_FOLLOWER_FLAG_UNIQUE` is 0x1.
- `GarrFollowerType` row 4: `MaxItemLevel` 950, `MaxFollowers` 5, `GarrTypeID` 3.
- `GarrFollowerQuality`, follower type 4: quality 1 → 2 abilities, 0 traits; 2 → 3, 0; 3 → 3, 1; 4 → 3, 2; 5 and 6 → 3, 3.
- `GarrFollowerXAbility` for 856, per faction: 354 Discipline, 457 Holy Nova, 777 Psychic Scream, then traits 414, 415, 855 (flag 0x1 `GARRISON_ABILITY_FLAG_TRAIT`). Faction index 0 is Horde, 1 Alliance.
- `GarrClassSpecPlayerCond`: 59 (spec 91, condition 42849, order 0), 60 (91, 41906, 1), 61 (92, 41907, 0), 62 (92, 41908, 1), 75 (155, 48410, 0).
- Follower status flags: inactive 0x04, troop 0x08.
- Existing packets: `GarrisonFollower` (`operator<<` in `GarrisonPackets.cpp:63`), `GarrisonAddFollowerResult` (GarrTypeID, Result, Follower), `GarrisonRemoveFollowerResult` (FollowerDBID, GarrTypeID, Result, Destroyed), `GarrisonInfo::Followers` (`std::vector<GarrisonFollower const*>`) and `GarrisonInfo::NumFollowerActivationsRemaining`.
- Opcode numbers: `CMSG_GARRISON_SET_FOLLOWER_INACTIVE` 0x32C5, `CMSG_GARRISON_REMOVE_FOLLOWER` 0x32F8, `CMSG_GARRISON_REQUEST_CLASS_SPEC_CATEGORY_INFO` 0x32D5, `SMSG_GARRISON_FOLLOWER_CHANGED_DURABILITY` 0x2904 (WowPacketParser: "update follower"), `SMSG_GARRISON_FOLLOWER_CHANGED_STATUS` 0x2915, `SMSG_GARRISON_NUM_FOLLOWER_ACTIVATIONS_REMAINING` 0x2917. Not defined yet: 0x2901 (WowPacketParser: "get class spec category info result").
- Layouts from WowPacketParser: set inactive = `uint64 FollowerDBID`, bit `Inactive` (6.x); class spec request = `uint32 GarrFollowerTypeID`; 0x2904 = `uint32 Result`, follower; 0x2917 = `uint32` site or type, `uint32` count; 0x2901 = `int32 GarrFollowerTypeID`, `uint32` count, then per entry `int32 GarrClassSpecID`, `int32 GarrClassSpecPlayerCondID`. No parser: the dismiss request and 0x2915.
- Garrison errors: no garrison 1, follower exists 27, invalid follower 28, not enough gold 47, follower inactive 69, follower active 70, follower activation unavailable 71, at follower hard cap 78, at class spec cap 81, invalid class 88.
- `GarrisonMgr::InitializeDbIdSequences` (`GarrisonMgr.cpp:351`) starts the generator from `MAX(dbId)` of `character_garrison_followers`.
- `Player::ResetDailyQuestStatus` (`Player.cpp:24410`) resets the Warlords garrison's activation count.
- The recruit criteria (type 176) compares its asset with the follower id passed as the first value.
- Baseline: DBErrors growth per boot 13,422.

## Builds and logins

Three long builds (about 40 minutes each) and the logins they serve:

| Build | Commits | Login |
|-------|---------|-------|
| A | Task 1, Task 2 | A: planted followers, discoveries for Tasks 3 to 6 |
| B | Tasks 3, 4, 5, 6 | B1: gain and activate; B2: limit, cost, dismiss, troop reply probe |
| C | Task 7 | C: troop kinds |

The packet log stays on for the whole run (`PacketLogFile = "followers.pkt"`).

---

### Task 0: Workspace, branches, backup and data checks

**Files:**
- Create on the host: `~/class-hall-followers/` with `build.sh`, `boot_check.sh`, `pkt.py`, `rotate_opcode.py`, `progress.md`, `dberr_base.txt`, `srv_base.txt`

- [ ] **Step 1: Check the preconditions**

```bash
ssh -o BatchMode=yes chris@172.30.1.185 'cd ~/arguscore && git status --short | head -3; git branch --show-current; pgrep -x docker-buildx | wc -l; docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT COUNT(*) FROM characters.characters WHERE online=1" 2>/dev/null'
```

Expected: no changes, `feature/priest-class-hall`, `0`, `0`.

- [ ] **Step 2: Create the two branches**

In `~/arguscore-engine`: `git switch -c engine/class-hall-followers` (from `engine/class-hall-talents`).
In `~/arguscore`: `git switch -c integration/class-hall-followers` (from `feature/priest-class-hall`).

- [ ] **Step 3: Copy and adapt the helper scripts**

Copy `build.sh`, `boot_check.sh`, `pkt.py`, `rotate_opcode.py`, `dberr_base.txt` and `srv_base.txt` from `~/class-hall-talents/`. In `build.sh` the integration branch is `integration/class-hall-followers`, the only merge line is `git merge --no-edit engine/class-hall-followers`, and the warning filter adds `SpellEffects|GarrisonMgr`. Start `progress.md` with `# SDD ledger — plan: docs/superpowers/plans/2026-10-13-class-hall-followers.md` and a ruling that the 13,422 baseline is reused.

- [ ] **Step 4: Back up the test character**

```bash
docker exec arguscore-db-1 mysqldump -uarguscore -parguscore --no-tablespaces characters > ~/class-hall-followers/characters_before.sql
```

Record in the ledger Gairy's `xp`, `money`, level, rewarded quest count and currency rows. These are what the final revert is checked against.

- [ ] **Step 5: Data checks**

With the WDC1 reader in `/tmp/stage1/`:

1. List spells whose effect 220 names a class hall follower: every priest champion's spell (856 is 219756, 857 is 219757; 875 is needed in Task 5), one champion of another class (for the class refusal) and, if any exists, a priest troop (for the limit refusal). Write the ids to the ledger. Expected: a champion spell per champion; possibly no troop spell.
2. Print the `GarrFollowerXAbility` record ids for follower 856, faction 1. Expected: id order gives 354, 457, 777 before the traits. If the id order differs from that, record a ruling on the order rule before Task 2.

- [ ] **Step 6: Check the hand-send command reaches unhandled replies**

Read `HandleDebugSendOpcodeCommand` to its end and `WorldSession::SendPacket`. Expected: the command sends with `forced` true, or `STATUS_UNHANDLED` replies are not blocked. If they are blocked, the status-change probe of Task 2 Step 6 moves to login B1, after Task 3 has made them sendable, and Task 5 is written after that login; record the ruling.

- [ ] **Step 7: Turn the packet log on**

Set `PacketLogFile = "followers.pkt"` in `~/arguscore-server/configs/worldserver.conf`. Ledger: REVERT at cleanup.

---

### Task 1: Load `GarrFollowerType.db2` and `GarrFollowerQuality.db2`

**Files:**
- Create: `sql/updates/hotfixes/master/<date>_00_hotfixes.sql`
- Modify: `src/server/game/DataStores/DB2Structure.h`, `DB2LoadInfo.h`, `DB2Stores.h`, `DB2Stores.cpp`
- Modify: `src/server/database/Database/Implementation/HotfixDatabase.h`, `HotfixDatabase.cpp`

**Interfaces:**
- Produces:

```cpp
struct GarrFollowerQualityEntry
{
    uint32 ID;
    uint32 XpToNextQuality;
    uint16 ShipmentXP;
    uint8 Quality;
    uint8 AbilityCount;
    uint8 TraitCount;
    uint8 GarrFollowerTypeID;
    uint32 ClassSpecID;
};

struct GarrFollowerTypeEntry
{
    uint32 ID;
    uint16 MaxItemLevel;
    uint8 MaxFollowers;
    uint8 MaxFollowerBuildingType;
    uint8 GarrTypeID;
    uint8 LevelRangeBias;
    uint8 ItemLevelRangeBias;
    uint8 Flags;
};
```

- `sGarrFollowerQualityStore`, `sGarrFollowerTypeStore`; statements `HOTFIX_SEL_GARR_FOLLOWER_QUALITY`, `HOTFIX_SEL_GARR_FOLLOWER_TYPE` with their `_MAX_ID` twins.

- [ ] **Step 1: Write the hotfixes update**

Two tables, `garr_follower_quality` and `garr_follower_type`, columns in the struct order above with `ID` first, plus `VerifiedBuild`, primary key `(ID, VerifiedBuild)`, as `garr_talent` was written. No locale tables: neither file has text.

- [ ] **Step 2: Add the two stores**

Follow `GarrTalentTree` (a store whose id is not a field of the record: both metas have `IndexField = -1`). Field order and signs come from `GarrFollowerQualityMeta` and `GarrFollowerTypeMeta` in `DB2Metadata.h`; the load info lists `ID` first.

- [ ] **Step 3: Commit**

```bash
git add sql/updates/hotfixes src/server/game/DataStores src/server/database/Database/Implementation/HotfixDatabase.h src/server/database/Database/Implementation/HotfixDatabase.cpp
git commit -m "feat(engine,sql): load GarrFollowerType.db2 and GarrFollowerQuality.db2"
```

No build here; Task 2 shares it.

---

### Task 2: Store class hall followers and report them

**Files:**
- Create: `sql/updates/characters/master/<date>_00_characters.sql`
- Create: `src/server/game/ClassHall/ClassHallFollowers.h`, `ClassHallFollowers.cpp`
- Modify: `src/server/game/ClassHall/ClassHall.h`, `ClassHall.cpp`
- Modify: `src/server/database/Database/Implementation/CharacterDatabase.h`, `CharacterDatabase.cpp`
- Modify: `src/server/game/Entities/Player/Player.h` (login query slot), `Player.cpp` (load call), `src/server/game/Handlers/CharacterHandler.cpp` (login query)
- Modify: `src/server/game/Garrison/GarrisonMgr.cpp:351` (id generator start)

**Interfaces:**
- Consumes: the two stores of Task 1; `ClassHall` and its load, save, delete and `BuildInfo` (foundation, talents).
- Produces:

```cpp
class TC_GAME_API ClassHallFollowers
{
public:
    explicit ClassHallFollowers(Player* owner);

    void LoadFromDB(PreparedQueryResult followers);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

    void BuildInfo(WorldPackets::Garrison::GarrisonInfo& info) const;

private:
    struct Follower
    {
        GarrFollowerEntry const* Entry = nullptr;
        WorldPackets::Garrison::GarrisonFollower PacketInfo;

        bool IsTroop() const;       // Entry->Vitality > 0
    };

    std::list<GarrAbilityEntry const*> GetAbilities(GarrFollowerEntry const* follower, uint32 quality) const;

    Player* _owner;
    std::map<uint64 /*dbId*/, Follower> _followers;
};
```

- `ClassHall::GetFollowers()` (const and non-const), `bool ClassHall::LoadFromDB(PreparedQueryResult classHall, PreparedQueryResult talents, PreparedQueryResult followers)`.
- Statements `CHAR_SEL_CHARACTER_CLASS_HALL_FOLLOWERS`, `CHAR_INS_CHARACTER_CLASS_HALL_FOLLOWER`, `CHAR_DEL_CHARACTER_CLASS_HALL_FOLLOWERS`; login slot `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL_FOLLOWERS`.

- [ ] **Step 1: Write the characters update**

```sql
CREATE TABLE IF NOT EXISTS `character_class_hall_follower` (
  `dbId` BIGINT UNSIGNED NOT NULL,
  `guid` BIGINT UNSIGNED NOT NULL,
  `followerId` INT UNSIGNED NOT NULL,
  `quality` INT UNSIGNED NOT NULL DEFAULT '0',
  `level` INT UNSIGNED NOT NULL DEFAULT '0',
  `itemLevelWeapon` INT UNSIGNED NOT NULL DEFAULT '0',
  `itemLevelArmor` INT UNSIGNED NOT NULL DEFAULT '0',
  `xp` INT UNSIGNED NOT NULL DEFAULT '0',
  `durability` INT UNSIGNED NOT NULL DEFAULT '0',
  `status` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`dbId`),
  KEY `idx_guid` (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall followers';
```

with a comment saying why there is no unique key on `(guid, followerId)` (two troops can share a follower id) and no foreign key (as the talents table).

- [ ] **Step 2: Write `ClassHallFollowers`**

- `LoadFromDB`: per row, look the follower up; skip with one `TC_LOG_ERROR("entities.player", ...)` naming the player and the id when it is unknown or its `GarrTypeID` is not `GARRISON_TYPE_CLASS_ORDER`. Otherwise fill `PacketInfo` from the row and `AbilityID` from `GetAbilities`.
- `GetAbilities`: the owner's faction index is 0 for Horde, 1 for Alliance (as `Garrison::GetFaction`). Find the `GarrFollowerQuality` row with the follower's `GarrFollowerTypeID` and the given quality; with none, return an empty list. Walk `sGarrFollowerXAbilityStore` in id order; of the rows for this follower and faction, take the first `AbilityCount` whose ability lacks `GARRISON_ABILITY_FLAG_TRAIT` and the first `TraitCount` that have it.
- `SaveToDB`, `DeleteFromDB`, `BuildInfo`: as `ClassHallTalents` does; `BuildInfo` pushes `&follower.PacketInfo`.

- [ ] **Step 3: Forward from `ClassHall`, hook the login query, start the generator above both tables**

`GarrisonMgr::InitializeDbIdSequences` takes the larger of the two tables' `MAX(dbId)`.

- [ ] **Step 4: Commit, build, boot**

```bash
git add sql/updates/characters src/server/game/ClassHall src/server/database/Database/Implementation/CharacterDatabase.h src/server/database/Database/Implementation/CharacterDatabase.cpp src/server/game/Entities/Player/Player.h src/server/game/Entities/Player/Player.cpp src/server/game/Handlers/CharacterHandler.cpp src/server/game/Garrison/GarrisonMgr.cpp
git commit -m "feat(engine,sql): store class hall followers and report them to the client"
```

Run `build.sh` (long; covers Task 1) — expected `none`, `BUILD OK`.
Run `boot_check.sh` — expected `READY`, both updates applied, growth equal to the baseline, `none`.

- [ ] **Step 5: Plant the test rows**

With Gairy offline: the hall row `(1, 560)` and these followers (values from each follower's data row; `dbId` 9000001 upward so they are easy to tell apart):

| dbId | follower | status | note |
|------|----------|--------|------|
| 9000001 | 857 | 0 | champion, active |
| 9000002 | 870 | 4 | champion, inactive |
| 9000003 | 921 | 8 | Acolyte |
| 9000004 | 921 | 8 | a second Acolyte with the same follower id |
| 9000005 | 679 | 8 | Zealot |
| 9000006 | 999999 | 0 | unknown follower |
| 9000007 | a Warlords follower id | 0 | wrong garrison type |

- [ ] **Step 6: [user] Login A — the list, and the discoveries for later tasks**

1. The list:

```
/run for _,f in ipairs(C_Garrison.GetFollowers(4)) do print(f.garrFollowerID,f.name,f.level,f.quality,f.iLevel,f.isTroop,f.durability,f.maxDurability,f.status) end
```

Expected: five lines (857, 870, 921 twice, 679) with the level, quality and item level of the data, troops with their vitality, 870 marked inactive.

2. Abilities of the first champion:

```
/run local f=C_Garrison.GetFollowers(4) for _,x in ipairs(f) do if not x.isTroop then for _,a in ipairs(C_Garrison.GetFollowerAbilities(x.followerID)) do print(x.name,a.id,a.name,a.isTrait) end end end
```

Expected: for each champion its spec and one ability, no "Empty Equipment Slot".

3. The hall report: click the class hall button at the minimap; the followers tab shows the same five.
4. `/dump C_Garrison.GetFollowerActivationCost()` and `/dump C_Garrison.GetNumFollowerActivationsRemaining(3)`. Write both to the ledger.
5. Requests, to read their layouts from the packet log (the server ignores them for now):

```
/run local id for _,f in ipairs(C_Garrison.GetFollowers(4)) do if f.garrFollowerID==857 then id=f.followerID end end C_Garrison.SetFollowerInactive(id,true)
```
```
/run local id for _,f in ipairs(C_Garrison.GetFollowers(4)) do if f.garrFollowerID==679 then id=f.followerID end end C_Garrison.RemoveFollower(id)
```
```
/run C_Garrison.RequestClassSpecCategoryInfo(4)
```

6. Status-change probe (only if Task 0 Step 6 allows it): `rotate_opcode.py` feeds candidates that mark follower 9000001 inactive: 0x2904 as `uint32 0` + follower; 0x2915 as `uint32 0` + follower; 0x2915 as `uint64 dbId` + `uint32 4`. After each `.debug send opcode` the user runs the list line of step 1.
7. `.save`, log out.

Afterwards:

```bash
python3 ~/class-hall-followers/pkt.py ~/arguscore-server/logs/followers.pkt 32C5 32C5 32F8 32F8 32D5 32D5
grep -c "unknown GarrFollower\|not a class hall follower" ~/arguscore-server/logs/Server.log
```

Expected: one request of each kind with a readable layout (write each to the ledger, with whether the client sent it at all without a mission window); two error lines; the seven planted rows reduced to five by the save.

Rulings to write before Task 3: the three request layouts; which reply and layout marks a follower inactive on the client; the activation cost and daily count the client shows; whether the abilities matched. If a request was not sent without a window, plan to open the mission window by hand in login B1 (send 0x2923 "open mission NPC" with `.debug send opcode`) and capture then.

- [ ] **Step 7: A character without a hall; deletion**

As in the talents run: plant one follower row for the next character guid, start a poller on that row and the character, and ask the user to create `Followertest`, log in, `.save`, log out and delete it.
Expected: the row is untouched through login and save, and gone after the deletion.

---

### Task 3: Add the garrison follower packets

**Files:**
- Modify: `src/server/game/Server/Protocol/Opcodes.h`, `Opcodes.cpp`
- Modify: `src/server/game/Server/Packets/GarrisonPackets.h`, `GarrisonPackets.cpp`

**Interfaces:**
- Produces, in `WorldPackets::Garrison` (layouts from the ledger where Task 2 Step 6 settled them):

```cpp
class GarrisonSetFollowerInactive final : public ClientPacket     // CMSG_GARRISON_SET_FOLLOWER_INACTIVE
{
    void Read() override;
    uint64 FollowerDBID = 0;
    bool Inactive = false;
};

class GarrisonRemoveFollower final : public ClientPacket          // CMSG_GARRISON_REMOVE_FOLLOWER
{
    void Read() override;
    uint64 FollowerDBID = 0;        // plus whatever else the packet log showed
};

class GarrisonRequestClassSpecCategoryInfo final : public ClientPacket
{
    void Read() override;
    uint32 GarrFollowerTypeID = 0;
};

class GarrisonFollowerChangedStatus final : public ServerPacket   // the reply the probe confirmed
{
    WorldPacket const* Write() override;
    uint32 Result = 0;
    GarrisonFollower Follower;
};

class GarrisonNumFollowerActivationsRemaining final : public ServerPacket
{
    WorldPacket const* Write() override;
    int32 GarrTypeID = 0;
    uint32 Activations = 0;
};

struct GarrisonClassSpecCategoryInfo
{
    int32 GarrClassSpecID = 0;
    int32 GarrClassSpecPlayerCondID = 0;
};

class GarrisonGetClassSpecCategoryInfoResult final : public ServerPacket
{
    WorldPacket const* Write() override;
    int32 GarrFollowerTypeID = 0;
    std::vector<GarrisonClassSpecCategoryInfo> CategoryInfos;
};
```

- [ ] **Step 1: Opcodes**

`Opcodes.h`: add `SMSG_GARRISON_GET_CLASS_SPEC_CATEGORY_INFO_RESULT = 0x2901` in alphabetical position. `Opcodes.cpp`: a `DEFINE_SERVER_OPCODE_HANDLER(..., STATUS_NEVER, CONNECTION_TYPE_INSTANCE)` line for it, and `STATUS_NEVER` in place of `STATUS_UNHANDLED` for the replies this plan sends (0x2917 and whichever of 0x2904 and 0x2915 the probe confirmed). No existing opcode is renamed.

- [ ] **Step 2: Packet classes**

Follow `GarrisonResearchTalent` and `GarrisonResearchTalentResult` from the talents branch.

- [ ] **Step 3: Commit**

```bash
git add src/server/game/Server/Protocol/Opcodes.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Server/Packets/GarrisonPackets.h src/server/game/Server/Packets/GarrisonPackets.cpp
git commit -m "feat(engine): add the garrison follower packets"
```

No build here; Tasks 4 to 6 share it.

---

### Task 4: Add a class hall follower from its spell

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallFollowers.h`, `ClassHallFollowers.cpp`
- Modify: `src/server/game/Spells/SpellEffects.cpp` (`Spell::EffectAddGarrisonFollower`)

**Interfaces:**
- Consumes: `GarrisonAddFollowerResult` (existing), `GetAbilities` (Task 2), `sGarrisonMgr.GenerateFollowerDbId()`.
- Produces:

```cpp
void AddFollower(uint32 garrFollowerId);
uint32 CountActiveChampions() const;
uint32 CountTroops(uint32 garrClassSpecId) const;

// private
GarrisonError CanAddFollower(GarrFollowerEntry const* follower) const;
uint32 GetClassSpec(GarrFollowerEntry const* follower) const;      // the owner's faction's class spec id
```

- [ ] **Step 1: `CanAddFollower`**

| Check | Error |
|-------|-------|
| `follower` is not null and its `GarrTypeID` is `GARRISON_TYPE_CLASS_ORDER` | `INVALID_FOLLOWER` |
| `ChrClassID` is 0 or the owner's class | `INVALID_CLASS` |
| it lacks `GARRISON_FOLLOWER_FLAG_UNIQUE`, or the owner has no follower with this id | `FOLLOWER_EXISTS` |
| it is not a troop, or `CountTroops(GetClassSpec(follower))` is below the class spec's `FollowerClassLimit` | `AT_CLASS_SPEC_CAP` |

- [ ] **Step 2: `AddFollower`**

Always sends one `GarrisonAddFollowerResult` with `GarrTypeID = GARRISON_TYPE_CLASS_ORDER` and the result. On success: a new id from the generator; quality, level and item levels from the data row; experience 0; durability = `Vitality`; status `FOLLOWER_STATUS_TROOP` for a troop, and for a champion `FOLLOWER_STATUS_INACTIVE` when `CountActiveChampions()` has already reached the follower type's `MaxFollowers` (ruling to confirm in Step 6); abilities from `GetAbilities`; the follower in the reply; then `_owner->UpdateCriteria(CriteriaType::RecruitGarrisonFollower, garrFollowerId)`.

- [ ] **Step 3: Route the spell effect**

In `Spell::EffectAddGarrisonFollower`: look the follower up; when it is a class hall follower, call the class hall's `GetFollowers().AddFollower(id)` if the player has a hall, send `GarrisonAddFollowerResult` with `GARRISON_ERROR_NO_GARRISON` and the class hall type if not, and return. Otherwise the existing lines run unchanged.

- [ ] **Step 4: Commit**

```bash
git add src/server/game/ClassHall src/server/game/Spells/SpellEffects.cpp
git commit -m "feat(engine): add a class hall follower from its spell"
```

The build comes after Task 6. The in-game check is Step 5, in login B1.

- [ ] **Step 5: [user] Login B1, first half — gain**

State before login: the hall row, followers 857 (active), 921 twice and 679 from Task 2; no quest.

1. `.quest add 43270`
2. `.cast self 219756 triggered`, then the list line of Task 2 Step 6.
   Expected: Calia in the list without a relog; the quest's "Recruit Calia" objective is ticked.
3. `.cast self 219756 triggered` again. Expected: still one Calia.
4. The other class's champion spell from Task 0 Step 5. Expected: nothing added.
5. If Task 0 Step 5 found a troop spell: cast the Acolyte one twice. Expected: the first makes three Acolytes, the second is refused.
6. `.save`.

```bash
python3 ~/class-hall-followers/pkt.py ~/arguscore-server/logs/followers.pkt 2902 2902
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT dbId, followerId, status FROM characters.character_class_hall_follower WHERE guid=1 ORDER BY dbId" 2>/dev/null
```

Expected: results 0, 27 (follower exists), 88 (invalid class), and 81 for the refused troop; Calia's row with a `dbId` above 9000007.

- [ ] **Step 6: Record what the sixth champion does**

Covered in login B2 (Task 5 Step 5), where five champions are active.

---

### Task 5: Set a class hall follower active or inactive

**Files:**
- Create: `sql/updates/characters/master/<date>_01_characters.sql`
- Modify: `src/server/game/ClassHall/ClassHallFollowers.h`, `ClassHallFollowers.cpp`, `ClassHall.h`, `ClassHall.cpp`
- Modify: `src/server/database/Database/Implementation/CharacterDatabase.cpp` (the hall's select and replace statements)
- Modify: `src/server/game/Server/WorldSession.h`, `src/server/game/Server/Protocol/Opcodes.cpp`, `src/server/game/Handlers/GarrisonHandler.cpp`
- Modify: `src/server/game/Entities/Player/Player.cpp:24410` (daily reset)

**Interfaces:**
- Consumes: the packets of Task 3; the ledger's reply choice, activation cost and daily count.
- Produces:

```cpp
// ClassHallFollowers
void SetFollowerInactive(uint64 dbId, bool inactive);
uint32 GetActivationsRemaining() const { return _activationsRemaining; }
void SetActivationsRemaining(uint32 activations) { _activationsRemaining = activations; }
void ResetActivations();        // back to the daily count, and tells the client

// ClassHall
void ResetFollowerActivations();
```

- `void WorldSession::HandleGarrisonSetFollowerInactive(WorldPackets::Garrison::GarrisonSetFollowerInactive& setFollowerInactive)`

- [ ] **Step 1: The column**

```sql
ALTER TABLE `character_class_hall` ADD COLUMN `followerActivationsRemaining` INT UNSIGNED NOT NULL DEFAULT '1' AFTER `siteLevelId`;
```

`ClassHall` loads it into its followers and saves it from them; `ClassHall::BuildInfo` sets `info.NumFollowerActivationsRemaining`. The default and the daily count are the ledger's value (expected 1).

- [ ] **Step 2: `SetFollowerInactive`**

Checked in this order; the first failure sends the status reply with the error and changes nothing:

| Check | Error |
|-------|-------|
| the owner has a follower with this id | `INVALID_FOLLOWER` |
| it is not a troop | `INVALID_FOLLOWER` |
| its state differs from the wanted one | `FOLLOWER_INACTIVE` when asked to deactivate, `FOLLOWER_ACTIVE` when asked to activate |
| activating: `CountActiveChampions()` is below `MaxFollowers` | `AT_FOLLOWER_HARD_CAP` |
| activating: an activation is left today | `FOLLOWER_ACTIVATION_UNAVAILABLE` |
| activating: the owner has the cost | `NOT_ENOUGH_GOLD` |

On success: activating takes the cost (`ModifyMoney`) and one activation, and sends `GarrisonNumFollowerActivationsRemaining`; the status flag changes; the status reply is sent with result 0 and the follower.

The cost is a named constant in `ClassHallFollowers.cpp` holding the value the client displayed, unless Task 2 Step 6 showed it is in client data.

- [ ] **Step 3: Handler and daily reset**

`HandleGarrisonSetFollowerInactive`: with a class hall, call `SetFollowerInactive`; without one, do nothing (the request belongs to another garrison type). `Opcodes.cpp`: `STATUS_LOGGEDIN`, `PROCESS_THREADUNSAFE`. In `Player::ResetDailyQuestStatus`, next to the Warlords line: `if (_classHall) _classHall->ResetFollowerActivations();`

- [ ] **Step 4: Commit**

```bash
git add sql/updates/characters src/server/game/ClassHall src/server/database/Database/Implementation/CharacterDatabase.cpp src/server/game/Server/WorldSession.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Handlers/GarrisonHandler.cpp src/server/game/Entities/Player/Player.cpp
git commit -m "feat(engine,sql): set a class hall follower active or inactive"
```

- [ ] **Step 5: [user] Logins B1 (second half) and B2 — active, inactive, limit, cost**

B1, continuing after Task 4 Step 5 (two active champions):

1. Set Calia inactive from the follower list's menu (or `C_Garrison.SetFollowerInactive(id,true)`); the list shows her inactive without a relog.
2. Note Gairy's gold. Activate her again. Expected: active again, gold down by the cost, `/dump C_Garrison.GetNumFollowerActivationsRemaining(3)` is 0.
3. Set her inactive and try to activate once more. Expected: refused (no activation left today), gold unchanged.
4. `/run C_Garrison.SetFollowerInactive("0x0000000000000000",true)`. Expected: nothing happens, no disconnect.
5. `.save`, log out.

With Gairy offline: plant champions 871, 872, 873, 874 active so five are active (857 and these four; Calia inactive), set `followerActivationsRemaining` to 1 and Gairy's money to 1 copper below the cost.

B2:

1. Try to activate Calia. Expected: refused (five active).
2. Set 874 inactive, try Calia. Expected: refused (not enough gold), gold unchanged.
3. `.modify money` up by one gold; activate Calia. Expected: active, cost taken.
4. `.cast self 219757 triggered` is already owned; cast the spell of champion 875 (from Task 0 Step 5). Expected and to record: with five active, the new champion arrives inactive. If the client shows something else, correct `AddFollower` and the spec.
5. `.save`.

```bash
python3 ~/class-hall-followers/pkt.py ~/arguscore-server/logs/followers.pkt 32C5 32C5 2904 2904 2915 2915 2917 2917
```

Expected results in order: 0, 0 with the activations reply, 71, 28; then 78, 47, 0.

---

### Task 6: Dismiss a class hall follower

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallFollowers.h`, `ClassHallFollowers.cpp`
- Modify: `src/server/game/Server/WorldSession.h`, `src/server/game/Server/Protocol/Opcodes.cpp`, `src/server/game/Handlers/GarrisonHandler.cpp`

**Interfaces:**
- Consumes: `GarrisonRemoveFollower`, `GarrisonRemoveFollowerResult`.
- Produces: `void ClassHallFollowers::RemoveFollower(uint64 dbId)`; `void WorldSession::HandleGarrisonRemoveFollower(WorldPackets::Garrison::GarrisonRemoveFollower& removeFollower)`.

- [ ] **Step 1: `RemoveFollower`**

Always sends `GarrisonRemoveFollowerResult` with the id and the class hall type. Refused with `INVALID_FOLLOWER` when the owner has no such follower. Refused when the follower may not be dismissed: troops may; champions follow what the client offered in login A (ledger; expected: the client offers no dismissal for a champion, so the server refuses one). On success the follower is erased and the result has `Destroyed = 1`.

- [ ] **Step 2: Handler, commit, build, boot**

Handler as in Task 5. Then:

```bash
git add src/server/game/ClassHall src/server/game/Server/WorldSession.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine): dismiss a class hall follower"
```

Run `build.sh` (long; covers Tasks 3 to 6) — expected `none`, `BUILD OK`.
Run `boot_check.sh` — expected `READY`, the characters update applied, baseline growth, `none`.

Logins B1 and B2 follow (Task 4 Step 5, Task 5 Step 5, and Step 3 below at the end of B2).

- [ ] **Step 3: [user] End of login B2 — dismiss, and the troop reply probe**

1. Dismiss one of the two Acolytes with follower id 921 from the list's menu (or `C_Garrison.RemoveFollower(id)`). Expected: one Acolyte 921 left, without a relog.
2. `/run C_Garrison.RemoveFollower("0x0000000000000000")`. Expected: nothing happens.
3. Try to dismiss a champion, if the client offers it; otherwise `C_Garrison.RemoveFollower` with a champion's id. Record what happens.
4. Troop reply probe: `.debug send opcode` with 0x2901 = `uint32 4`, `uint32 2`, `uint32 91`, `uint32 59`, `uint32 92`, `uint32 61`; then

```
/run for _,c in ipairs(C_Garrison.GetClassSpecCategoryInfo(4)) do print(c.classSpecID,c.name,c.count,c.limit) end
```

Expected: two lines, "Group of Acolytes" with count 1 and limit 3, "Band of Zealots" with 1 and 2. Write the result to the ledger; Task 7 depends on it.
5. `.save`, log out, close the client.

```bash
python3 ~/class-hall-followers/pkt.py ~/arguscore-server/logs/followers.pkt 32F8 32F8 2903 2903
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT dbId, followerId, status FROM characters.character_class_hall_follower WHERE guid=1 ORDER BY dbId" 2>/dev/null
```

Expected: results 0, 28 and the champion's outcome; one row with follower 921.

---

### Task 7: Report the class hall's troop kinds

Only if Task 6 Step 3's probe showed the client accepts the reply. If it did not, stop here, record it, and leave this task to a follow-up as the spec allows.

**Files:**
- Create: `sql/updates/hotfixes/master/<date>_01_hotfixes.sql`
- Modify: the six store files of Task 1 (for `GarrClassSpecPlayerCond.db2`)
- Modify: `src/server/game/ClassHall/ClassHallFollowers.h`, `ClassHallFollowers.cpp`
- Modify: `src/server/game/Server/WorldSession.h`, `src/server/game/Server/Protocol/Opcodes.cpp`, `src/server/game/Handlers/GarrisonHandler.cpp`

**Interfaces:**
- Produces:

```cpp
struct GarrClassSpecPlayerCondEntry
{
    uint32 ID;
    LocalizedString Name;
    int32 IconFileDataID;
    uint8 OrderIndex;
    uint32 GarrClassSpecID;
    uint32 PlayerConditionID;
    int32 FlavorGarrStringID;
};
```

- `sGarrClassSpecPlayerCondStore` with its hotfix tables (`garr_class_spec_player_cond`, and a locale table for `Name`).
- `void ClassHallFollowers::SendClassSpecCategoryInfo(uint32 garrFollowerTypeId) const`
- `void WorldSession::HandleGarrisonRequestClassSpecCategoryInfo(WorldPackets::Garrison::GarrisonRequestClassSpecCategoryInfo& requestClassSpecCategoryInfo)`

- [ ] **Step 1: The store**

As Task 1; field order and signs from `GarrClassSpecPlayerCondMeta`.

- [ ] **Step 2: `SendClassSpecCategoryInfo`**

For a follower type other than `FOLLOWER_TYPE_CLASS_ORDER`, send the reply empty. Otherwise collect the class spec ids (for the owner's faction) of every `GarrFollower` that is a class hall troop of the owner's class. For each, among the `GarrClassSpecPlayerCond` rows of that class spec whose `PlayerConditionID` the owner meets (`ConditionMgr::IsPlayerMeetingCondition`), send the one with the highest `OrderIndex`; with none met, leave the kind out.

- [ ] **Step 3: Handler, commit, build, boot**

```bash
git add sql/updates/hotfixes src/server/game/DataStores src/server/database/Database/Implementation/HotfixDatabase.h src/server/database/Database/Implementation/HotfixDatabase.cpp src/server/game/ClassHall src/server/game/Server/WorldSession.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine,sql): report the class hall's troop kinds"
```

Run `build.sh` (long) — expected `none`, `BUILD OK`. Run `boot_check.sh` — expected `READY`, the hotfixes update applied, baseline growth, `none`.

- [ ] **Step 4: [user] Login C**

```
/run C_Garrison.RequestClassSpecCategoryInfo(4)
```
```
/run for _,c in ipairs(C_Garrison.GetClassSpecCategoryInfo(4)) do print(c.classSpecID,c.name,c.count,c.limit) end
```

Expected: one line per troop kind whose condition Gairy meets, with the counts of her planted troops. Which conditions she meets is not known in advance; the packet log shows what was sent, and each line sent must match a condition the server evaluated as met. If no kind is listed, record the conditions' contents (42849, 41907, 48410) and whether that is right for a character without the campaign quests; that is a correct empty answer, not a failure.

---

### Task 8: Land it

- [ ] **Step 1: Final review**

Dispatch one fresh reviewer on the most capable model over `engine/class-hall-talents..engine/class-hall-followers`, with this plan, the spec, the Review Focus section and the ledger's rulings. The test setup stays in place until it reports. Critical and Important findings get one fix pass, each fix rebuilt and rechecked in game; Minor findings go to the ledger as deferred.

- [ ] **Step 2: Clean up**

With every character offline: remove every planted row listed in the ledger, set `PacketLogFile` back to empty, remove `followers.pkt` and `opcode.txt`. Put Gairy back exactly as the backup of Task 0 Step 4 has her: no hall row, no followers, quest 43270 gone, `xp`, `money`, currencies and progress counters as recorded (use the binary log if a value changed that the ledger did not record). Compare against the ledger's record and show the before and after counts. Restart the server clean.

- [ ] **Step 3: Merge**

With no build running, each merge in a temporary worktree except the last:

```bash
cd ~/arguscore
git worktree add /tmp/wt-dev dev && git -C /tmp/wt-dev merge --no-ff --no-edit engine/class-hall-followers && git worktree remove /tmp/wt-dev
git worktree add /tmp/wt-art feature/priest-artifact-weapon && git -C /tmp/wt-art merge --no-edit dev && git worktree remove /tmp/wt-art
git switch feature/priest-class-hall && git merge --no-edit dev
git diff --stat integration/class-hall-followers feature/priest-class-hall -- src sql
git branch -D integration/class-hall-followers
```

Expected: the `git diff` prints nothing.

- [ ] **Step 4: Update the docs and push**

On `dev`, in a temporary worktree: add `engine/class-hall-followers` to "Current branches" in `docs/BRANCHING.md` and replace the spec's "Open points" with what the tests showed, as one `chore(local)` commit. Merge `dev` into both priest branches again. Push `engine/class-hall-followers`, `dev`, `feature/priest-artifact-weapon` and `feature/priest-class-hall` (rule 13).

- [ ] **Step 5: Report**

State which checks passed and which did not; every `Ruling:` line with what it costs if wrong; every deferred minor; the answer to each open point of the spec; which branch `~/arguscore` is on. Update the class hall memory file.
