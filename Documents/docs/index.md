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
| [Engine → Platform](Engine/Platform/index.md) | Platform: interface, Cross (GLFW/Null), Windows, MacOS |
| [Engine → Runtime](Engine/Runtime/index.md) | Runtime: AssetHub, Audio, Global, Graphics, Input, Physics2D, RHI, Tasks, UI, Window |

## Links

- **Site**: [visera.io](https://visera.io/)
- **Repository**: [Visera-io/Visera-II](https://github.com/Visera-io/Visera-II)
