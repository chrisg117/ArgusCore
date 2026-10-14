# Class Hall Shipments Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A character with a class hall can order troops from a recruiter for Order Resources, see the orders in the order window and the hall report, and collect a finished troop from the object beside the recruiter.

**Architecture:** A new `ClassHallShipments` class in `src/server/game/ClassHall/`, owned by `ClassHall`, with its own table `character_class_hall_shipment`. A gossip option names the shipment an NPC offers (a new column beside `GarrTalentTreeID`). An order is made by casting the shipment's own spell: the spell check refuses it before the cost is taken, and spell effect 216 adds it. Using a game object of type 45 collects ready orders and adds the troop through `ClassHallFollowers`. The Warlords `Garrison` class is not changed.

**Tech Stack:** C++ (ArgusCore, a TrinityCore fork for 7.3.5 build 26972), MySQL update files, Docker image build on the host `172.30.1.185`.

**Spec:** `docs/superpowers/specs/2026-10-14-class-hall-shipments-design.md`

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

1. A refused order takes nothing: no Order Resources, no row. Pinned in Task 8 Step 1 (items 6 and 7).
2. A create request with a count far above what is allowed creates only what is allowed. Pinned in Task 8 Step 1 (item 6). A count of 0 is read in review.
3. A request naming an NPC that is not the opened one, or one out of reach, is ignored. Pinned in Task 8 Step 1 (item 5).
4. A ready order whose troop kind is full stays ready and gives no quest credit. Pinned in Task 8 Step 3.
5. Saved orders whose hall row is missing do not load and do not block login; a saved order with an unknown shipment is skipped. Pinned in Task 2 Steps 6 and 7.

Not covered by any in-game test, for the final reviewer to read with care: a Horde character; the order spell of a Warlords container (effect 216 must still do nothing for it); collecting a held-back order after a troop slot frees up; the character dump.

## Facts the tasks rely on

Read from the client files and the code at `engine/class-hall-followers` `a4e42f93`:

- `CharShipmentMeta` (`DB2Metadata.h:1820`): 9 fields, `IndexField` -1, `ParentIndexField` 5. `CharShipmentContainerMeta`: 16 fields, `IndexField` -1, no parent.
- Priest troop shipments (id: container, follower, seconds, `MaxShipments`, order spell, cost in currency 1220, on-complete spell): 275: 149, 677, 1800, 4, 210957, 20, 210901. 333 (tutorial): 149, 677, 5, 1, 219888, 20, 219889. 276: 150, 679, 1800, 3, 210958, 85, 210902. Not a troop: 370 (container 207, armaments). A monk's: 249 (container 124, follower 622).
- Troop limits (`GarrClassSpec.FollowerClassLimit`): Acolytes (class spec 91) 3, Zealots (92) 2.
- Order spells have effect 216 with the shipment id as `MiscValue`, target the caster, and have no cast time. On-complete spells 210901 and 219889 give kill credit 106404, the objective of quest 43275.
- Opcodes, all `STATUS_UNHANDLED` today: `CMSG_OPEN_SHIPMENT_NPC` 0x32DD, `CMSG_GARRISON_REQUEST_SHIPMENT_INFO` 0x32DE, `CMSG_GARRISON_REQUEST_LANDING_PAGE_SHIPMENT_INFO` 0x32DF, `CMSG_CREATE_SHIPMENT` 0x32E0, `SMSG_OPEN_SHIPMENT_NPC_FROM_GOSSIP` 0x27DA (10202), `SMSG_GET_SHIPMENT_INFO_RESPONSE` 0x27DB (10203), `SMSG_CREATE_SHIPMENT_RESPONSE` 0x27DD (10205), `SMSG_COMPLETE_SHIPMENT_RESPONSE` 0x27DE (10206), `SMSG_GET_SHIPMENTS_OF_TYPE_RESPONSE` 0x27DF (10207), `SMSG_GARRISON_LANDING_PAGE_SHIPMENT_INFO` 0x27E0 (10208).
- Layouts from WowPacketParser: the table in the spec's "Packets" section. One pending order is `int32` shipment, `uint64` order id, `uint64` follower, `uint32` time, `int32` duration, `int32` building type.
- The client's order window reads the info event as `success, _, maxShipments, ownedShipments, plotID` and offers `maxShipments - pending - ownedShipments` orders.
- `gossip_menu_option_addon` has `MenuID`, `OptionID`, `GarrTalentTreeID`, `VerifiedBuild` and no rows. `ObjectMgr::LoadGossipMenuItemAddon` (`ObjectMgr.cpp:9766`) fills `GossipMenuItemAddon` (`ObjectMgr.h:748`). `Player::OnGossipSelect` handles `GossipOptionNpc::GarrisonTalent` at `Player.cpp:13645`; `ShipmentCrafter` (28) falls to `default`.
- `Spell::CheckCast` has a switch over each effect (`Spell.cpp`, near the `SPELL_EFFECT_CREATE_HEIRLOOM_ITEM` case at 6618). The effect table has `&Spell::EffectNULL` for 216 (`SpellEffects.cpp:304`). `Spell::EffectAddGarrisonFollower` (`SpellEffects.cpp:5447`) is the model for a class hall effect.
- `GameObject::Use` (`GameObject.cpp`, the `GAMEOBJECT_TYPE_ARTIFACT_FORGE` case at 3310) is the model for a player-only use. The container of a type 45 object is `GetGOInfo()->garrisonShipment.ShipmentContainer`.
- `PlayerDump.cpp:98` lists `character_class_hall` and `character_class_hall_talent`.
- `ClassHallFollowers`: `void AddFollower(uint32 garrFollowerId)` is public; `CanAddFollower(GarrFollowerEntry const*)`, `GetClassSpec(GarrFollowerEntry const*)` and `CountTroops(uint32 garrClassSpecId)` are private.
- `WorldObject::CastSpell` returns a `SpellCastResult`.
- World database: `Vicar Eliza` 106451 has `npcflag` 2 and no gossip menu; object templates 250909 (container 149) and 250910 (container 150) exist; nothing of them is spawned.
- `.debug send opcode` reaches replies marked `STATUS_UNHANDLED` (followers ledger). A candidate line is `label | <decimal opcode> <type value>...`, with `itsguid` for the target's packed guid and `{now}` for the time.
- Baseline: DBErrors growth per boot 13,422.

