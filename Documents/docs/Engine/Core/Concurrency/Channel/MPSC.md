# Core.Concurrency.Channel.MPSC (Visera.Core.Concurrency.Channel.MPSC)

**Core.Concurrency.Channel.MPSC** provides a lock-free multi-producer single-consumer (MPSC) queue: multiple threads can push, one consumer thread pops. For work queues, event queues, or task dispatch: multiple producers submit, one worker thread processes in order, no locking, less contention.

## Responsibilities
- Ensures thread safety and ordering for multi-threaded push and single-threaded pop; typically implemented with atomics and lock-free list or ring buffer.
- Compared to [SPSC](SPSC.md), MPSC allows more producers, for multi-source single-sink pipelines or event aggregation.

## See also
- [Channel](index.md) — Parent module
- [SPSC](SPSC.md) — Single-producer single-consumer variant
- [OS.Thread.Queue.MPSC](../../OS/Thread/Queue/MPSC.md) — OS-level thread queue (can be used with or instead of this module)
