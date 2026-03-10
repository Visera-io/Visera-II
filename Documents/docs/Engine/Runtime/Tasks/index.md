# Runtime.Tasks (Visera.Runtime.Tasks)

**Runtime.Tasks** provides the task system: task interface (submit/wait) and scheduler (e.g. thread pool). splits parallel work into tasks and schedules them on multiple threads, with [Core.Concurrency](../../Core/Concurrency/index.md) channels or async; for load, physics, AI and other parallelizable logic. 

## Submodules
| Module | Description |
|------|------|
| [Interface](Interface.md) | task submit and wait interface.  |
| [Scheduler](Scheduler.md) | scheduler implementation (thread pool etc.).  |

## See also
- [Runtime](../index.md) — Parent module
- [Core.Concurrency](../../Core/Concurrency/index.md) — async and channels
- [Core.OS.Thread](../../Core/OS/Thread/index.md) — threads and sync