## Builds and logins

Two long builds (about 40 minutes each) and the logins they serve:

| Build | Commits | Login |
|-------|---------|-------|
| A | Task 1, Task 2 | A: planted orders, discoveries for Tasks 3 to 7 |
| B | Tasks 3 to 7 | B1: open, order, collect, report; B2: ready orders after a relog, the full troop kind |

The packet log stays on for the whole run (`PacketLogFile = "shipments.pkt"`).

---

### Task 0: Workspace, branches, backups and data checks

**Files:**
- On the host: `~/class-hall-shipments/` (it already holds `ship0.py`, `ship0.out` and `spellraw.py` from the research for this plan)

- [ ] **Step 1: Check the preconditions**

```bash
ssh -o BatchMode=yes chris@172.30.1.185 'cd ~/arguscore && git status --short | head -3; git branch --show-current; pgrep -x docker-buildx | wc -l; docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT COUNT(*) FROM characters.characters WHERE online=1" 2>/dev/null'
```

Expected: no changes, `feature/priest-class-hall`, `0`, `0`.

- [ ] **Step 2: Create the two branches**

In `~/arguscore-engine`: `git switch -c engine/class-hall-shipments` (from `engine/class-hall-followers`).
In `~/arguscore`: `git switch -c integration/class-hall-shipments` (from `feature/priest-class-hall`).

- [ ] **Step 3: Copy and adapt the helper scripts**

Copy `build.sh`, `boot_check.sh`, `pkt.py`, `rotate_opcode.py`, `dbdiff.py`, `edit_common.py`, `dberr_base.txt` and `srv_base.txt` from `~/class-hall-followers/`. In the scripts the directory is `~/class-hall-shipments`, the integration branch is `integration/class-hall-shipments`, the only merge line is `git merge --no-edit engine/class-hall-shipments`, and the warning filter adds `ObjectMgr|Spell\.cpp|GameObject\.cpp`. Start `progress.md` with `# SDD ledger — plan: docs/superpowers/plans/2026-10-14-class-hall-shipments.md` and a ruling that the 13,422 baseline is reused.

- [ ] **Step 4: Back up**

```bash
docker exec arguscore-db-1 mysqldump -uarguscore -parguscore --no-tablespaces characters > ~/class-hall-shipments/characters_before.sql
```

Record in the ledger Gairy's `xp`, `money`, level, rewarded quest count and currency rows, and from the world database: the `npcflag` of creature 106451, and that no `creature_template_gossip` row for 106451 and no `gossip_menu` row 9000001 exist. These are what the final revert is checked against.

- [ ] **Step 5: Data checks**

1. `python3 ship0.py | grep -A4 "^container 149 "`. Expected: the three shipments 275, 332, 333 with the values in "Facts".
2. Print one container of `GarrTypeID` 2 with its fields. Expected: field 10 (`GarrBuildingType`) is not 0, which confirms the order of the one-byte fields used in Task 1.
3. `python3 spellraw.py 210957 219888 210958`. Expected: one effect 216 each, and currency 1220 costs 20, 20 and 85.

- [ ] **Step 6: Plant the recruiter's flags and turn the packet log on**

`UPDATE world.creature_template SET npcflag = 68719476737 WHERE entry = 106451` (gossip, and "shipment crafter" in the upper half). Set `PacketLogFile = "shipments.pkt"` in `~/arguscore-server/configs/worldserver.conf`. Ledger: REVERT both at cleanup.

---

### Task 1: Load `CharShipment.db2` and `CharShipmentContainer.db2`

**Files:**
- Create: `sql/updates/hotfixes/master/<date>_00_hotfixes.sql`
- Modify: `src/server/game/DataStores/DB2Structure.h`, `DB2LoadInfo.h`, `DB2Stores.h`, `DB2Stores.cpp`
- Modify: `src/server/database/Database/Implementation/HotfixDatabase.h`, `HotfixDatabase.cpp`

**Interfaces:**
- Produces:

```cpp
struct CharShipmentEntry
{
    uint32 ID;
    uint32 TreasureID;
    uint32 Duration;
    int32 SpellID;
    int32 DummyItemID;
    int32 OnCompleteSpellID;
    uint16 ContainerID;
    uint16 GarrFollowerID;
    uint8 MaxShipments;
    uint8 Flags;
};

struct CharShipmentContainerEntry
{
    uint32 ID;
    LocalizedString PendingText;
    LocalizedString Description;
    uint32 WorkingSpellVisualID;
    uint16 UiTextureKitID;
    uint16 WorkingDisplayInfoID;
    uint16 SmallDisplayInfoID;
    uint16 MediumDisplayInfoID;
    uint16 LargeDisplayInfoID;
    uint16 CrossFactionID;
    uint8 BaseCapacity;
    uint8 GarrBuildingType;
    uint8 GarrTypeID;
    uint8 MediumThreshold;
    uint8 LargeThreshold;
    int8 Faction;
    uint32 CompleteSpellVisualID;
};
```

