# Visera Engine

![Visera Logo](assets/images/Visera.png)

**Visera** is a cross-platform engine for modern games and real-time applications. It uses a C++20 modular design and provides a full runtime stack from platform abstraction to rendering, audio, input, and asset pipelines.

## Introduction

Visera uses a layered architecture:

- **Core** — Foundation: math, containers, type system, concurrency, OS abstraction (file, memory, thread, time), logging, compression, and more.
- **Platform** — Platform layer: window, path, file system, and dynamic library loading; supports Windows, macOS, and GLFW-based cross-platform or Null stub implementations.
- **Runtime** — Runtime: RHI (Vulkan), graphics (scene, material, render graph), AssetHub, audio, input, task system, UI (ImGui), and 2D physics.

The **Engine** section in the sidebar follows the same structure as the source: **Core**, **Platform**, and **Runtime**, with one page per module for easy lookup of APIs and concepts.

## Quick links

| Section | Description |
|--------|-------------|
| [Engine → Core](Engine/Core/index.md) | Core library: algorithm, compression, concurrency, containers, delegate, font, image, log, math, meta, OS, traits, types |
| [Engine → Platform](Engine/Platform/index.md) | Platform: interface, GLFW, Null, Windows, MacOS |
| [Engine → Runtime](Engine/Runtime/index.md) | Runtime: AssetHub, Audio, Global, Graphics, Input, Physics2D, RHI, Tasks, UI, Window |

## Recent changes

Summary of recent modifications (engine, game, and tooling):

- **Runtime.Graphics** — Render API uses `FRenderArea`; channel and poison pill unchanged. **Major flow change**: Graphics thread extracts **draw list** from scene data (`ExtractAndSortDrawList` → `FRenderList`: batches by Material/PSO, opaque vs transparent). **FRenderContext** now carries `RenderList` only (no raw `Data`/`RenderView`); draw passes iterate batches. **RegisterPass** write-locked; pass array snapshotted under read lock each frame. **PipelineCache**: hash-based O(1) lookup. **RenderGraph**: `AddPass(FName, ...)`; `Execute(FRenderContext*)` creates command list and submits; **CullDeadPasses** O(N+E) reverse BFS. See [Framework](Engine/Runtime/Graphics/Framework.md), [RenderGraph](Engine/Runtime/Graphics/RenderGraph.md), [PipelineCache](Engine/Runtime/Graphics/PipelineCache.md).
- **Runtime.Graphics.Scene.Camera** — Docs: `FCamera` API, left-handed, lazy matrices, Euler Yaw–Pitch–Roll. See [Camera](Engine/Runtime/Graphics/Scene/Camera.md).
- **Core.Types.Path** — `FPath::NormalizeString` now preserves a leading slash for absolute paths (fixes shader/material loading from app bundle on macOS).
- **Runtime.Input** — Mouse cursor position is in **framebuffer (pixel) space**: on macOS/Retina the GLFW window multiplies cursor coordinates by content scale before notifying Input. Keyboard key storage uses a dense array `Keys[FirstKey..LastKey]` with offset in `GetKey`, avoiding unused slots for low key codes.
- **Platform.GLFW.Window** — Cursor position callback scales by `GetScaleX()`/`GetScaleY()`; `SetContentScale` updates scale when the window content-scale callback fires (e.g. moving to another monitor).
- **Game / Scripting** — Scripting bindings keep a list of created applications (`State->Apps`); `CreateApplication` returns `TSharedPtr<FViseraApp>`. Mouse position exposed to scripts via `GetCursor().Position`. Engine terminate clears `Apps` before resetting the engine.
- **Game (Verdandi)** — Demo uses background material and sky texture, R8G8B8A8_UNorm scene color, black clear; sprite push constants include `GlowRadius` (from left-button hold duration); hit test uses cursor AABB and glow radius; window size read each frame for resize.

## Links

- **Site**: [visera.io](https://visera.io/)
- **Repository**: [Visera-io/Visera-II](https://github.com/Visera-io/Visera-II)
