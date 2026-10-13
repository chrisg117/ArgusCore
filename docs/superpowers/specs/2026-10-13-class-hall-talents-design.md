# Class hall talents: research, finish, swap and effects

Date: 2026-10-13
Branch: `engine/class-hall-talents`, stacked on `engine/class-hall-foundation`
Status: awaiting review
Builds on: `2026-10-12-class-hall-foundation-design.md`

## Goal

A character with a class hall can research a hall upgrade (a "talent"), see
it finish whether online or offline, and swap a choice in a row. A finished
talent casts its player spell, and game data can ask whether a talent is
selected, researched or in progress.

This is the second of five engine pieces for class halls. Followers,
shipments and missions come after it.

## Decisions

Made with the owner on 2026-10-13:

- **Effects in scope:** the talent's player spell and the three talent
  condition checks. Mission and follower abilities are left to those pieces.
- **Retail research times, no server setting.** The client shows the
  countdown from its own copy of the talent data, and the server only sends
  the start time. Faster research, if ever wanted, is a data change (a
  hotfix row the client also receives), not engine code.
- **A separate `ClassHallTalents` class in its own files, owned by the
  hall.** Sub-systems the player owns (`RestMgr`, `ReputationMgr`,
  `CinematicMgr`) are separate classes; this follows that one level down
  and keeps `ClassHall` small as later pieces arrive.
- **Finished research is noticed by a check on the player's regular
  update.** The player update already runs countdown checks of this kind and
  schedules no one-off events. The comparison is the one the Warlords
  garrison uses for buildings: start time plus the duration from client
  data, against the clock.

## What the client data says

`GarrTalentTree.db2` has 14 trees, all of garrison type 3 (class hall).
Twelve belong to a class; the priest tree is 134. Two have no class (trees
150 and 151, used by a scenario).

`GarrTalent.db2` has 184 talents. For each talent, in the order the fields
appear:

| Field | Example (446, Divine Fate) | Meaning |
|-------|----------------------------|---------|
| Name, Description | "Divine Fate" | shown in the window |
| IconFileDataID | 135976 | |
| ResearchDurationSecs | 7200 | 2 hours to 5 days in the class trees |
| Tier | 0 | row, 0 to 7 |
| UiOrder | 0 | column, 0 or 1 |
| Flags | 0 | 0 in every row |
| ID | 446 | |
| GarrTalentTreeID | 134 | |
| GarrAbilityID | 0 | a mission or follower ability, on some talents |
| PlayerConditionID | 41962 | what unlocks the talent |
| ResearchCost | 50 | |
| ResearchCostCurrencyTypesID | 1220 | Order Resources |
| ResearchGoldCost | 0 | 0 in every row |
| PerkSpellID | 219866 | a spell for the player, on some talents |
| PerkPlayerConditionID | 41975 | condition for that spell |
| RespecCost | 500 | cost of swapping to this talent |
| RespecCostCurrencyTypesID | 1220 | |
| RespecDurationSecs | 86400 | 1 day |
| RespecGoldCost | 0 | |

Field names follow the meaning the values show; the layout is the one
`GarrTalentMeta` in `DB2Metadata.h` already describes (20 fields, id at
index 7, tree as the parent at index 8).

Other facts:

- A class tree has two talents in rows 0 to 4 and 6, and one in rows 5
  and 7.
- The unlock conditions are level gates (100 for row 0, 105 for row 1, 110
  for rows 2 to 5) and one achievement (row 6). Row 7 has none.
- Thirteen distinct player spells are used by the class trees. Twelve are
  passive auras with no duration. One (221587, paladin) is a one-off cast.
- `ModifierTree.db2` has 292 rows of type 202 (talent researched), 23 of
  type 207 (research in progress) and 2 of type 201 (talent selected).
- The client's research window (`Blizzard_OrderHallTalents.lua`) sends one
  request, `C_Garrison.ResearchTalent(id)`, for both a first choice and a
  swap, and redraws on the events `GARRISON_TALENT_UPDATE` and
  `GARRISON_TALENT_COMPLETE`. It works out which talents are available by
  itself, so the server's rules must agree with it.

## What the core already has

- `GarrTalentTree.db2` is loaded.
- The research window opens: a gossip option of type "garrison talent" sends
  `SMSG_GARRISON_OPEN_TALENT_NPC`.
- The garrison info reply has a talent list (id, research start time,
  flags). The class hall sends it empty.
