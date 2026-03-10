# Core.OS (Visera.Core.OS)

**Core.OS** provides OS-related base capabilities: filesystem and file I/O, memory allocation and Arena, thread and sync primitives (atomic, critical section, event, RW lock, spinlock), and time (clock, duration, time point). All APIs are in `Visera` namespace; implementation uses std or platform API for consistent cross-platform behavior.

## Responsibilities
- **FileSystem**: Path resolution, file open (stream or FFile), read/write mode and error state (EIOStatus, EFileMode, EStreamMode).
- **Memory**: Global memory ops and [Arena](Memory/Arena.md) allocator for frame or local fast allocation.
- **Thread**: Thread creation is by caller or runtime; this module provides thread-safe queues (MPSC/SPSC), atomic, critical section, event, RW lock, spinlock for sync and lock-free communication.
- **Time**: Clock, Duration, TimePoint and common time types for frame rate, timing and scheduling.

Difference from [Platform](../../Platform/index.md): Platform provides **platform-specific** window and path (Windows/macOS/GLFW); OS provides **platform-agnostic** interface and common implementation (e.g. file I/O via std::fstream/FILE*).

## Submodules
| Module | Description |
|------|------|
| [FileSystem](FileSystem/index.md) | Filesystem abstraction and [File](FileSystem/File.md) handle. |
| [Memory](Memory/index.md) | Memory and [Arena](Memory/Arena.md). |
| [Thread](Thread/index.md) | Thread queues (MPSC/SPSC) and [Sync](Thread/Sync/index.md) primitives. |
| [Time](Time/index.md) | [Clock](Time/Clock.md), [Duration](Time/Duration.md), [TimePoint](Time/TimePoint.md). |

## See also
- [Core](../index.md) — Parent module
- [Platform](../../Platform/index.md) — Platform-specific path and window
- [Concurrency](../Concurrency/index.md) — Async and channels