- `sCharShipmentStore`, `sCharShipmentContainerStore`; statements `HOTFIX_SEL_CHAR_SHIPMENT`, `HOTFIX_SEL_CHAR_SHIPMENT_CONTAINER`, `HOTFIX_SEL_CHAR_SHIPMENT_CONTAINER_LOCALE` with their `_MAX_ID` twins.

- [ ] **Step 1: Write the hotfixes update**

Tables `char_shipment`, `char_shipment_container` and `char_shipment_container_locale`, columns in the struct order above with `ID` first, plus `VerifiedBuild`, as `garr_class_spec_player_cond` and its locale table were written.

- [ ] **Step 2: Add the two stores**

`CharShipment` follows `GarrFollowerXAbility` (a store with a parent field); `CharShipmentContainer` follows `GarrClassSpecPlayerCond` (a store with text). Field order and signs come from the two metas; the load info lists `ID` first.

- [ ] **Step 3: Commit**

```bash
git add sql/updates/hotfixes src/server/game/DataStores src/server/database/Database/Implementation/HotfixDatabase.h src/server/database/Database/Implementation/HotfixDatabase.cpp
git commit -m "feat(engine,sql): load CharShipment.db2 and CharShipmentContainer.db2"
```

No build here; Task 2 shares it.

---

### Task 2: Store class hall shipments

**Files:**
- Create: `sql/updates/characters/master/<date>_00_characters.sql`
- Create: `src/server/game/ClassHall/ClassHallShipments.h`, `ClassHallShipments.cpp`
- Modify: `src/server/game/ClassHall/ClassHall.h`, `ClassHall.cpp`
- Modify: `src/server/database/Database/Implementation/CharacterDatabase.h`, `CharacterDatabase.cpp`
- Modify: `src/server/game/Entities/Player/Player.h` (login query slot), `Player.cpp` (load call), `src/server/game/Handlers/CharacterHandler.cpp` (login query)
- Modify: `src/server/game/Tools/PlayerDump.cpp:98` (table list)

**Interfaces:**
- Consumes: the two stores of Task 1; `ClassHall` and its load, save and delete.
- Produces:

```cpp
class TC_GAME_API ClassHallShipments
{
public:
    ClassHallShipments(Player* owner, ClassHall* classHall);

    void LoadFromDB(PreparedQueryResult shipments);
    void SaveToDB(CharacterDatabaseTransaction trans) const;
    static void DeleteFromDB(ObjectGuid::LowType ownerGuid, CharacterDatabaseTransaction trans);

private:
    struct Shipment
    {
        CharShipmentEntry const* Entry = nullptr;
        uint64 Id = 0;
        time_t CreationTime = 0;
        uint32 Duration = 0;

        bool IsReady() const;       // CreationTime + Duration has passed
    };

    static bool IsTroopShipment(CharShipmentEntry const* shipment);  // class hall container, names a follower

    Player* _owner;
    ClassHall* _classHall;
    std::map<uint64 /*shipmentId*/, Shipment> _shipments;
    uint64 _nextShipmentId = 1;
};
```

- `ClassHall::GetShipments()` (const and non-const); `bool ClassHall::LoadFromDB(PreparedQueryResult classHall, PreparedQueryResult talents, PreparedQueryResult followers, PreparedQueryResult shipments)`.
- Statements `CHAR_SEL_CHARACTER_CLASS_HALL_SHIPMENTS`, `CHAR_INS_CHARACTER_CLASS_HALL_SHIPMENT`, `CHAR_DEL_CHARACTER_CLASS_HALL_SHIPMENTS`; login slot `PLAYER_LOGIN_QUERY_LOAD_CLASS_HALL_SHIPMENTS`.

- [ ] **Step 1: Write the characters update**

```sql
CREATE TABLE IF NOT EXISTS `character_class_hall_shipment` (
  `guid` BIGINT UNSIGNED NOT NULL,
  `shipmentId` BIGINT UNSIGNED NOT NULL,
  `charShipmentId` INT UNSIGNED NOT NULL,
  `creationTime` BIGINT NOT NULL DEFAULT '0',
  `duration` INT UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`, `shipmentId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='Legion class hall troop orders';
```

with a comment saying why there is no foreign key (as the talents table).

- [ ] **Step 2: Write `ClassHallShipments`**

- `LoadFromDB`: per row, look the shipment up; skip with one `TC_LOG_ERROR("entities.player", ...)` naming the player and the id when it is unknown or `IsTroopShipment` is false. The message contains the words "class hall shipment", which Step 6 greps for. `_nextShipmentId` ends one above the highest id loaded.
- `SaveToDB`, `DeleteFromDB`: as `ClassHallFollowers` does.

- [ ] **Step 3: Forward from `ClassHall`, hook the login query, add the table to the dump list**

`ClassHall` declares `_shipments` after `_followers`. The dump entry is `{ "character_class_hall_shipment", DTT_CHAR_TABLE }`.

- [ ] **Step 4: Commit, build, boot**

```bash
git add sql/updates/characters src/server/game/ClassHall src/server/database/Database/Implementation/CharacterDatabase.h src/server/database/Database/Implementation/CharacterDatabase.cpp src/server/game/Entities/Player/Player.h src/server/game/Entities/Player/Player.cpp src/server/game/Handlers/CharacterHandler.cpp src/server/game/Tools/PlayerDump.cpp
git commit -m "feat(engine,sql): store class hall shipments"
```

