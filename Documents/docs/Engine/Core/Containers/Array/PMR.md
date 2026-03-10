# Core.Containers.Array.PMR (Visera.Core.Containers.Array.PMR)

PMR-backed dynamic array **TPMRArray&lt;T&gt;**; same interface as [TArray](index.md) but uses `std::pmr::vector<T>`. All constructors accept an optional `std::pmr::memory_resource*` (default from `Memory::GetDefaultResource()`). Use when allocating from a specific PMR (e.g. [OS.Memory.Arena](../../OS/Memory/Arena.md)).

## See also

- [Array](index.md) — parent module
- [Containers](../index.md)
- [OS.Memory.Arena](../../OS/Memory/Arena.md) — arena allocator
