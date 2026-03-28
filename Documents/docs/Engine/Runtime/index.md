# Runtime (`Visera.Runtime`)

**Runtime** is the application-facing layer: RHI (Vulkan), graphics (scene, material, render graph), AssetHub, audio, input, scripting (V8), UI (ImGui), and window integration. It depends on [Core](../Core/index.md) and [Platform](../Platform/index.md) and is what games link through the top-level **`Visera`** module (`Engine/Visera.ixx`).

## Overview

- **Module**: `Visera.Runtime`
- **Source path**: `Engine/Runtime/Source/`
- **Dependencies**: [Core](../Core/index.md) and [Platform](../Platform/index.md). Optional [Forge](../Forge/index.md) is a separate executable, not a Runtime dependency for games.
- **Build**: Built as part of the `Visera` library target.

## Submodules

The root module `Visera-Runtime.ixx` re-exports:

| Module | Description |
|--------|-------------|
| [AssetHub](AssetHub/index.md) | Asset loading: images (PNG/EXR), font (FreeType), shader. |
| [Audio](Audio/index.md) | Audio interface, Null backend, Wwise and Wwise-IO. |
| [Graphics](Graphics/index.md) | Framework, material, pipeline cache, render graph, scene (camera, light, renderable). |
| [Input](Input/index.md) | Actions, mapping, devices (keyboard, mouse). |
| [Scripting](Scripting/index.md) | V8 VM, context, bindings (graphics, input, audio), entry script and `OnTick` / `OnInit`. |
| [RHI](RHI/index.md) | Barriers, command list, registry, resources, Vulkan backend. |
| [UI](UI/index.md) | UI context and ImGui integration. |
| [Window](Window/index.md) | Runtime window and platform integration. |

**Reserved:** `Visera.Runtime.World` exists under `Runtime/Source/World/` but is **not** re-exported by `Visera.Runtime`. See [Architecture](../Architecture.md).

## See also

- [Architecture](../Architecture.md) — `FEngineCreateInfo`, service order, `Run()`
- [Engine Core](../Core/index.md)
- [Platform](../Platform/index.md)
