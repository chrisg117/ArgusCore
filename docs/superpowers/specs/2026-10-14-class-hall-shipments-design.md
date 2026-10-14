# Class hall shipments: troop orders

Date: 2026-10-14
Branch: `engine/class-hall-shipments`, stacked on `engine/class-hall-followers`
Status: built and tested in game on 2026-10-14; landed in `dev`
Builds on: `2026-10-12-class-hall-foundation-design.md`,
`2026-10-13-class-hall-talents-design.md`,
`2026-10-13-class-hall-followers-design.md`

## Goal

A character with a class hall can order troops from a recruiter, pay for
them in Order Resources, see the orders in the order window and in the hall
report, and collect a finished troop from the object beside the recruiter.
The collected troop becomes a follower through the followers piece.

This is the fourth of five engine pieces for class halls. Missions come
after it.

Two words used throughout, as the client data uses them:

- A **shipment** is one kind of order (`CharShipment.db2`): what it costs,
  how long it takes and what it gives. "Acolytes" is a shipment.
- A **container** groups the shipments one place handles
  (`CharShipmentContainer.db2`). The priest's Acolyte recruiter and the
  object beside her share one container.

## Decisions

These are proposed by this draft and are for the owner to confirm:

- **Scope is troop orders**: shipments that give a follower. That is 88 of
  the 137 class hall shipments. The other class hall shipments (champion
  armaments, Seals of Broken Fate, artifact research notes, cooking
  research, the world quest call-ins) give an item or a treasure, which is
  a different mechanism; they are left out.
- **A separate `ClassHallShipments` class in its own files, owned by the
  hall,** with its own table. This is the decision made for talents and
  followers, applied again.
- **Which shipment an NPC offers is content, set on its gossip option.**
  The talent NPC already works this way (`GarrTalentTreeID` on the gossip
  option). The engine adds the column and the handling; the rows are
  content.
- **An order is made by casting the shipment's own spell.** The spell
  carries the cost, so the existing spell code checks and takes the Order
  Resources. Nothing about the cost is written into the core.
- **Orders of one container are made one after another**, as Warlords work
  orders were: the second starts when the first is done. No source I may
  use says this for Legion troops; see "Open points".
- **Retail values, no server settings.**

## What the client data says

`CharShipmentContainer.db2` has 134 containers, 87 of them for the class
hall (`GarrTypeID` 3). Fields used here: `ID`, `GarrTypeID`,
`BaseCapacity`. The rest (texts, display ids, thresholds) are for the
client.

`CharShipment.db2` has 255 shipments, 137 of them in class hall containers.
Fields: `ID`, `ContainerID`, `SpellID`, `Duration` (seconds),
`GarrFollowerID`, `MaxShipments`, `OnCompleteSpellID`, `TreasureID`,
`DummyItemID`, `Flags`.

The priest's troop shipments:

| Shipment | Container | Gives follower | Time | Max | Order spell | Cost | On-complete spell |
|----------|-----------|----------------|------|-----|-------------|------|-------------------|
| 275 Acolytes | 149 | 677 | 30 min | 4 | 210957 | 20 | 210901 |
| 332 Paragons | 149 | 678 | 30 min | 4 | 219886 | 20 | 219887 |
| 333 Acolytes, tutorial | 149 | 677 | 5 s | 1 | 219888 | 20 | 219889 |
| 276 Zealots | 150 | 679 | 30 min | 3 | 210958 | 85 | 210902 |
| 334 Dark Zealots | 150 | 920 | 30 min | 3 | 219903 | 85 | 219905 |
| 436 Hooded Priests | 231 | 1042 | 30 min | 2 | 239367 | 40 | 239368 |

- **The order spell** has one effect of type 216 ("create shipment") whose
  value is the shipment id, no cast time, and a cost in currency 1220
  (Order Resources) in `SpellReagentsCurrency.db2`.
- **The on-complete spell** gives a kill credit (effect 90). For 210901
  and 219889 the credit is creature 106404, which is the objective "Train
  Acolytes" of the priest quest "Recruiting the Troops" (43275).
- **The follower comes from the shipment row**, not from a spell. The
  on-complete spells have no "add follower" effect.
- **`MaxShipments` is one above the troop kind's limit** for the two basic
  kinds: Acolytes 4 against a limit of 3, Zealots 3 against 2. Hooded
  Priests are 2 against 2. The tutorial shipment is 1.
