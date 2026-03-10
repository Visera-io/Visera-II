# Core.OS.Thread (Visera.Core.OS.Thread)

**Core.OS.Thread** provides thread-related primitives: lock-free queues (MPSC, SPSC) and sync primitives (atomic, critical section, event, RW lock, spinlock). Thread creation is by caller or runtime; this module provides inter-thread communication and mutex tools for engine multithreading and [Concurrency](../../Concurrency/index.md) channels.

## Submodules
| Module | Description |
|------|------|
| [Queue](Queue/index.md) | Lock-free queue [MPSC](Queue/MPSC.md), [SPSC](Queue/SPSC.md). |
| [Sync](Sync/index.md) | [Atomic](Sync/Atomic.md), [CriticalSection](Sync/CriticalSection.md), [Event](Sync/Event.md), [RWLock](Sync/RWLock.md), [SpinLock](Sync/SpinLock.md). |

## See also
- [OS](../index.md) — Parent module
- [Concurrency](../../Concurrency/index.md) — Async and channels
- [Memory](../Memory/index.md) — Memory and allocation
