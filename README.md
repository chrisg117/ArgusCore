# ArgusCore

<p align="center">
  <img src="assets/logo/ArgusCore.png" alt="ArgusCore" width="400"/>
</p>

**ArgusCore** is a WoW Legion 7.3.5 server emulator, forked from TrinityCore and maintained as a modern, stable, maintainable framework.

## Build Status

| Platform | Status |
|----------|--------|
| Linux (GCC) | [![Linux Build](../../actions/workflows/build-linux.yml/badge.svg)](../../actions/workflows/build-linux.yml) |
| Windows x64 | [![Windows Build](../../actions/workflows/build-windows.yml/badge.svg)](../../actions/workflows/build-windows.yml) |

## Development Status

What is playable on the `dev` branch. Counts only; details live in the specs.

**Class halls and artifact weapons**

| Class | Class hall | Artifact weapon |
|-------|------------|-----------------|
| Death Knight | Acherus open, campaign through The King Rises | Frost (Blades of the Fallen Prince) |
| Demon Hunter | Fel Hammer open, campaign slice 1 | Havoc (Twinblades of the Deceiver) |
| Priest | Netherlight Temple entry and mission table | Shadow (Xal'atath) |
| All other classes | None | None |

**Legion zone quests**

| Zone | Quests playable |
|------|-----------------|
| Stormwind and the Broken Shore (Alliance opening) | 5 |
| Demon Hunter starting zone (Mardum and the Vault of the Wardens) | 31 |
| Azsuna | 29 |
| Dalaran | 0 (city services only) |
| Val'sharah, Highmountain, Stormheim, Suramar | 0 |

**Raid bosses**

| Raid | Bosses playable |
|------|-----------------|
| Emerald Nightmare | 2 of 7 |
| Trial of Valor | 0 of 3 |
| The Nighthold | 0 of 10 |
| Tomb of Sargeras | 0 of 9 |
| Antorus, the Burning Throne | 0 of 11 |

## About

ArgusCore is a C++ server emulator for World of Warcraft patch 7.3.5 (Legion).

It is derived from the [TrinityCore](https://github.com/TrinityCore/TrinityCore) project, with a focus on:

- Long-term stability over fast iteration
- Incremental modernization of the codebase
- Maintainability and clean architecture
- Fully playable gameplay at every step

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
