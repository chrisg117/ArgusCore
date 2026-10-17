# Branching rules for this fork

Agreed 2026-10-13. These apply to every session working on this repository.

The fork has two aims that pull in different directions: run a Legion server
with our own content, and keep engine work clean enough to offer upstream.
The rules below keep the two apart.

## What each branch is for

| Branch | Cut from | Holds | Rule |
|--------|----------|-------|------|
| `main` | upstream | Upstream's code only | Never commit to it; fast-forward only |
| `engine/<system>-<piece>` | `main`, or the engine branch before it | Core code and schema SQL | Never content, Docker, GM helpers or docs |
| `dev` | `main` | Docker, docs, landed engine work, finished content | Only receives merges and local-only commits |
| `feature/<name>` | `dev`, or the feature it depends on | Content: scripts and world SQL | One feature per branch |
| `integration/<name>` | a feature branch | Nothing of its own | Throwaway for testing an engine branch; delete after |

`engine/*` is what could go upstream. `feature/*` is work in progress. `dev`
is the finished state of the server.

## Direction of flow

1. Changes move one way: `main` → `engine/*` → `dev` → `feature/*`. Nothing
   is merged backwards.
2. A stacked feature branch pulls from the one below it, never the other
   way. Example: `feature/priest-artifact-weapon` merges into
   `feature/priest-class-hall`, not the reverse.
3. When an engine branch lands in `dev`, merge `dev` into every active
   feature branch in the same sitting. This keeps their engine files
   identical, so switching the build folder between them does not trigger a
   full rebuild.
4. A feature branch merges into `dev` when the feature is called finished.

## Commits

5. One kind per commit: `feat(engine)` or `fix(engine)`, `fix(DB/World)`,
   `feat(scripts,sql)`, `chore(local)`. Engine and content never share a
   commit.
6. One SQL file per logical change, named `YYYY_MM_DD_NN_<db>.sql` under
   `sql/updates/<db>/master/`, safe to re-run where the SQL allows it. The
   date is the server's date (UTC), the clock git stamps commits with.
7. Once a branch is pushed or merged, never rebase or amend it. If `main`
   has moved when engine work is to be offered upstream, cherry-pick onto a
   fresh `upstream/<topic>` branch cut from `main`.

## The build folder and two sessions

8. Every image build comes from `~/arguscore`. Never build another folder or
   a git worktree, and never use `--no-cache`. The build cache goes by file
   time, so a second folder leaves stale objects behind.
9. One session owns `~/arguscore` at a time. Before touching it, check which
   branch it is on, that the tree is clean, and that no build is running
   (`pgrep -x docker-buildx`).
10. Merges that do not need a build are done in a temporary worktree
    (`git worktree add /tmp/<name> <branch>`), so file times in
    `~/arguscore` stay as they are.
11. Tell the other session when a branch is renamed or the build folder
    changes branch.
12. Before a server restart: every character offline, and the game client
    closed. A client left at character selection hangs on the loading screen
    afterwards.

## Pushing and upstream

13. Push a branch once its work is landed or a stage is committed. Never
    force-push.
14. Upstream pull requests and issues are opened by the repository owner
    only, never by an assistant session. Anything submitted must be work the
    owner has read and can explain.

## Testing an engine branch

An engine branch cannot run on its own, because the Docker files are only on
`dev`. To test one:

1. Create `integration/<name>` from the feature branch the server runs.
2. After each engine commit, merge the engine branch into it, build, boot,
   and check in game.
3. When every commit has passed, merge the engine branch into `dev`
   (`--no-ff`), merge `dev` into the feature branches, and delete the
   integration branch.

## Current branches (2026-10-14)

- `engine/class-hall-foundation`: class hall engine, first piece. Landed in
  `dev`.
- `engine/class-hall-talents`: class hall engine, second piece (research,
  finish, swap, talent spell, condition checks), stacked on the foundation.
  Landed in `dev`.
- `engine/class-hall-followers`: class hall engine, third piece (owning
  champions and troops: gain, active and inactive, dismiss, troop kinds),
  stacked on the talents. Landed in `dev`.
- `engine/class-hall-shipments`: class hall engine, fourth piece (troop
  orders: the order window from a gossip option, ordering by the order
  spell, collecting at the container object, the hall report), stacked on
  the followers. Landed in `dev`.
- `engine/class-hall-missions`: class hall engine, fifth piece (the
  mission table: offering, opening, starting with the client's success
  chance, completing, rewards), stacked on the shipments. Landed in
  `dev`.
- `engine/class-hall-follower-growth`: class hall engine, sixth piece
  (follower experience, levels, quality, item level and equipment),
  stacked on the missions. Landed in `dev`.
- `engine/class-hall-mission-extras`: class hall engine, seventh piece
  (the whole team's chance, cost and duration, troop vitality, Lethal,
  heals), stacked on the follower growth. Landed in `dev`.
- `engine/class-hall-mission-rewards`: class hall engine, eighth piece
  (the bonus roll, single-mission abilities, remembering and reviving
  expired troops), stacked on the mission extras. Landed in `dev`.
- `engine/quest-completed-bit`: one fix, cut from `main`: quest completed
  bits were written to the wrong block. The talents need it. Landed in
  `dev`.
- `feature/broken-shore-scenario`: Broken Shore intro scenario.
- `feature/priest-artifact-weapon`: shadow priest artifact content, stacked
  on the Broken Shore branch.
- `feature/priest-class-hall`: priest class hall content, stacked on the
  artifact branch.
