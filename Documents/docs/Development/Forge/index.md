# Forge

**Forge** is the Visera engine toolchain: a standalone executable (built when `VISERA_BUILD_FORGE` is ON) that lives outside the main runtime. It is not linked into `Visera.dll` and outputs under `Toolkit/Forge` with per-config subdirs (Debug / Develop / Release).

## Overview

- **Build**: Toggled by the CMake option `VISERA_BUILD_FORGE` (default: ON). When OFF, the Forge target is not built.
- **Output**: `Toolkit/Forge/<Config>/` (e.g. `Toolkit/Forge/Debug/Visera-Forge.exe` on Windows).
- **Dependencies**: Forge links to the main `Visera` library and uses its own externals (e.g. Abseil, RE2) via `Engine/Forge/Scripts/install_*.cmake`.

## Modules and utilities

| Topic | Description |
|-------|--------------|
| [Regex](Regex.md) | RE2-based regex API using `FStringView` / `FString`: FullMatch, PartialMatch, Replace, Extract, QuoteMeta, `FRegexPattern`. |

## Structure

- **Engine/Forge/** – CMake, sources, and scripts.
- **Engine/Forge/Source/** – C++ module sources (e.g. `Visera-Forge.ixx`, `Utils/Regex/...`).
- **Engine/Forge/Scripts/** – `install.cmake`, `install_abseil.cmake`, `install_re2.cmake`.
- **Engine/Forge/External/** – Abseil, RE2, etc.

For detailed Regex usage and examples, see [Regex](Regex.md).