Run `build.sh` (long; covers Task 1) — expected `none`, `BUILD OK`.
Run `boot_check.sh` — expected `READY`, both updates applied, growth equal to the baseline, `none`.

- [ ] **Step 5: Plant the test rows**

With Gairy offline: the hall row `(1, 560)` and these orders for guid 1, with `T` the time of planting:

| shipmentId | charShipmentId | creationTime | duration | note |
|------------|----------------|--------------|----------|------|
| 1 | 275 | T - 100 | 1800 | being made |
| 2 | 275 | T + 1700 | 1800 | waits for the first |
| 3 | 999999 | T | 1800 | unknown shipment |
| 4 | 370 | T | 43200 | gives no troop |

- [ ] **Step 6: [user] Login A — the planted orders, and the discoveries for later tasks**

Before the login, start `rotate_opcode.py cands_a.txt` with these candidates:

```
1: 0x27DA open, container 149         | 10202 itsguid uint32 149
2: 0x27DB info, 6.x, none pending     | 10203 uint8 128 uint32 275 uint32 4 uint32 0 uint32 0
3: 0x27DB info, one more value (7)    | 10203 uint8 128 uint32 275 uint32 4 uint32 0 uint32 7 uint32 0
4: 0x27DB info, 6.x, one pending      | 10203 uint8 128 uint32 275 uint32 4 uint32 1 uint32 0 uint32 275 uint64 1 uint64 0 uint32 {now} uint32 1800 uint32 0
5: 0x27DD create reply                | 10205 uint64 2 uint32 275 uint32 0
6: 0x27E0 report, with leading value  | 10208 uint32 0 uint32 1 uint32 275 uint64 1 uint64 0 uint32 {now} uint32 1800 uint32 0
7: 0x27E0 report, without it          | 10208 uint32 1 uint32 275 uint64 1 uint64 0 uint32 {now} uint32 1800 uint32 0
8: 0x27DF of type, guess, one ready   | 10207 uint32 149 uint32 1 uint32 275 uint64 1 uint64 0 uint32 {now} uint32 1 uint32 0
9: 0x27DE complete reply, guess       | 10206 uint64 1 uint32 0
```

In game:

1. An event printer, two lines:

```
/run SF=CreateFrame("Frame") SF:SetScript("OnEvent",function(_,e,...) print(e,...) end)
```
```
/run for _,e in ipairs({"SHIPMENT_CRAFTER_OPENED","SHIPMENT_CRAFTER_INFO","SHIPMENT_UPDATE","SHIPMENT_CRAFTER_CLOSED","GARRISON_LANDINGPAGE_SHIPMENTS","GARRISON_SHIPMENT_RECEIVED"}) do SF:RegisterEvent(e) end
```

2. `.modify currency 1220 200`, `.npc add temp 106451`, target her.
3. `.debug send opcode` (candidate 1). Expected: the order window opens and chat prints `SHIPMENT_CRAFTER_OPENED 149`.
4. `.debug send opcode` three times (candidates 2, 3, 4). After each: what chat printed for `SHIPMENT_CRAFTER_INFO`, a screenshot of the window, and `/dump C_Garrison.GetNumPendingShipments()`.
5. Click the window's order button once, then `.debug send opcode` (candidate 5). What chat printed.
6. `/run C_Garrison.RequestLandingPageShipmentInfo()`, then `.debug send opcode` twice (candidates 6, 7); after each `/dump C_Garrison.GetFollowerShipments(3)` and `/dump C_Garrison.GetLandingPageShipmentInfoByContainerID(149)`.
7. `.gobject add temp 250909 3600`. A screenshot of the object; click it. Then `.debug send opcode` (candidate 8), look and click again; then `.debug send opcode` (candidate 9) and what chat printed.
8. `.save`, log out.

Afterwards:

```bash
python3 ~/class-hall-shipments/pkt.py ~/arguscore-server/logs/shipments.pkt 32DD 32E0
grep -c "class hall shipment" ~/arguscore-server/logs/Server.log
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT shipmentId, charShipmentId, creationTime, duration FROM characters.character_class_hall_shipment WHERE guid=1 ORDER BY shipmentId" 2>/dev/null
```

and list every client packet within five seconds of each click on the object (`pkt.py` over the whole range, by tick).

Expected: orders 1 and 2 unchanged and 3 and 4 dropped by the save; two error lines; the client's requests 0x32DE, 0x32E0 and 0x32DF with readable layouts.

Rulings to write before Task 3:

- the layout of the info reply, and whether the "owned" count comes from the packet;
- the layout of the report reply;
- how the click on the object arrives (expected: `CMSG_GAME_OBJ_USE`), and what the object needs to be shown and clicked;
- the layout of the complete reply, if a candidate did something visible. If none did, Task 6 sends `uint64` order id, `uint32` result, and the ruling says it is unverified.

If the window did not open from the hand-sent packet, the info and create probes move to login B1, after Task 4 has opened it properly, and Task 3's info reply is first written with the 6.x layout; record the ruling.

- [ ] **Step 7: A character without a hall; deletion**

As in the followers run: plant one order row for the next character guid, start a poller on that row and the character, and ask the user to create `Shipmenttest`, log in, `.save`, log out and delete it.
Expected: the row is untouched through login and save, and gone after the deletion.

---

### Task 3: Add the shipment packets

