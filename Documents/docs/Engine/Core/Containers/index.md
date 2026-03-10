# Core.Containers (Visera.Core.Containers)

**Core.Containers** provides unified container types within the engine: dynamic array, map, set, queue, cache (incl. LRU), list (incl. intrusive), and SlotMap. all containers use Visera naming (e.g. `TArray`, `TMap`), and work with [Types](../Types/index.md) for strings, paths, etc.; some containers support custom allocators (e.g. [OS.Memory.Arena](../OS/Memory/Arena.md)). 

## Responsibilities
- **TArray**: dynamic array with contiguous memory; interface similar to `std::vector`, supports iterators, resize, push_back; the most commonly used linear container. 
- **TMap / TSet**: hash-based or ordered associative container for key-value lookup and deduplication. 
- **TQueue**: FIFO queue for BFS or message buffering. 
- **Cache(incl. LRU)**: cache with capacity and eviction policy; often used for resource handle caching or recent-use sets. 
- **List(incl. Intrusive)**: list; intrusive version embeds nodes inside elements to avoid extra allocation; suitable for frequent insert/remove during object lifetime. 
- **SlotMap**: dense storage accessible by stable handle; handle invalidated when element is removed; for entity/component ID etc.. 

## Submodules
| Module | Description |
|------|------|
| [Array](Array/index.md) | TArray; [Inline](Array/Inline.md), [PMR](Array/PMR.md).  |
| [Cache](Cache/index.md) | cache abstraction; [LRU](Cache/LRU.md) implementation.  |
| [List](List/index.md) | list; [Intrusive](List/Intrusive.md) intrusive list.  |
| [Map](Map.md) | associative map `TMap`.  |
| [Queue](Queue.md) | queue `TQueue`.  |
| [Set](Set.md) | set `TSet`.  |
| [SlotMap](SlotMap.md) | dense storage with stable handles.  |

## See also
- [Core](../index.md) — Parent module
- [Types](../Types/index.md) — use with strings, paths, JSON, etc.
- [OS.Memory](../OS/Memory/index.md) — allocator and Arena
