# Branching rules for this fork

Agreed 2026-10-13; reshaped 2026-10-17, when one branch per feature had
grown too many branches and too many merges. These apply to every session
working on this repository.

The fork has two aims that pull in different directions: run a Legion server
with our own content, and keep engine work clean enough to offer upstream.
The one separation the branches keep is engine code apart from content.
Content features do not need to ship on their own.

## What each branch is for

| Branch | Cut from | Holds | Lives |
|--------|----------|-------|-------|
| `main` | upstream | Upstream's code only | Forever; never commit to it, fast-forward only |
| `engine` | `main` | All core code and schema SQL, one line | Forever; never content, Docker, GM helpers or docs |
| `dev` | `main` | What the server runs: Docker, docs, landed engine work, all content | Forever; the build folder is on it |
| `topic/<name>` | `dev` | A chunk of work while two sessions work at once, or a risky change | Days: merged back and deleted |
| `try` | `dev` | Nothing of its own: `dev` plus `engine`, to test engine commits | Local only, never pushed; reset for each piece |

`engine` is what could go upstream. `dev` is the server.

## Direction of flow

1. Changes move one way: `main` → `engine` → `dev`, and `topic/*` → `dev`.
   Nothing is merged backwards, and `dev` is never merged into `engine`.
2. A topic branch is never stacked on another and is not kept in step with
   `dev`. Merge `dev` into it only when its work needs something new there.
   When it is tested, merge it into `dev` with `--no-ff` and delete it.
3. Content may be committed on `dev` directly when only one session is
   working.

## Engine pieces

4. An engine piece is a series of commits on `engine`, with its own spec
   and plan. It lands with one `--no-ff` merge into `dev` whose message
   names the piece, and a tag `piece/<name>` on the last engine commit.
5. A core fix found while writing content is committed on `engine`, never
   on `dev` or a topic, and then merged into `dev`.

## Commits

6. One kind per commit: `feat(engine)` or `fix(engine)`, `fix(DB/World)`,
   `feat(scripts,sql)`, `feat(content)`, `chore(local)`. Engine and content
   never share a commit.
7. One SQL file per logical change, named `YYYY_MM_DD_NN_<db>.sql` under
   `sql/updates/<db>/master/`, safe to re-run where the SQL allows it. The
   date is the server's date (UTC), the clock git stamps commits with.
8. Once a branch is pushed or merged, never rebase or amend it. To offer
   engine work upstream, cherry-pick it onto a fresh `upstream/<topic>`
   branch cut from `main`.

## The build folder and two sessions

9. Every image build comes from `~/arguscore`, which stays on `dev` (or on
   `try` while an engine piece is tested). Never build another folder or a
   git worktree, and never use `--no-cache`. The build cache goes by file
   time, so a second folder leaves stale objects behind.
10. One session owns `~/arguscore` at a time. Before touching it, check which
    branch it is on, that the tree is clean, and that no build is running
    (`pgrep -x docker-buildx`).
11. Merges that do not need a build are done in a temporary worktree
    (`git worktree add /tmp/<name> <branch>`), so file times in
    `~/arguscore` stay as they are. Engine work is written in the
    `~/arguscore-engine` worktree, on `engine`.
12. Tell the other session when a branch is created, merged or deleted, or
    the build folder changes branch.
13. Before a server restart: every character offline, and the game client
    closed. A client left at character selection hangs on the loading screen
    afterwards.

## Pushing and upstream

14. Push `engine` and `dev` once work is landed or a stage is committed.
    Push a topic branch only if another session needs it. Never
    force-push.
15. Upstream pull requests and issues are opened by the repository owner
    only, never by an assistant session. Anything submitted must be work the
    owner has read and can explain.

## Testing an engine piece

An engine commit cannot run on its own, because the Docker files are only on
`dev`. To test one:

1. Reset `try` to `dev` in `~/arguscore`.
2. After each engine commit, merge `engine` into `try`, build, boot, and
   check in game.
3. When every commit has passed, merge `engine` into `dev` (`--no-ff`), tag
   the piece, put `~/arguscore` back on `dev`, and delete `try`.

## Retired branches

The branches of the first model were retired on 2026-10-17. Each one is kept
as a tag `archive/<branch>` on its last commit, so nothing was lost:

- the engine pieces `engine/class-hall-foundation`, `-talents`,
  `-followers`, `-shipments`, `-missions`, `-follower-growth`,
  `-mission-extras` and `-mission-rewards`, and the fix
  `engine/quest-completed-bit`, all on `engine` now;
- the content branches `feature/broken-shore-scenario`,
  `feature/priest-artifact-weapon` and `feature/priest-class-hall`, all
  merged into `dev`.

Three core fixes that had been made on the content branches (the scenario
game-event criteria and two conversation actor fixes) were cherry-picked
onto `engine`, so it holds every core change `dev` has.

## Current branches (2026-10-17)

- `main`, `engine`, `dev`.
- `feature/emerald-nightmare`: the Emerald Nightmare, cut before the
  change by another session; it becomes a topic branch when that session
  next works on it.