- No shipment gives a class-less troop (the Argus troops, followers 1055
  to 1060). They are not part of this piece.

The client's order window (`Blizzard_GarrisonCapacitiveDisplay.lua`) works
out how many can be ordered as: the maximum the server sent, minus the
pending orders, minus an "owned" count, limited by what the character can
pay. It reads the cost from the order spell itself.

The client's hall report (`Blizzard_GarrisonLandingPage.lua`) lists the
containers that have pending follower shipments, each with a count of
ready and total and a timer.

## What the core already has

- The layouts of both `.db2` files in `DB2Metadata.h`. Neither is loaded.
- Opcode numbers, none handled: `CMSG_OPEN_SHIPMENT_NPC` (0x32DD),
  `CMSG_GARRISON_REQUEST_SHIPMENT_INFO` (0x32DE),
  `CMSG_GARRISON_REQUEST_LANDING_PAGE_SHIPMENT_INFO` (0x32DF),
  `CMSG_CREATE_SHIPMENT` (0x32E0),
  `SMSG_OPEN_SHIPMENT_NPC_FROM_GOSSIP` (0x27DA),
  `SMSG_GET_SHIPMENT_INFO_RESPONSE` (0x27DB),
  `SMSG_OPEN_SHIPMENT_NPC_RESULT` (0x27DC),
  `SMSG_CREATE_SHIPMENT_RESPONSE` (0x27DD),
  `SMSG_COMPLETE_SHIPMENT_RESPONSE` (0x27DE),
  `SMSG_GET_SHIPMENTS_OF_TYPE_RESPONSE` (0x27DF),
  `SMSG_GARRISON_LANDING_PAGE_SHIPMENT_INFO` (0x27E0).
- Spell effects 216 ("create shipment") and 248 ("finish shipment"), with
  no handler, and the spell failures "shipments full" (261) and three
  more (262 to 264).
- Spell costs in currency: loaded, checked and taken by the spell code.
- Gossip option type 28 ("shipment crafter"), shown in menus; choosing it
  does nothing. The table `gossip_menu_option_addon` holds the talent tree
  of a type 32 option.
