# Core.Concurrency (Visera.Core.Concurrency)

**Core.Concurrency** provides concurrency utilities: async execution and lock-free MPSC (multi-producer/single-consumer) and SPSC channels. For task queues, event dispatch, and lock-free message passing between threads; does not depend on OS thread primitives (threads and locks are in [OS.Thread](../OS/Thread/index.md)).

## Responsibilities
- **Async**: Async execution and Future-style interface (if provided) for running work in the background and getting results.
- **Channel**: Lock-free queue abstraction; MPSC allows multiple producers to one consumer; SPSC is single-producer single-consumer with lower latency. For work stealing, event-bus producer side, etc.

Difference from [OS.Thread](../OS/Thread/index.md): OS.Thread provides threads, mutexes, condition variables, atomics, and thread-local queues; Concurrency focuses on task/message channels and async patterns. Both can be combined.

## Submodules
| Module | Description |
|------|------|
| [Async](Async.md) | Async execution and related types. |
| [Channel](Channel/index.md) | Channel aggregate; [MPSC](Channel/MPSC.md) and [SPSC](Channel/SPSC.md) implementation. |

## See also
- [Core](../index.md) — Parent module
- [OS.Thread](../OS/Thread/index.md) — Threads, sync, and thread-safe queues
- [Delegate](../Delegate/index.md) — Callbacks and events
