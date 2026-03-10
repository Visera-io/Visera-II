# Runtime (Visera.Runtime)

**Runtime** is the application-facing layer: RHI (Vulkan), graphics (scene, material, render graph), AssetHub, audio, input, task system, UI (ImGui), 2D physics, and window/swapchain integration. It depends on Core and Platform and is what games and editors link against.

## Overview

- **Module**: `Visera.Runtime`
- **Source path**: `Engine/Runtime/Source/`
- **Dependencies**: Depends on [Core](../Core/index.md) and [Platform](../Platform/index.md); does not depend on Forge or other tooling.
- **Build**: Built as library/modules and linked by the application.

## Submodules

| Module | Description |
|--------|-------------|
| [AssetHub](AssetHub/index.md) | Asset loading and management: image (PNG/EXR), font (FreeType), shader. |
| [Audio](Audio/index.md) | Audio interface, Null backend, Wwise and Wwise-IO. |
| [Global](Global/index.md) | Global configuration and services. |
| [Graphics](Graphics/index.md) | Framework, material, pipeline cache, render graph, scene (camera, light, renderable). |
| [Input](Input/index.md) | Input actions, mapping, devices (keyboard, mouse). |
| [Physics2D](Physics2D/index.md) | 2D physics: rigid body, common types. |
| [RHI](RHI/index.md) | RHI: barriers, command list, resources (buffer, texture, sampler, descriptor set, shader, render pass, compute pass), Vulkan backend. |
| [Tasks](Tasks/index.md) | Task system: interface, scheduler. |
| [UI](UI/index.md) | UI context and ImGui integration. |
| [Window](Window/index.md) | Window and swapchain binding. |

## See also

- [Engine Core](../Core/index.md) — Core types and OS
- [Platform](../Platform/index.md) — Platform window and file system