- `CMSG_GARRISON_RESEARCH_TALENT` (0x32D1) is defined and unhandled.
- The garrison error list has the talent errors: invalid talent (84),
  already researching (85), failed condition (86), invalid tier (87),
  invalid class (88), not enough currency (46), not enough gold (47).
- The three talent condition checks are marked "not implemented" and answer
  false.

Missing: the talent store, storage, the request handler, the timer, the
replies, and the effects.

## Packets

The request carries only the talent id.

The three replies have no known numbers for this client build in
WowPacketParser. Its 9.0.1 table names them, in this order, directly after
the garrison "upgrade result" reply. The 7.3.5 table has exactly three
unnamed numbers at that place:

| Reply | Number (inferred) | Contents (from the 8.0.1 parser) |
|-------|-------------------|----------------------------------|
| Research talent result | 0x28FE | result code, garrison type, a "do not alert" bit, the talent (id, start time, flags) |
| Talent completed | 0x28FF | garrison type, talent id, flags |
| Talent removed | 0x2900 | garrison type, talent id (no parser exists; smallest plausible layout) |

WowPacketParser names five talent flag values: researched 0x1, respec 0x2,
researching 0x4, ongoing 0x8, do not reset 0x10. They come from a later
client.

Both the numbers and the layouts are inferences. Each is confirmed in game
by the commit that first uses it. See "Open points".

## Scope

In scope:

- Loading `GarrTalent.db2`.
- `ClassHallTalents`, its table, and its place in the hall's load, save,
  delete, update and info reply.
- Handling the research request, with the rules below.
- Finishing research on time, online or offline.
- Casting a finished talent's player spell, and re-applying passive ones at
  login.
- The condition checks 201, 202 and 207.
- Swapping a talent.

Out of scope:

- Mission and follower abilities from talents (`GarrAbilityID`).
- What a player spell does once cast. Some use aura types this core may not
  implement (219866 uses aura 474). That is spell-system work.
- Trees with no class (150 and 151). Their talents are instant, which the
  class trees cannot exercise. They are refused with "invalid class".
- Achievement counters for started and finished research (10 criteria in
  the client data).
- Later-expansion condition checks (290, 317, 318).
- Spawning the research NPCs and their gossip (content, on
  `feature/priest-class-hall`).
- Checking that the character stands at a research NPC. This client's
  request does not name one.
- GM commands or other test helpers.

## Rules this work follows

- Engine commits contain core code and schema SQL only.
- New code goes in new files. Shared files get small hook changes only.
- No warnings, no debug code, no TODO comments.
- Small commits, each booted and tested in game before the next.
- SQL files are named by the server's date, which is UTC.
- This document lives on `dev`, never on the engine branch.

## Design

### 1. Talent store

Load `GarrTalent.db2` like any other store: entry struct, load info, hotfix
statement and `LOAD_DB2` line, plus the `garr_talent` and
`garr_talent_locale` hotfix tables (the talent has two text fields).

### 2. `ClassHallTalents`

New files: `src/server/game/ClassHall/ClassHallTalents.h` and `.cpp`.

It belongs to one `ClassHall` and holds the character's talents. For each:
the talent's data row, the research start time, and whether it is finished.
It also remembers when the running research ends, or that none is running.

It can:

- **Load** from its rows. A row whose talent does not exist in client data
  is skipped with one logged error.
- **Save** its rows inside the player's save transaction.
- **Delete** all rows of a character.
- **Fill** the talent list of the hall's info entry.
- **Research** a talent for the owner, applying the rules in section 4.
- **Update**: finish the running research when its time has come.
- **Answer** the three condition checks for a talent id.

`ClassHall` owns one and forwards load, save, delete, update and the info
entry to it. `ClassHall` gains an `Update` for this.

### 3. Storage

One new table. No existing table changes.

`character_class_hall_talent`:

| Column | Type | Meaning |
|--------|------|---------|
| `guid` | bigint unsigned | the character |
| `talentId` | int unsigned | `GarrTalent` id |
| `researchStartTime` | bigint | unix time the research began |
| `flags` | int unsigned | the talent's state, as sent to the client |

Primary key `(guid, talentId)`.

Statements: select by character, insert, and delete by character. Saving
deletes the character's rows and inserts the current ones, inside the
player's save transaction, which is how the core saves other small sets.

The table has no database link to `character_class_hall`. The hall row is
saved by delete and insert, so a cascading link would wipe the talents on
every save.

The table is added to the character dump list. Character deletion removes
its rows.

The cost is taken from the character in memory when research starts, and
the talent row is written at the next save. Currency is saved the same way,
so the two cannot drift apart.

