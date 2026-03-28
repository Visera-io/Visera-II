# Repository layout (`Engine/`)

Top-level directories next to `Visera.ixx` and `Engine/CMakeLists.txt`:

| Directory | Contents |
|-----------|----------|
| **`Core/`** | C++ modules for `Visera.Core`. Source under `Core/Source/`; third parties under `Core/External/`. |
| **`Platform/`** | C++ modules for `Visera.Platform`. `Source/Interface/`, `Windows/`, `MacOS/`, `GLFW/`, `Null/`. |
| **`Runtime/`** | C++ modules for `Visera.Runtime`. Major areas under `Runtime/Source/`: `RHI/`, `Graphics/`, `AssetHub/`, `Audio/`, `Input/`, `Window/`, `UI/`, `Scripting/`, `World/` (stub). |
| **`Forge/`** | Toolchain sources for `Visera.Forge` and the `Visera-Forge` executable. Shader compiler and utilities; CMake option `VISERA_BUILD_FORGE`. |
| **`APIs/`** | `Visera.d.ts` and `tsconfig.json` for TypeScript consumers and editor tooling aligned with script APIs. |
| **`Schemas/`** | JSON Schema definitions for engine data. Current files: `Material.schema.json`, `InputMap.schema.json`. |
| **`Shaders/`** | Slang shader library and includes consumed by the runtime shader path and Forge (`VISERA_ENGINE_SHADERS_DIR` in CMake). |

The **Visera** library target is defined at `External/Visera/CMakeLists.txt` and pulls in `Engine/Visera.ixx` plus the Engine subtree via `add_subdirectory(Engine)`.

## See also

- [Architecture](Architecture.md) — module graph and engine lifecycle
