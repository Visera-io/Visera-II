# Core.Containers.Array (Visera.Core.Containers.Array)

**Core.Containers.Array** provides dynamic array type `TArray<T>`, the most commonly used linear container in the engine. Internally based on `std::vector<T>`; provides std-compatible iterators, capacity management and copy/move semantics; supports copy when `std::copy_constructible` etc., always supports move. 

## Responsibilities
- Stores same-type elements in contiguous memory; O(1) random access and tail insert/remove.
- Provides `Size()`, `Capacity()`, `Resize()`, `Reserve()`, `PushBack()`, `EmplaceBack()`, `PopBack()` and related APIs.
- Construct from iterator range, initializer list or repeat value; configurable allocator (default std allocator or [OS.Memory.Arena](../OS/Memory/Arena.md) etc.).

## Main types and usage

| Type / Concept | Description |
|------------|------|
| `TArray<T>` | Dynamic array of type `T`; iterator and reference types align with `std::vector`. |
| Iterator | `begin`/`end`, `rbegin`/`rend`; satisfies C++20 range; works with [Algorithm.Ranges](../Algorithm/Ranges.md). |
| Construction | Default, by count, by count+value, iterator range, `initializer_list`; copy ctor/assign when `T` is copyable. |

## See also
- [Containers](index.md) — Parent module
- [Algorithm.Ranges](../Algorithm/Ranges.md) — sort, find etc. on TArray
- [OS.Memory.Arena](../OS/Memory/Arena.md) — custom allocator
