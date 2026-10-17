# ArgusCore

<p align="center">
  <img src="assets/logo/ArgusCore.png" alt="ArgusCore" width="400"/>
</p>

**ArgusCore** is a WoW Legion 7.3.5 server emulator, forked from TrinityCore and maintained as a modern, stable, maintainable framework.

> **This is a fork** of [Trion-Control-Panel/ArgusCore](https://github.com/Trion-Control-Panel/ArgusCore).
> It runs a Legion server with its own content and builds engine systems that
> upstream does not have yet, on separate branches so they can be offered back.
> See [What this fork adds](#what-this-fork-adds).

## Build Status

| Platform | Status |
|----------|--------|
| Linux (GCC) | [![Linux Build](../../actions/workflows/build-linux.yml/badge.svg)](../../actions/workflows/build-linux.yml) |
| Windows x64 | [![Windows Build](../../actions/workflows/build-windows.yml/badge.svg)](../../actions/workflows/build-windows.yml) |

## About

ArgusCore is a C++ server emulator for World of Warcraft patch 7.3.5 (Legion).

It is derived from the [TrinityCore](https://github.com/TrinityCore/TrinityCore) project, with a focus on:

- Long-term stability over fast iteration
- Incremental modernization of the codebase
- Maintainability and clean architecture
- Fully playable gameplay at every step

## What this fork adds

The default branch, `dev`, is upstream's `main` plus everything below: the
engine work, which is built on the `engine` branch and merged in piece by
piece, and all content, which is built on `dev` itself. How the branches relate is in
[docs/BRANCHING.md](docs/BRANCHING.md); every engine piece has a design and a
plan in [docs/superpowers](docs/superpowers).

Sources: client DB2 data, Blizzard's UI source, TrinityCore, WowPacketParser,
Wowhead and retail videos. No code or data is taken from leaked cores.

### Legion class halls (engine)

Upstream knows only the Warlords garrison; every class hall request was
unhandled. The fork adds the class hall as its own `ClassHall` class
(`src/server/game/ClassHall/`) with its own tables, one piece at a time, each
tested in game. All engine work is on the `engine` branch; the pieces built
before 2026-10-17 had branches of their own, kept as `archive/` tags.

| Piece | Archived as | What it covers | Status |
|-------|-------------|----------------|--------|
| Foundation | `archive/engine/class-hall-foundation` | The hall and its storage, created by its spell, reported to the client | In `dev` |
| Talents | `archive/engine/class-hall-talents` | Research with its timer, swapping, talent spells, the client's condition checks | In `dev` |
| Followers | `archive/engine/class-hall-followers` | Champions and troops: gaining, active and inactive, dismissing, troop kinds | In `dev` |
| Shipments | `archive/engine/class-hall-shipments` | Troop orders: the order window, ordering, collecting at the container, the hall report | In `dev` |
| Missions | `archive/engine/class-hall-missions` | The mission table, which missions are offered, starting, the success roll, rewards | In `dev` |
| Follower growth | `archive/engine/class-hall-follower-growth` | Follower experience, levels, quality, item level and equipment | In `dev` |
| Mission extras | `archive/engine/class-hall-mission-extras` | Every ability in the chance, cost and duration from threats and abilities, troop vitality, Lethal, heals | In `dev` |
| Mission rewards | `archive/engine/class-hall-mission-rewards` | The bonus roll, single-mission items, remembering and reviving troops | In `dev` |

### Other engine fixes

| Fix | Where |
|-----|-------|
| Quest completed bits were all written to the first block | `engine`, in `dev` |
| Scenario criteria fed by game events could never complete | `engine`, in `dev` |
| Conversations whose actors are not world objects failed to start | `engine`, in `dev` |

### Content

All content is on `dev`; work in progress goes on a short-lived `topic/` branch
only while two sessions work at once (see `docs/BRANCHING.md`).

| Content | What | Status |
|---------|------|--------|
| Broken Shore scenario | The Legion intro scenario, Alliance side, every stage scripted against a retail recording | Playable start to end; polish parked |
| Shadow priest artifact | The opening quests, the "Blade in Twilight" scenario, the Dalaran turn-in, the way into Netherlight Temple | Playable; polish parked |
| Priest class hall | Content on top of the engine pieces | Started: the mission table |
| Emerald Nightmare | The raid: enterable through its Val'sharah portal, with an exit, a meeting stone and an instance script for all seven encounters | Scaffold done; no bosses yet (Nythendra next) |

Local only: a Docker setup for running a server (`Dockerfile`,
`docker-compose.yml`) and `.scenario` GM commands for stepping through
scenarios.

### Roadmap

1. Priest class hall content: recruiters, research, troops (the mission table is in place).
2. Broken Shore polish, after deciding between retail tuning and solo scaling.

## Requirements

### Linux

| Dependency | Minimum Version |
|------------|----------------|
| CMake | 3.28 |
| GCC | 12 |
| OpenSSL | 3.0 |
| Boost | 1.74 |
| MySQL | 8.0.34 |
| MariaDB | 10.6.3 (alternative to MySQL) |

Install on Ubuntu 24.04:

```bash
sudo apt-get install cmake gcc-13 g++-13 ninja-build \
  libboost-all-dev libssl-dev libmysqlclient-dev \
  libreadline-dev zlib1g-dev
```

### Windows

| Dependency | Minimum Version |
|------------|----------------|
| CMake | 3.28 |
| Visual Studio | 2022 (MSVC 14.3+) |
| OpenSSL | 3.0 |
| Boost | 1.83 |
| MySQL | 8.0.34 |

Install via Chocolatey:

```powershell
choco install cmake openssl boost-msvc-14.3
```

MySQL connector: download the Windows ZIP from [dev.mysql.com](https://dev.mysql.com/downloads/mysql/) and set `MYSQL_ROOT_DIR` to the extracted folder.

## Building

**Linux:**

```bash
cmake -S . -B build -G Ninja \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DSCRIPTS=static -DTOOLS=ON -DSERVERS=ON

cmake --build build --parallel $(nproc)
```

**Windows:**

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 `
  -DSCRIPTS=static -DTOOLS=ON -DSERVERS=ON `
  -DMYSQL_ROOT_DIR="C:\mysql"

cmake --build build --config RelWithDebInfo --parallel 4
```

## Configuration

After building, copy the sample config files and edit them for your environment:

```
src/server/worldserver/worldserver.conf.dist  ->  worldserver.conf
src/server/bnetserver/bnetserver.conf.dist    ->  bnetserver.conf
```

Default database connection expects a MySQL/MariaDB user named `arguscore` with password `arguscore`.

## Reporting Issues

Open an issue via the [GitHub issue tracker](../../issues).

Before filing, check for existing issues to avoid duplicates. Include server logs, database version, and steps to reproduce.

## Contributing

Pull requests are welcome. Keep changes small and focused, one fix or feature per PR.

## Copyright

License: GPL 2.0, see [COPYING](COPYING).

ArgusCore is based on TrinityCore. Original TrinityCore copyright belongs to the TrinityCore developers (2008-present).
