# Forge (`Visera.Forge`, `Visera-Forge`)

**Forge** is the optional **engine toolchain**: shader compilation, file discovery helpers, and related utilities. It is **not** the game runtime; games link the **`Visera`** library and use `Visera::CreateEngine` from the umbrella module.

## CMake

- Option **`VISERA_BUILD_FORGE`** (default on in root `CMakeLists.txt`) controls whether the Forge subdirectory is added.
- Target **`Visera-Forge`** is an **executable** that links **`Visera`** (see `Engine/Forge/CMake/install.cmake`). When Visera is built as a shared library, the Forge output may copy the Visera DLL next to the executable for loading.

## Modules

- **`Visera.Forge`** — Aggregates Forge submodules (e.g. `Visera.Forge.Shader`, `Visera.Forge.Utils`).
- Shader-related code lives under `Engine/Forge/Source/Shader/` (compiler, validator).
- Utilities under `Engine/Forge/Source/Utils/` (pattern matching, escaping, etc.).

Forge sources import **Core**, **Platform**, **Runtime**, and the top-level **Visera** module as needed for shared types and services used in batch tooling.

## Shaders and includes

Forge uses the engine shader tree via **`VISERA_ENGINE_SHADERS_DIR`**, pointing at `Engine/Shaders`. See [Repository layout](../Repository-Layout.md).

## See also

- [Architecture](../Architecture.md) — how Forge fits next to Runtime and Core
- [Runtime.RHI](../Runtime/RHI/index.md) — runtime graphics API used by games (distinct from Forge batch work)