**Files:**
- Modify: `src/server/game/Server/Protocol/Opcodes.cpp`
- Modify: `src/server/game/Server/Packets/GarrisonPackets.h`, `GarrisonPackets.cpp`

**Interfaces:**
- Produces, in `WorldPackets::Garrison` (layouts from the ledger where Task 2 Step 6 settled them):

```cpp
struct GarrisonShipment
{
    int32 ShipmentRecID = 0;
    uint64 ShipmentID = 0;
    uint64 AssignedFollowerDBID = 0;
    time_t CreationTime = 0;
    int32 ShipmentDuration = 0;
    int32 BuildingTypeID = 0;
};

class OpenShipmentNpcFromGossip final : public ServerPacket       // SMSG_OPEN_SHIPMENT_NPC_FROM_GOSSIP
{
    WorldPacket const* Write() override;
    ObjectGuid NpcGUID;
    int32 CharShipmentContainerID = 0;
};

class GarrisonRequestShipmentInfo final : public ClientPacket     // CMSG_GARRISON_REQUEST_SHIPMENT_INFO
{
    void Read() override;
    ObjectGuid NpcGUID;
};

class GetShipmentInfoResponse final : public ServerPacket         // SMSG_GET_SHIPMENT_INFO_RESPONSE
{
    WorldPacket const* Write() override;
    bool Success = false;
    int32 ShipmentRecID = 0;
    int32 MaxShipments = 0;
    std::vector<GarrisonShipment> Shipments;
    int32 PlotInstanceID = 0;           // plus the owned count, if the ledger says the packet carries it
};

class CreateShipment final : public ClientPacket                  // CMSG_CREATE_SHIPMENT
{
    void Read() override;
    ObjectGuid NpcGUID;
    uint32 Count = 0;
};

class CreateShipmentResponse final : public ServerPacket          // SMSG_CREATE_SHIPMENT_RESPONSE
{
    WorldPacket const* Write() override;
    uint64 ShipmentID = 0;
    uint32 ShipmentRecID = 0;
    uint32 Result = 0;
};

class CompleteShipmentResponse final : public ServerPacket        // SMSG_COMPLETE_SHIPMENT_RESPONSE
{
    WorldPacket const* Write() override;
    uint64 ShipmentID = 0;
    uint32 Result = 0;
};

class GarrisonRequestLandingPageShipmentInfo final : public ClientPacket
{
    void Read() override { }
};

class GarrisonLandingPageShipmentInfo final : public ServerPacket // SMSG_GARRISON_LANDING_PAGE_SHIPMENT_INFO
{
    WorldPacket const* Write() override;
    uint32 CurrentArtifactKnowledge = 0;
    std::vector<GarrisonShipment> Shipments;
};
```

- [ ] **Step 1: Opcodes**

`Opcodes.cpp`: `STATUS_NEVER` in place of `STATUS_UNHANDLED` for the five replies this plan sends (0x27DA, 0x27DB, 0x27DD, 0x27DE, 0x27E0). No opcode is renamed; the handlers are set in Tasks 4, 5 and 7.

- [ ] **Step 2: Packet classes**

Follow `GarrisonResearchTalent` and `GarrisonResearchTalentResult`. The time is written as `GarrisonTalent::ResearchStartTime` is.

- [ ] **Step 3: Commit**

```bash
git add src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Server/Packets/GarrisonPackets.h src/server/game/Server/Packets/GarrisonPackets.cpp
git commit -m "feat(engine): add the shipment packets"
```

No build here; Tasks 4 to 7 share it.

---

### Task 4: Open a class hall shipment from its gossip option

**Files:**
- Create: `sql/updates/world/master/<date>_00_world.sql`
- Modify: `src/server/game/Globals/ObjectMgr.h:748`, `ObjectMgr.cpp:9766`
- Modify: `src/server/game/Entities/Player/Player.cpp:13645` (`Player::OnGossipSelect`)
- Modify: `src/server/game/ClassHall/ClassHallShipments.h`, `ClassHallShipments.cpp`, `ClassHallFollowers.h`
- Modify: `src/server/game/Server/WorldSession.h`, `src/server/game/Server/Protocol/Opcodes.cpp`, `src/server/game/Handlers/GarrisonHandler.cpp`

**Interfaces:**
- Consumes: the packets of Task 3; `ClassHallFollowers::CanAddFollower`.
- Produces:

```cpp
// GossipMenuItemAddon
Optional<int32> CharShipmentID;

// ClassHallFollowers: made public, unchanged
GarrisonError CanAddFollower(GarrFollowerEntry const* follower) const;
uint32 GetClassSpec(GarrFollowerEntry const* follower) const;
uint32 CountTroops(uint32 garrClassSpecId) const;

// ClassHallShipments
void OpenNpc(ObjectGuid npcGuid, uint32 charShipmentId);
void SendShipmentInfo(ObjectGuid npcGuid) const;
GarrisonError CanOfferShipment(CharShipmentEntry const* shipment) const;

// private
ObjectGuid _openNpc;
CharShipmentEntry const* _openShipment = nullptr;
```

- `void WorldSession::HandleGarrisonRequestShipmentInfo(WorldPackets::Garrison::GarrisonRequestShipmentInfo& garrisonRequestShipmentInfo)`

- [ ] **Step 1: The column and its loading**

```sql
ALTER TABLE `gossip_menu_option_addon` ADD COLUMN `CharShipmentID` INT DEFAULT NULL AFTER `GarrTalentTreeID`;
```

`LoadGossipMenuItemAddon` reads it as it reads `GarrTalentTreeID`: a value naming no `CharShipment` row is logged to `sql.sql` and reset.