### 4. Research rules

Checked in this order. The first failure changes nothing and sends the
"research talent result" reply with the error.

| Check | Error |
|-------|-------|
| The character has a class hall | no garrison (1) |
| The talent exists and its tree is of the class hall type | invalid talent (84) |
| The tree's class is the character's class | invalid class (88) |
| The character does not already have this talent | invalid talent (84) |
| No research is running | already researching (85) |
| The talent's unlock condition is met, if it has one | failed condition (86) |
| The row before it has a finished talent (row 0 needs none) | invalid tier (87) |
| The character has the currency cost | not enough currency (46) |
| The character has the gold cost | not enough gold (47) |

If the same row already holds a finished talent, the request is a swap and
section 7 applies: the swap cost and swap time are used in place of the
research ones.

On success the cost is taken, the talent is added with the current time as
its start, the end time is remembered, and the reply is sent with result 0
and the talent.

### 5. Finishing

`Player::Update` calls `ClassHall::Update` when the character has a hall.
That reaches `ClassHallTalents::Update`, which returns at once unless
research is running and its end time has passed.

On finish:

1. The talent is marked researched.
2. The "talent completed" reply is sent.
3. The talent's effect is applied (section 6).

Research that ended while the character was offline finishes on the first
update after login, by the same path. Nothing special happens at load.

### 6. Effects

**Player spell.** When a talent finishes and it has a player spell, the
spell is cast on the character, provided the talent's spell condition is
unset or met.

Passive spells are also cast at login for every finished talent, because
the core does not save passive auras. The condition is checked again then.
A spell that is not passive is cast only once, at finish.

When a talent is removed (section 7), the aura of its player spell is
removed.

The spell condition is checked at finish and at login only. A character
whose condition changes in between gets the spell at the next login.

**Condition checks.** In `CriteriaHandler`, the three checks ask the
character's hall:

| Check | True when |
|-------|-----------|
| 201 talent selected | the character has the talent, running or finished |
| 202 talent researched | the character has the talent and it is finished |
| 207 research in progress | the character has the talent and it is still running |

A character with no hall gets false, as today.

### 7. Swap

A swap is a research request for a talent whose row already holds a
different, finished talent.

- All rules of section 4 apply, with the talent's swap cost, swap currency
  and swap gold cost in place of the research ones.
- The old talent is removed: its row is dropped, its player spell aura is
  removed, and the "talent removed" reply is sent.
- The new talent starts researching and finishes after its swap time.

Rows with a single talent cannot be swapped.

Swap is the last commit. If the client's expectations turn out to differ
from this, swap moves to a follow-up and the seven commits before it stand.

## Commit sequence

Each commit builds without new warnings, boots, and is checked in game
before the next one starts.

1. `feat(engine,sql)`: load `GarrTalent.db2`
2. `feat(engine,sql)`: store class hall talents and report them to the
   client
3. `feat(engine)`: add the garrison talent packets
4. `feat(engine)`: research a class hall talent
5. `feat(engine)`: finish class hall research on time
6. `feat(engine)`: cast a finished class hall talent's spell
7. `feat(engine)`: answer the garrison talent condition checks
8. `feat(engine)`: swap a class hall talent

Commits 1, 2 and 4 change widely included headers (DB2 structures, database
statements, the session), so each is a build of about 40 minutes. Commit 3
defines all three replies at once and has no behaviour of its own; it is
built together with commit 4, because the opcode list is also included
almost everywhere and adding the replies one at a time would cost two more
full builds. Commits 5 to 8 change only the class hall files and a few
source files (`Player.cpp`, `CriteriaHandler.cpp`), so they are short
builds.

## Error handling

- Saved talent that does not exist in client data: skipped, one error
  logged, login continues.
- Saved talents for a character whose hall row is missing or bad: not
  loaded, because the hall itself is not loaded.
- Two saved talents both still running (possible only through a hand
  edit): both load; they finish in start order.
- Request for a talent the character cannot research: refused with the
  error from the table in section 4. Nothing is taken.
- Player spell that does not exist: nothing is cast, one error logged, the
  talent still finishes.

## Verification

There are no unit tests in this piece, for the same reason as the
foundation: every change is database, packet or player plumbing, and the
proof is in the client.

The engine branch is merged into a throwaway `integration/class-hall-talents`
branch cut from `feature/priest-class-hall`, and the server is built from
that in `~/arguscore`.

Test setup, done by hand and never committed:

- Gairy (priest, level 100) is given a class hall with
  `.cast self 185506 triggered`, and Order Resources with the currency GM
  command.
- The research window is opened with `/run ToggleOrderHallTalentUI()`; the
  client falls back to the character's class tree when no NPC named one.
  Only if that fails is a gossip option of the talent type planted on a
  spawned NPC.
- Row 0 also needs one completed class hall quest (see "Open points"), so
  Gairy is given quest 42210 as rewarded.
- "Moving the start time back" means editing `researchStartTime` in the
  table while Gairy is offline.

Checked after every commit:

- The server boots and DBErrors growth is 13,422, the recorded baseline.
- A character with no hall, and a character with a hall and no talents,
  log in and play as before.

Commit 1:

1. Boot only. The store loads without a DBErrors line.

Commit 2, with rows planted by hand for talents of tree 134:

1. Log in, `.save`, log out. The rows are unchanged.
2. `/dump C_Garrison.GetTalent(446)` for a row with each candidate flag
   value. Record what the client reports as selected, researched and being
   researched. This fixes the flag values.
3. A row with talent id `999999`: login succeeds, Server.log has one error
   naming it.
4. A character dump contains the talent rows.
5. A throwaway character with a hall and a talent row is deleted. Its rows
   are gone.

Commit 4, starting with no talents and 100 Order Resources:

1. Open the window and research Divine Fate (446). Gairy has 50 resources
   left, the window shows the countdown without a relog, and after `.save`
   the table has the row.
2. While it runs, request Daily Prayers (447) with
   `/run C_Garrison.ResearchTalent(447)`. Nothing changes.
3. With the first research cleared and 10 resources: research is refused
   and nothing is taken.
4. Request a row 1 talent (452) with no row 0 talent finished. Refused.
5. Request a talent of another class's tree (402). Refused.

Commit 5:

1. Research 446, log out, move the start time back by more than 2 hours,
   log in. The window shows it finished and the row's flags say so after
   `.save`.
2. Research the next one, move the start time back to leave about a minute,
   log in and wait with the window open. It finishes without a relog.

Commit 6:

1. After 446 finishes, Gairy has the aura of spell 219866 (`.list auras`).
2. After a relog the aura is there again.

Commit 7:

1. A condition that checks talent 446 is false before research and true
   after it finishes. The in-progress check is true only while it runs.
   The checks are observed through a planted gossip option that is shown
   only when a player condition built on that check passes; planning picks
   the exact rows from `ModifierTree.db2` and `PlayerCondition.db2`.

Commit 8, with 446 finished and enough resources:

1. Request 447. Gairy pays 500, 446 and the aura of 219866 are gone, 447
   shows as researching with a one-day countdown, and the window follows
   without a relog.
2. After `.save` the table has 447 and not 446.

After testing, Gairy's talent rows, the planted gossip row and the hall row
are removed by hand, and the integration branch is deleted.

## Open points

Each is settled by the in-game check named, and the spec is corrected in
the same sitting if the answer differs.

- **The reply numbers and layouts** are inferred. Commit 4 confirms the
  result reply, commit 5 the completed reply, commit 8 the removed reply.
  If one is wrong and cannot be found, the fallback is to resend the full
  garrison info reply, which is a known packet; nothing else in the design
  depends on the three replies.
- **The flag value for each state.** Commit 2, step 2 answers it before any
  code depends on it.
- **Whether the row-before rule needs that talent finished or only
  chosen.** Settled in planning without a test: the two cannot differ.
  While a talent is still running, every other request is refused as
  "already researching", so by the time a request can pass, the talent in
  the row before is finished. The server asks for a finished one.
- **Whether the old talent stays active while a swap is running.** The
  design removes it at once. Commit 8 shows whether the client agrees.
- **What the client shows for a swap's cost and time.** Commit 8.
- **Condition 41975** (the spell condition of the twelve row 0 talents)
  is not in `PlayerCondition.db2`, neither as a record nor as a copied
  row. The core treats a condition id with no row as met, so the spell is
  cast. Planning found this; the cast commit confirms it.
- **Row 0 has a quest gate.** Its unlock condition (41962) needs level 100
  and one completed quest out of twelve, one per class. The engine does
  nothing special for it; the existing condition code evaluates it.

## What comes next

Each is its own spec, plan and branch, stacked on this one:

1. `engine/class-hall-followers`: champions and troops
2. `engine/class-hall-shipments`: troop work orders and recruits
3. `engine/class-hall-missions`: missions and rewards, including the
   mission abilities that talents grant
