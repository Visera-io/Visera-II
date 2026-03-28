# Visera Engine

![Visera Logo](assets/images/Visera.png)

**Visera** is a cross-platform engine for modern games and real-time applications. It uses **C++23** with C++ modules (`.ixx`) and provides a full runtime stack from platform abstraction to rendering, audio, input, scripting, and asset pipelines.

## Introduction

Visera uses a layered architecture:

- **Core** — Foundation: math, containers, type system, concurrency, OS abstraction (file, memory, thread, time), logging, compression, and more.
- **Platform** — Window, path, file system, and dynamic library loading: Windows and macOS native backends, plus **GLFW** and **Null** implementations compiled alongside them. The CMake configuration for this tree targets **Windows and Apple** hosts only.
- **Runtime** — RHI (Vulkan), graphics (scene, material, render graph), AssetHub, audio, input, UI (ImGui), window, and **JavaScript scripting** (V8).
- **Forge** (optional) — Separate **`Visera-Forge`** toolchain executable for shaders and batch utilities, controlled by **`VISERA_BUILD_FORGE`**.

The **Engine** section in the sidebar matches the source layout: **Architecture**, **Repository layout**, **Forge**, **Core**, **Platform**, and **Runtime**, with pages keyed by module names.

## Quick links

| Section | Description |
|--------|-------------|
| [Engine → Architecture](Engine/Architecture.md) | Layer diagram, `Visera` module, `FViseraEngine`, Standard vs Forge mode, service dependencies |
| [Engine → Repository layout](Engine/Repository-Layout.md) | `Core`, `Platform`, `Runtime`, `Forge`, `APIs`, `Schemas`, `Shaders` directories |
| [Engine → Forge](Engine/Forge/index.md) | Optional `Visera-Forge` executable and `Visera.Forge` modules |
| [Engine → Core](Engine/Core/index.md) | Core library: algorithm, compression, concurrency, containers, delegate, font, image, log, math, meta, OS, traits, types |
| [Engine → Platform](Engine/Platform/index.md) | Platform: interface, GLFW, Null, Windows, MacOS |
| [Engine → Runtime](Engine/Runtime/index.md) | Runtime: AssetHub, Audio, Graphics, Input, Scripting, RHI, UI, Window |

## Recent changes

Summary of recent modifications (engine, game, and tooling):

- **Runtime.Graphics** — **Immediate-mode draw API**: `FScene` removed. Game submits per-frame via `SetCamera(FCamera)`, `SubmitLight(FLight)`, `Draw(const IRenderable&)` or `Draw(const FRenderableMeta&)`. Data is captured at submit time (`FRenderableMeta`: InstanceData + Material + Mesh); no `TSharedPtr<IRenderable>` crosses the thread boundary. Engine calls `Render(FWindow*)` after `OnPreRender` to build `FRenderTask` from accumulated draws/camera/lights and send to the Graphics thread. **Engine loop order**: `OnTick` → `OnPreRender` → `Graphics->Render(Window)` → `OnPostRender`. **FRenderData** holds `DrawCommands` (`TArray<FRenderableMeta>`) and `Lights`; **BatchAndSort** iterates draw commands. Channel, **RegisterPass**, **PipelineCache**, **RenderGraph** unchanged. See [Framework](Engine/Runtime/Graphics/Framework.md), [RenderGraph](Engine/Runtime/Graphics/RenderGraph.md), [Scene.Renderable](Engine/Runtime/Graphics/Scene/Renderable.md).
- **Runtime.Graphics.Scene.Camera** — Docs: `FCamera` API, left-handed, lazy matrices, Euler Yaw–Pitch–Roll. See [Camera](Engine/Runtime/Graphics/Scene/Camera.md).
- **Core.Types.Path** — `FPath::NormalizeString` now preserves a leading slash for absolute paths (fixes shader/material loading from app bundle on macOS).
- **Runtime.Input** — Mouse cursor position is in **framebuffer (pixel) space**: on macOS/Retina the GLFW window multiplies cursor coordinates by content scale before notifying Input. Keyboard key storage uses a dense array `Keys[FirstKey..LastKey]` with offset in `GetKey`, avoiding unused slots for low key codes.
- **Platform.GLFW.Window** — Cursor position callback scales by `GetScaleX()`/`GetScaleY()`; `SetContentScale` updates scale when the window content-scale callback fires (e.g. moving to another monitor).
- **Game / Scripting** — Scripting bindings keep a list of created applications (`State->Apps`); `CreateApplication` returns `TSharedPtr<FViseraApp>`. Mouse position exposed to scripts via `GetCursor().Position`. Engine terminate clears `Apps` before resetting the engine.
- **Game (Verdandi)** — Demo uses background material and sky texture, R8G8B8A8_UNorm scene color, black clear; sprite push constants include `GlowRadius` (from left-button hold duration); hit test uses cursor AABB and glow radius; window size read each frame for resize.

## Links

- **Site**: [visera.io](https://visera.io/)
- **Repository**: [Visera-io/Visera-II](https://github.com/Visera-io/Visera-II)