- [ ] **Step 2: `CanOfferShipment`**

| Check | Error |
|-------|-------|
| `IsTroopShipment(shipment)` (which is false for null) | `GARRISON_ERROR_OPERATION_NOT_SUPPORTED` |
| `CanAddFollower` of the shipment's follower is `GARRISON_SUCCESS` or `GARRISON_ERROR_AT_CLASS_SPEC_CAP` | the error `CanAddFollower` gave |

- [ ] **Step 3: `OpenNpc` and the gossip hook**

`OpenNpc`: when `CanOfferShipment` succeeds, remember the NPC and the shipment and send `OpenShipmentNpcFromGossip` with the shipment's `ContainerID`. Otherwise send nothing and change nothing.

`Player::OnGossipSelect` gets `case GossipOptionNpc::ShipmentCrafter:` beside the talent case: with a class hall and an addon that has `CharShipmentID`, call `GetShipments().OpenNpc(guid, *itemAddon->CharShipmentID)`.

- [ ] **Step 4: `SendShipmentInfo` and its handler**

`SendShipmentInfo`: nothing unless `npcGuid` is `_openNpc`. The reply has `Success` true, the remembered shipment's id and `MaxShipments`, the pending orders of its container ordered by creation time (follower and building type 0), plot 0, and the owned count (`CountTroops(GetClassSpec(follower))`) if the ledger says the packet carries it.

`HandleGarrisonRequestShipmentInfo`: return unless `_player->GetNPCIfCanInteractWith(NpcGUID, UNIT_NPC_FLAG_NONE, UNIT_NPC_FLAG_2_SHIPMENT_CRAFTER)`; with a class hall call `SendShipmentInfo`; without one do nothing. `Opcodes.cpp`: `STATUS_LOGGEDIN`, `PROCESS_THREADUNSAFE`.

- [ ] **Step 5: Commit**

```bash
git add sql/updates/world src/server/game/Globals/ObjectMgr.h src/server/game/Globals/ObjectMgr.cpp src/server/game/Entities/Player/Player.cpp src/server/game/ClassHall src/server/game/Server/WorldSession.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine,sql): open a class hall shipment from its gossip option"
```

---

### Task 5: Order a class hall shipment

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallShipments.h`, `ClassHallShipments.cpp`
- Modify: `src/server/game/Spells/Spell.h` (one declaration), `Spell.cpp` (`Spell::CheckCast`), `SpellEffects.cpp` (the table entry for 216 and the new effect)
- Modify: `src/server/game/Server/WorldSession.h`, `src/server/game/Server/Protocol/Opcodes.cpp`, `src/server/game/Handlers/GarrisonHandler.cpp`

**Interfaces:**
- Consumes: `CanOfferShipment`, `_openNpc`, `_openShipment` (Task 4); `CreateShipment`, `CreateShipmentResponse` (Task 3); `CountTroops`, `GetClassSpec`.
- Produces:

```cpp
// ClassHallShipments
GarrisonError CanCreateShipment(CharShipmentEntry const* shipment) const;
uint32 GetAvailableShipments(CharShipmentEntry const* shipment) const;
void RequestShipments(ObjectGuid npcGuid, uint32 count);
void CreateShipment(CharShipmentEntry const* shipment);
static SpellCastResult CheckCreateShipmentCast(Player const* caster, uint32 charShipmentId);

// Spell
void EffectCreateShipment();
```

- `void WorldSession::HandleCreateShipment(WorldPackets::Garrison::CreateShipment& createShipment)`

- [ ] **Step 1: The capacity rule**

`GetAvailableShipments`: `MaxShipments` minus the troops owned of the follower's kind (`CountTroops(GetClassSpec(follower))`) minus the pending orders in the shipment's container; never below 0.

`CanCreateShipment`: the error of `CanOfferShipment` if it has one; `GARRISON_ERROR_AT_CLASS_SPEC_CAP` when `GetAvailableShipments` is 0; otherwise `GARRISON_SUCCESS`.

- [ ] **Step 2: The request**

`RequestShipments`: nothing unless `npcGuid` is `_openNpc` and a shipment is remembered. When `CanCreateShipment` fails, send `CreateShipmentResponse` with order id 0, the shipment id and the error, and stop. Otherwise cast the shipment's `SpellID` on the owner, not triggered, `min(count, GetAvailableShipments)` times, stopping at the first cast that does not return `SPELL_CAST_OK`.

`HandleCreateShipment`: the NPC check of Task 4 Step 4, then `RequestShipments`.

- [ ] **Step 3: The spell check**

`CheckCreateShipmentCast`: `SPELL_CAST_OK` when the shipment is unknown or its container is not a class hall container (not this code's business). Otherwise `SPELL_FAILED_NO_BUILDING_FOR_SHIPMENT` without a class hall; and from `CanCreateShipment`: success → `SPELL_CAST_OK`, `AT_CLASS_SPEC_CAP` → `SPELL_FAILED_SHIPMENTS_FULL`, anything else → `SPELL_FAILED_NO_SHIPMENTS_FOR_CONTAINER`.

`Spell::CheckCast` gets `case SPELL_EFFECT_CREATE_SHIPMENT:` in its effect switch: for a player caster, return the result of `CheckCreateShipmentCast(playerCaster, spellEffectInfo.MiscValue)` when it is not `SPELL_CAST_OK`.

- [ ] **Step 4: The effect**

`Spell::EffectCreateShipment`, shaped as `EffectAddGarrisonFollower`: for a player target whose shipment (`effectInfo->MiscValue`) is in a class hall container and who has a class hall, call `GetShipments().CreateShipment(shipment)`. Anything else does nothing, as today.

`CreateShipment`: return when `CanCreateShipment` fails. The new order gets `_nextShipmentId++`, the shipment's `Duration`, and as creation time the later of now and the finish time of the container's last pending order. Send `CreateShipmentResponse` with its id, the shipment id and `GARRISON_SUCCESS`.

- [ ] **Step 5: Commit**

```bash
git add src/server/game/ClassHall src/server/game/Spells/Spell.h src/server/game/Spells/Spell.cpp src/server/game/Spells/SpellEffects.cpp src/server/game/Server/WorldSession.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine): order a class hall shipment"
```

---

### Task 6: Collect finished class hall shipments

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallShipments.h`, `ClassHallShipments.cpp`, `ClassHallFollowers.h`, `ClassHallFollowers.cpp`
- Modify: `src/server/game/Entities/GameObject/GameObject.cpp` (`GameObject::Use`), or the hook the ledger's ruling names