- The NPC flag "shipment crafter" and game object type 45 ("garrison
  shipment", with its container). Using such an object does nothing.
- In the world database: 29 gossip options of type 28 taken from retail,
  among them the mage recruiter's ("I would like to requisition a Squad of
  Apprentices" and "... of Conjurors", two options on one menu) and the
  cooking NPC's (five options, one per recipe, all in one container). The
  objects "Training Troops" 250909 (container 149) and 250910 (container
  150) exist as templates. The priest recruiters, Vicar Eliza (106451) and
  Grand Anchorite Gesslar (106450), exist as templates with no gossip, no
  flags and no spawn.
- From the followers piece: adding a follower with its limits, and the
  count of troops of a kind.

Missing: both stores, storage, the gossip handling, the three request
handlers, the spell effect, and the object's use.

## Packets

| Packet | Layout | Source |
|--------|--------|--------|
| 0x27DA open from gossip | packed NPC guid, `uint32` container | WowPacketParser |
| 0x32DE request info | packed NPC guid | WowPacketParser |
| 0x27DB info reply | bit "success", `int32` shipment, `int32` maximum, `int32` count, `int32` plot, then per pending order: `int32` shipment, `uint64` order id, `uint64` follower, time created, `int32` duration, `int32` building type | WowPacketParser, written for 6.x |
| 0x32E0 create | packed NPC guid, `uint32` count | WowPacketParser |
| 0x27DD create reply | `uint64` order id, `uint32` shipment, `uint32` result | WowPacketParser |
| 0x32DF request report | empty | WowPacketParser |
| 0x27E0 report reply | `uint32` artifact knowledge (7.2 on), `uint32` count, then the same per-order entry | WowPacketParser |
| 0x27DE complete reply | not known; expected `uint64` order id, `uint32` result | to find |
| 0x27DF shipments of type | not known | to find |
| 0x27DC open result, 0x32DD open | 0x32DD is a packed NPC guid; 0x27DC not known | not used, see below |

The info reply's layout is from 6.x. The 7.x window reads one value more
(the "owned" count), so the layout may have grown. It is settled in game
before the packet classes are written.

## Scope

In scope:

- Loading `CharShipment.db2` and `CharShipmentContainer.db2`.
- `ClassHallShipments`, its table, and its place in the hall's load, save,
  delete and the character dump.
- Opening the order window from a gossip option.
- Answering the window's request for the pending orders.
- Ordering: the request, the spell effect, and the check that refuses an
  order before anything is paid.
- Collecting finished orders from the object: the follower, the
  on-complete spell, and the reply.
- The hall report's list of orders.

Out of scope:

- Shipments that give no follower.
- Warlords work orders. Requests from a character without a class hall are
  left as they are today (ignored).
- Spell effect 248 ("finish shipment") and anything else that shortens an
  order.
- Hall talents that change costs, times or limits.
- Choosing among look-alike troops. A troop gets the follower id its
  shipment names (677 for Acolytes); the other followers of the kind (921
  to 926) are not chosen from.
- The criteria type "collect garrison shipment" (182) and the three
  criteria conditions about shipments. The core marks them as not done;
  the class hall quests use the kill credit instead.
- Opening the window without a gossip option (`CMSG_OPEN_SHIPMENT_NPC` and
  its reply 0x27DC). Retail's class hall recruiters use gossip options.
- The recruiters, their gossip, the objects, and the conditions that
  decide which option a character sees (content, on
  `feature/priest-class-hall`).
- GM commands or other test helpers.

## Rules this work follows

- Engine commits contain core code and schema SQL only.
- New code goes in new files. Shared files get small hook changes only.
- No warnings, no debug code, no TODO comments.
- Small commits, each booted and tested in game before the next.
- SQL files are named by the server's date, which is UTC.
- This document lives on `dev`, never on the engine branch.
- Test rows are planted by hand, never committed, and removed at the end.
  The test character is put back exactly as it was.

## Design

### 1. Stores

Load `CharShipment.db2` and `CharShipmentContainer.db2` like any other
store, each with its hotfix table. The container has two texts, so it also
gets a locale table.

### 2. `ClassHallShipments`

New files: `src/server/game/ClassHall/ClassHallShipments.h` and `.cpp`.

It belongs to one `ClassHall` and holds the character's pending orders by
their id. For each: the shipment row, when it was created, and how long it
takes. An order is ready when its creation time plus its duration has
passed. Nothing ticks; readiness is worked out when asked.

It also remembers, for the current session only, which NPC's order window
is open and for which shipment. The later requests name only the NPC.

`ClassHall` owns one and forwards load, save and delete to it.

### 3. Storage

One new table, `character_class_hall_shipment`:

| Column | Type | Meaning |
|--------|------|---------|
| `guid` | bigint unsigned | the character |
| `shipmentId` | bigint unsigned | the order's id, unique for this character |
| `charShipmentId` | int unsigned | `CharShipment` id |
| `creationTime` | bigint | when the order starts being made (unix time) |
| `duration` | int unsigned | seconds, as it was when ordered |

Primary key `(guid, shipmentId)`.

The order id only has to be unique for the character: the client never
sees another character's orders. A new order gets one more than the
highest id the character has had this session. This is unlike followers,
whose ids are shared across characters, and it means the table can go in
the character dump as it is.

The duration is stored because it is sent with each order and belongs to
the order: a later change of the data must not move orders already paid
for.

Saving deletes the character's rows and inserts the current ones, inside
the player's save transaction, as talents and followers do. The table has
no database link to `character_class_hall`. Character deletion removes its
rows.

### 4. Opening the order window

A new column `CharShipmentID` on `gossip_menu_option_addon` names the
shipment a gossip option of type 28 offers. A row naming a shipment that
does not exist is logged at startup and ignored.

When a player chooses such an option, the hall is asked to open it. If the
character could ever order this shipment (see the first two checks in
section 6), the hall remembers the NPC and the shipment and sends
`SMSG_OPEN_SHIPMENT_NPC_FROM_GOSSIP` with the NPC and the shipment's
container. Otherwise nothing is sent.

One option names one shipment because retail did it that way: the mage
recruiter has an option per troop kind and the cooking NPC an option per
recipe within one container. Which option a character sees (basic troop,
upgraded troop, tutorial order) is decided by the conditions content puts
on the options, with the gossip conditions the core already has.

### 5. The window's request

`CMSG_GARRISON_REQUEST_SHIPMENT_INFO` names the NPC. It is answered when
the NPC is the one remembered, has the gossip flag, and is in reach.
The reply has the remembered shipment, its capacity (section 6), and the
character's pending orders in that shipment's container, oldest first.

### 6. Ordering

`CMSG_CREATE_SHIPMENT` names the NPC and a count. The NPC is checked as in
section 5.

An order may be made when all of these hold. The first failure is the
answer.

| Check | Error |
|-------|-------|
| The shipment and its container exist, the container is a class hall container, and the shipment names a follower | operation not supported (66) |
| The follower could be owned by this character (the followers piece's rules, leaving out its limit) | that rule's error |
| Troops owned of the follower's kind, plus pending orders in the container, are below the capacity: the smaller of the shipment's `MaxShipments` and the kind's `FollowerClassLimit` | at class spec cap (81) |

The capacity is the number the client's window offers: tested in game,
the window caps a troop order at the kind's limit whatever maximum the
server sends. So with three Acolytes none can be ordered. (The draft
allowed one order beyond the limit; the test showed the client never
offers it.)

The request is answered in two steps:

1. The server works out how many orders the checks allow and takes the
   smaller of that and the count asked for. If none is allowed it sends
   `SMSG_CREATE_SHIPMENT_RESPONSE` with the error and stops.
2. It casts the shipment's order spell that many times, as a normal cast.
   The spell code refuses a cast the character cannot pay for, and the
   client shows that as any other spell failure.

The spell does the rest, so the same rules hold however it is cast:

- **Before the cast**, the spell check asks the hall whether the order may
  be made. A refusal fails the cast with "shipments full" (or "no
  shipments for container" for the other errors) before any cost is
  taken. A character without a hall gets "no building for shipment".
- **The effect** (216) adds the order and sends
  `SMSG_CREATE_SHIPMENT_RESPONSE` with its id and result 0.

A new order's creation time is now, or the moment the container's last
pending order finishes if that is later.

Shipments of containers that are not class hall containers are not touched
by either hook: the effect does nothing for them, as today.

### 7. Collecting

Clicking a game object of type 45 (the client reports it with
`CMSG_GAME_OBJ_REPORT_USE`) collects the character's ready orders of
the object's container, oldest first. For each:

1. The follower is added through the followers piece. If that is refused
   (the kind is at its limit), collecting stops here and the order stays
   ready. The followers piece has already told the client why.
2. The order is removed and `SMSG_COMPLETE_SHIPMENT_RESPONSE` is sent.
3. The shipment's on-complete spell, if it has one, is cast on the
   character. This is what credits the quest.

With no hall or nothing ready, nothing happens.

### 8. The hall report

`CMSG_GARRISON_REQUEST_LANDING_PAGE_SHIPMENT_INFO` is answered, for a
character with a class hall, with all of the hall's pending orders. The
artifact knowledge field of the reply is sent as 0; it belongs to the
artifact research shipments, which are out of scope.

## Commit sequence

Each commit builds without new warnings, boots, and is checked in game
before the next one starts.

1. `feat(engine,sql)`: load `CharShipment.db2` and
   `CharShipmentContainer.db2`
2. `feat(engine,sql)`: store class hall shipments
3. `feat(engine)`: add the shipment packets
4. `feat(engine,sql)`: open a class hall shipment from its gossip option
5. `feat(engine)`: order a class hall shipment
6. `feat(engine)`: collect finished class hall shipments
7. `feat(engine)`: report the class hall's shipments

Two long builds: commits 1 and 2 (store and statement headers), then
commits 3 to 7 (the opcode list and the session header). Commits that
share a build are still tested one after the other.

Commit 6 is the least certain: how the click on the object reaches the
server, and what the object needs from the server to be shown and clicked,
are not known (see "Open points"). If the client's behaviour turns out to
differ from this design in a way the test login cannot settle, commit 6
moves to a follow-up and the commits before it stand.

## Error handling

- Saved order whose shipment does not exist in client data, or is not a
  class hall troop shipment: skipped, one error logged, login continues.
  The next save drops the row.
- Saved orders for a character whose hall row is missing or bad: not
  loaded, because the hall is not loaded. The rows stay untouched.
- More saved orders than a shipment allows (possible only through a hand
  edit): all load as saved. The limits are applied to requests.
- A request naming an NPC that is not the one remembered, not a shipment
  crafter, or out of reach: ignored.
- A count of 0, or far above what is allowed: nothing, or as many as are
  allowed.
- An order the character cannot pay for: the cast fails, nothing is
  created.
- A ready order whose troop kind is full: stays ready; no credit.

## Verification

No unit tests, for the same reason as the earlier pieces: every change is
database, packet or player plumbing, and the proof is in the client.

The engine branch is merged into a throwaway
`integration/class-hall-shipments` branch cut from
`feature/priest-class-hall`, and the server is built from that in
`~/arguscore`.

Tools that worked before and are used again: the packet log,
`.debug send opcode` with a hand-written `opcode.txt`, the client's own
functions, and a backup of the `characters` database to restore from.

Test setup, done by hand and never committed:

- Gairy is given a class hall row and Order Resources.
- Vicar Eliza's template gets the gossip and shipment crafter flags and a
  test gossip menu with one option per case: Acolytes (275), the tutorial
  order (333), Zealots (276), a shipment that gives no troop (370), a
  monk's shipment (249), and a shipment id that does not exist.
- She and the "Training Troops" object are spawned next to Gairy with the
  temporary spawn commands.
- Orders are aged by editing their creation time while Gairy is offline,
  so nobody waits 30 minutes.

Checked after every commit: the server boots, DBErrors growth is the
recorded baseline, and a character with no hall logs in and plays as
before.

Commit 2, with rows planted by hand:

1. Log in, `.save`, log out. The planted orders are unchanged.
2. A row with shipment `999999`, and one with a shipment that gives no
   troop: login succeeds, Server.log has one error for each, and the save
   drops them.
3. A throwaway character with a planted row is deleted. Its row is gone.

Then, before the packets are written, one login to settle what is not
known: each reply is sent by hand in its candidate layouts, and the
requests the client sends are read from the packet log.

Commits 4 to 7, in one login and a short second one:

1. The tutorial option opens the window for Acolytes. Ordering one takes
   20 Order Resources. Five seconds later the object gives an Acolyte,
   and the objective "Train Acolytes" of quest 43275 is ticked.
2. The Acolytes option: ordering two takes 40. The second starts when the
   first ends.
3. A request for 1000 orders creates only as many as are allowed and
   takes only their cost.
4. With no room left, casting the order spell directly is refused and
   takes nothing.
5. The options for the troop-less shipment, the monk's shipment and the
   unknown shipment open nothing.
6. A request sent after walking away from the recruiter creates nothing.
7. The hall report lists the pending orders with their timer.
8. After a relog the orders are as they were.
9. With three Acolytes owned and one order ready, the object gives
   nothing and the order stays.

After testing, every planted row is removed, the integration branch is
deleted, and Gairy is put back exactly as she was.

## What the tests showed

Tested in game on 2026-10-14 with a priest and a hand-planted test menu.

- **The window opens only from a gossip window.** The open reply
  (0x27DA) works while a gossip window with the NPC is open; the client
  then asks for the info and closes the gossip. Clicking an NPC that has
  the shipment crafter flag sends a different request (0x32DD) and shows
  no gossip, so recruiters keep the gossip flag only.
- **The info reply keeps the Warlords layout**, with no "owned" count:
  the client counts owned troops from its follower list. It shows the
  kind's limit as the maximum whatever the server sends; hence the
  capacity rule of section 6.
- **The click on the object arrives as `CMSG_GAME_OBJ_REPORT_USE`**, not
  as an ordinary use. The complete reply (0x27DE: order id, then a 32-bit
  result) was accepted: a toast, the troop, and the quest credit.
- **The hall report reply (0x27E0) starts with a 32-bit field** (artifact
  knowledge) before the count; without it the client's list is empty.
- **Ordering, refusing and collecting behaved as designed**: a request
  for 1000 placed only the one allowed; the order spell cast by hand at
  the cap was refused before any cost; collecting three ready orders with
  room for two gave two troops and kept the third ready; out of reach the
  client sent nothing.

Still open:

- **The object is invisible** (the test object used a placeholder model)
  and shows "Awaiting Orders"; whether 0x27DF must be sent to show its
  state was not settled.
- **The client fires `SHIPMENT_UPDATE` every frame** while an order window
  is open and asks for the info about twice a second. It costs little;
  the cause was not found.
- **The client shows no message** for the add-follower refusal (81) when
  collecting at the limit.

Note on timing: the orders of a container run one after another, as
Warlords work orders did. No source found says how Legion troop orders
ran, and the owner does not remember; this is a deliberate choice, not a
bug. If it turns out they all finished together, it is a one-line change
in `ClassHallShipments::CreateShipment` (the creation time becomes the
moment of ordering).

## What comes next

`engine/class-hall-missions`: missions, rewards, follower experience and
gear, vitality loss. Its own spec, plan and branch, stacked on this one.
