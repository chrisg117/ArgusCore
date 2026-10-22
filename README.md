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

## What this fork adds

The fork has two working branches besides `main` (upstream's code, untouched):

- **`engine`**: changes to the core server code and the database schema, and
  nothing else
- **`dev`**: the default branch and what the server runs: upstream's `main`
  plus the engine work, all content (scripts and world data)

Sources: client DB2 data, Blizzard's UI source, TrinityCore, WowPacketParser,
Wowhead and retail videos. No code or data is taken from leaked cores.

### Legion class halls (engine)

| Piece | What it covers |
|-------|----------------|
| Foundation | The hall and its storage, created by its spell |
| Talents | Research, swapping, talent spells |
| Followers | Champions and troops: gaining, activating, dismissing |
| Shipments | Troop orders and collecting them |
| Missions | The mission table: offering, starting, the success roll, rewards |
| Follower growth | Experience, levels, quality, item level, equipment |
| Mission extras | Abilities and threats in the chance, cost and duration; troop vitality |
| Mission rewards | The bonus roll, single-mission items, reviving troops |

### Other engine fixes

All on `engine` and in `dev`:

- Quest completed bits were all written to the first block.
- Scenario criteria fed by game events could never complete.
- Conversations whose actors are not world objects failed to start.
- Area trigger re-paths went out in the wrong packet for 7.3.5.

### Content

All content is on `dev`; work in progress goes on a short-lived `topic/` branch
only while two sessions work at once (see `docs/BRANCHING.md`).

| Content | What | Status |
|---------|------|--------|
| Broken Shore scenario | The Legion intro scenario, Alliance side, every stage scripted against a retail recording | Playable start to end; polish parked |
| Shadow priest artifact | The opening quests, the "Blade in Twilight" scenario, the Dalaran turn-in, the way into Netherlight Temple | Playable; polish parked |
| Priest class hall | Content on top of the engine pieces | Started: the mission table |
| Emerald Nightmare | The raid, boss by boss on Normal (see below) | Nythendra playable; Il'gynoth playable, in testing; Elerethe Renferal in progress |

### Emerald Nightmare

| Boss | Status | What is in |
|------|--------|-----------|
| Nythendra | **Playable, tested in game** | Both phases; timers fitted to DBM; enrage; voiced yells; root walls that seal the room |
| Il'gynoth | **Playable, in testing** | The Eye and its Ichor explosions; tentacle and Horror waves fitted to DBM; heart phases with Dark Reconstitution and Final Torpor; a forcefield over the chamber. Group-only mechanics still to test |
| Elerethe Renferal | **In progress, being tuned** | Spider and Roc form mechanics implemented on an energy bar fitted to DBM; flights across three platforms with Dark Storm, the Eye of the Storm and Shimmering Feathers; spiderlings and tornadoes; enrage. Egg sacs and the Feather crush still to build |
| Ursoc, Dragons of Nightmare, Cenarius, Xavius | Not started | |

### Roadmap

1. Emerald Nightmare: finish Il'gynoth's in-game testing, finish and tune Elerethe Renferal, then the remaining bosses in journal order.
2. Priest class hall content: recruiters, research, troops (the mission table is in place).
3. Broken Shore polish, after deciding between retail tuning and solo scaling.

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
