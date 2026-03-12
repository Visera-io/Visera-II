# Platform (Visera.Platform)

**Platform** provides OS-specific abstraction and implementation: window creation and management, file system path and access, and dynamic library (DLL/dylib) loading and symbol resolution. The implementation is selected by build target (Windows or Apple); for cross-platform or headless use, **GLFW** (window + Device keyboard/mouse) and **Null** (no window, no file system, stub device) are available at the same level as Windows/MacOS.

## Overview

- **Module**: `Visera.Platform`
- **Source path**: `Engine/Platform/Source/`
- **Dependencies**: Depends on [Core](../Core/index.md) (Path, Text, Optional, Containers, Meta); does not depend on Runtime.
- **Build**: Macros (e.g. `VISERA_ON_WINDOWS_SYSTEM`, `VISERA_ON_APPLE_SYSTEM`) select Windows or MacOS at compile time; GLFW and Null are built alongside and can be used for cross-platform or headless builds.

Platform exposes unified type names (e.g. `FPlatformWindow`, `FPlatformPath`, `FPlatformFileSystem`) that map to Windows/MacOS concrete types so that upper layers (e.g. [Runtime.Window](../Runtime/Window/index.md)) only need to depend on the Platform interface for cross-platform builds.

## Submodules

| Module | Description |
|--------|-------------|
| [Interface](Interface/index.md) | Platform-agnostic abstract API: window, path, file system, dynamic library; implemented by each backend. |
| [GLFW](GLFW/index.md) | Cross-platform: window, **Device** (keyboard, mouse), event loop, file system. |
| [Null](Null/index.md) | Headless/stub: no window, no file system (`GetFileSystem()` returns `nullptr`), empty device enums. |
| [Windows](Windows/index.md) | Windows: window, path, file system, DLL loading; **Device** re-exports GLFW key/mouse types. |
| [MacOS](MacOS/index.md) | MacOS: window, path, file system, dylib loading; **Device** re-exports GLFW key/mouse types. |

## Difference from Core.OS

- **Core.OS**: Platform-agnostic file I/O (e.g. `FFile`, `FFileSystem::OpenFile`), memory, thread, time; based on std or generic APIs.
- **Platform**: OS-specific path format conversion, window handles, and dynamic library loading, tied to Win32/Cocoa/GLFW. Use both together: e.g. resolve path with Platform, then open file with Core.OS.FileSystem.

## See also

- [Engine Core](../Core/index.md) — Core types used by Platform
- [Runtime.Window](../Runtime/Window/index.md) — Runtime window and swapchain integration
