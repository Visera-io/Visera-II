# Platform.Interface (Visera.Platform.Interface)

**Platform.Interface** defines platform-agnostic abstract API: window, path, file system and dynamic library. Platform impl (Windows, MacOS) or cross (GLFW, Null) implement these interfaces so upper layers depend only on interface types for cross-platform build and run; at compile time typedef/using maps interfaces to target impl (e.g. FPlatformWindow → FWindowsWindow).

## Responsibilities
- **Window**: Create, destroy and query window handle, size, etc.; no Win32/Cocoa/GLFW types.
- **Path**: Path representation and platform format conversion (e.g. slash, volume).
- **FileSystem**: Path-based file access abstraction (if distinct from Core.OS, this is platform path and permissions).
- **Library**: Dynamic library load, unload and symbol resolve (GetProcAddress/dlsym abstraction).

## Submodules
| Module | Description |
|------|------|
| [Window](Window.md) | Window abstraction. |
| [Path](Path.md) | Path abstraction. |
| [FileSystem](FileSystem.md) | File system abstraction. |
| [Library](Library.md) | Dynamic library abstraction. |

## See also
- [Platform](../index.md) — Parent module
- [Cross](../Cross/index.md) — GLFW / Null implementation
- [Windows](../Windows/index.md), [MacOS](../MacOS/index.md) — Platform implementation