**Interfaces:**
- Consumes: `CompleteShipmentResponse` (Task 3); the ledger's rulings on how the click arrives and what the object needs.
- Produces:

```cpp
// ClassHallFollowers: return type changed from void
GarrisonError AddFollower(uint32 garrFollowerId);

// ClassHallShipments
void CompleteShipments(uint32 charShipmentContainerId);
```

- [ ] **Step 1: `AddFollower` returns its result**

The value it already sends in its reply. `Spell::EffectAddGarrisonFollower` ignores it.

- [ ] **Step 2: `CompleteShipments`**

Take the ready orders of the container, ordered by creation time. For each: call the hall's `GetFollowers().AddFollower(GarrFollowerID)`; when it is not `GARRISON_SUCCESS`, stop. Otherwise erase the order, send `CompleteShipmentResponse` with its id and `GARRISON_SUCCESS`, and cast `OnCompleteSpellID` on the owner as a triggered spell when it is not 0.

- [ ] **Step 3: The hook**

`GameObject::Use` gets `case GAMEOBJECT_TYPE_GARRISON_SHIPMENT:`, shaped as the artifact forge case: for a player who meets the object's interact condition and has a class hall, call `GetShipments().CompleteShipments(GetGOInfo()->garrisonShipment.ShipmentContainer)`.

If the ledger says the click arrives as a request of its own, add that opcode with its number from the packet log and a handler that finds the object with `GetGameObjectIfCanInteractWith` and makes the same call. If the ledger says the object needs the character's orders to be shown or clicked, add that reply (0x27DF, `STATUS_NEVER`) with the layout the probe confirmed and send it where the ledger says the client expects it.

- [ ] **Step 4: Commit**

```bash
git add src/server/game/ClassHall src/server/game/Entities/GameObject/GameObject.cpp
git commit -m "feat(engine): collect finished class hall shipments"
```

---

### Task 7: Report the class hall's shipments

**Files:**
- Modify: `src/server/game/ClassHall/ClassHallShipments.h`, `ClassHallShipments.cpp`
- Modify: `src/server/game/Server/WorldSession.h`, `src/server/game/Server/Protocol/Opcodes.cpp`, `src/server/game/Handlers/GarrisonHandler.cpp`

**Interfaces:**
- Consumes: `GarrisonRequestLandingPageShipmentInfo`, `GarrisonLandingPageShipmentInfo` (Task 3).
- Produces: `void ClassHallShipments::SendLandingPageShipmentInfo() const`; `void WorldSession::HandleGarrisonRequestLandingPageShipmentInfo(WorldPackets::Garrison::GarrisonRequestLandingPageShipmentInfo& garrisonRequestLandingPageShipmentInfo)`.

- [ ] **Step 1: The reply and its handler**

`SendLandingPageShipmentInfo`: all pending orders, ordered by container and creation time, artifact knowledge 0. The handler calls it for a character with a class hall and does nothing otherwise.

- [ ] **Step 2: Commit, build, boot**

```bash
git add src/server/game/ClassHall src/server/game/Server/WorldSession.h src/server/game/Server/Protocol/Opcodes.cpp src/server/game/Handlers/GarrisonHandler.cpp
git commit -m "feat(engine): report the class hall's shipments"
```

Run `build.sh` (long; covers Tasks 3 to 7) — expected `none`, `BUILD OK`.
Run `boot_check.sh` — expected `READY`, the world update applied, baseline growth, `none`.

- [ ] **Step 3: Plant the test menu and boot again**

The column exists now. With no character online, plant (ledger: REVERT at cleanup):

- `gossip_menu` row `(9000001, <an existing npc_text id>)` and `creature_template_gossip` row `(106451, 9000001)`;
- `gossip_menu_option` rows for menu 9000001, all with `OptionNpc` 28, and `gossip_menu_option_addon` rows naming their shipment:

| OptionID | OptionText | CharShipmentID |
|----------|------------|----------------|
| 0 | Acolytes | 275 |
| 1 | Acolytes (tutorial) | 333 |
| 2 | Zealots | 276 |
| 3 | Armaments (no troop) | 370 |
| 4 | Monk troop | 249 |
| 5 | Unknown shipment | 999999 |

Remove the orders left from Task 2 for guid 1. Run `boot_check.sh` again.
Expected: one new `sql.sql` error naming menu 9000001 option 5 and shipment 999999; nothing else new.

---

### Task 8: Test the loop in game

State before login B1: the hall row; no orders; no followers; the menu of Task 7 Step 3.

- [ ] **Step 1: [user] Login B1**

The event printer of Task 2 Step 6, then `.modify currency 1220 300`, `.quest add 43275`, `.npc add temp 106451`, `.gobject add temp 250909 3600`.

1. Talk to her. Expected: six options.
2. "Acolytes (tutorial)". Expected: the window opens for Acolytes and offers one order. Order it. Expected: `/dump (select(2,GetCurrencyInfo(1220)))` is 280. (Task 4, Task 5)
3. After five seconds, click the object. Expected: an Acolyte in the follower list without a relog, and the objective "Train Acolytes" ticked. (Task 6)
4. "Acolytes". Expected: the window offers three orders (four, less the one owned). Order two. Expected: 240. (Task 5)
5. Walk fifty yards away, then `/run C_Garrison.RequestShipmentCreation(1)`. Expected: still 240. (Review Focus 3. If the packet log shows the client sent nothing, record that this is covered by review.)
6. Walk back, choose "Acolytes" again, then `/run C_Garrison.RequestShipmentCreation(1000)`. Expected: 220, and the window offers none. (Review Focus 1, 2)
7. `.cast self 210957`. Expected: refused with a spell error; still 220. (Review Focus 1)
8. "Armaments (no troop)", "Monk troop", "Unknown shipment". Expected: no window for any of them. (Task 4)
9. Open the hall report at the minimap. Expected: "Troops Training" with none ready of three and a timer. A screenshot. (Task 7)
10. "Zealots". Expected: the window for Zealots; order one; 135.
11. `.save`, log out.

- [ ] **Step 2: Read the results, then age the orders**

```bash
python3 ~/class-hall-shipments/pkt.py ~/arguscore-server/logs/shipments.pkt 32DD 32E0 27DA 27E0
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT shipmentId, charShipmentId, creationTime, duration FROM characters.character_class_hall_shipment WHERE guid=1 ORDER BY shipmentId; SELECT followerId, COUNT(*) FROM characters.character_class_hall_follower WHERE guid=1 GROUP BY followerId" 2>/dev/null
```

Expected: create replies with result 0 for each order made and none for items 5 and 7; three orders of shipment 275 whose creation times are 1800 seconds apart, one of 276; one follower 677.

Then, with Gairy offline: `UPDATE characters.character_class_hall_shipment SET creationTime = creationTime - 7200 WHERE guid = 1`.

- [ ] **Step 3: [user] Login B2**

`.npc add temp 106451`, `.gobject add temp 250909 3600`, `.gobject add temp 250910 3600`.

1. The hall report. Expected: three of three ready for the Acolytes, one of one for the Zealots.
2. Click the Acolytes' object. Expected: two more Acolytes (three in all) and whatever the client says about the third. (Review Focus 4)
3. Click it again. Expected: nothing changes.
4. Click the Zealots' object. Expected: a Zealot in the list.
5. `.save`, log out, close the client.

```bash
docker exec arguscore-db-1 mysql -uarguscore -parguscore -N -e "SELECT shipmentId, charShipmentId FROM characters.character_class_hall_shipment WHERE guid=1; SELECT followerId, COUNT(*) FROM characters.character_class_hall_follower WHERE guid=1 GROUP BY followerId" 2>/dev/null
```

Expected: one order of shipment 275 left; followers 677 three times and 679 once.

---

### Task 9: Land it

- [ ] **Step 1: Final review**

Dispatch one fresh reviewer on the most capable model over `engine/class-hall-followers..engine/class-hall-shipments`, with this plan, the spec, the Review Focus section and the ledger's rulings. The test setup stays in place until it reports. Critical and Important findings get one fix pass, each fix rebuilt and rechecked in game; Minor findings go to the ledger as deferred.

- [ ] **Step 2: Clean up**

With every character offline: remove every planted row listed in the ledger (the menu 9000001 rows in four world tables; creature 106451's `npcflag` back to the recorded value), set `PacketLogFile` back to empty, remove `shipments.pkt` and `opcode.txt`. Put Gairy back exactly as the backup of Task 0 Step 4 has her, with `dbdiff.py` against the backup loaded into a scratch database, and show that no table but the client settings differs. Drop the scratch database. Restart the server clean.

- [ ] **Step 3: Merge**

With no build running, each merge in a temporary worktree except the last:

```bash
cd ~/arguscore
git worktree add /tmp/wt-dev dev && git -C /tmp/wt-dev merge --no-ff --no-edit engine/class-hall-shipments && git worktree remove /tmp/wt-dev
git worktree add /tmp/wt-art feature/priest-artifact-weapon && git -C /tmp/wt-art merge --no-edit dev && git worktree remove /tmp/wt-art
git switch feature/priest-class-hall && git merge --no-edit dev
git diff --stat integration/class-hall-shipments feature/priest-class-hall -- src sql
git branch -D integration/class-hall-shipments
```

Expected: the `git diff` prints nothing.

- [ ] **Step 4: Update the docs and push**

On `dev`, in a temporary worktree: add `engine/class-hall-shipments` to "Current branches" in `docs/BRANCHING.md` and replace the spec's "Open points" with what the tests showed, as one `chore(local)` commit. Merge `dev` into both priest branches again. Push `engine/class-hall-shipments`, `dev`, `feature/priest-artifact-weapon` and `feature/priest-class-hall` (rule 13).

- [ ] **Step 5: Report**

State which checks passed and which did not; every `Ruling:` line with what it costs if wrong; every deferred minor; the answer to each open point of the spec; which branch `~/arguscore` is on. Update the class hall memory file.
