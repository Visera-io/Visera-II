# Core.Containers.Array (Visera.Core.Containers.Array)

Array container aggregate. **TArray&lt;T&gt;** (dynamic, std::vector-based) is the primary linear container; **TInlineArray&lt;T, N&gt;** and **TPMRArray&lt;T&gt;** are re-exported from submodules.

## TArray&lt;T&gt;

Dynamic array: contiguous storage, O(1) random access and tail insert/remove. Capacity/size: `GetSize()`, `GetCapacity()`, `Reserve()`, `Resize()`, `ShrinkToFit()`. Access: `operator[]`, `At()`, `Front()`, `Back()`, `Data()`. Modifiers: `Clear()`, `PushBack()`, `EmplaceBack()`, `PopBack()`, `Insert()`, `Erase()`, `RemoveAtSwap()`, `Swap()`. Iterators and C++20 range; works with [Algorithm.Ranges](../../Algorithm/Ranges.md). Copy/move when `T` allows.

## Submodules

| Module | Description |
|--------|-------------|
| [Inline](Inline.md) | Inline-capacity array, no heap |
| [PMR](PMR.md) | PMR-backed dynamic array |

## See also

- [Containers](../index.md) — parent module
- [Algorithm.Ranges](../../Algorithm/Ranges.md) — ranges over arrays
- [OS.Memory.Arena](../../OS/Memory/Arena.md) — custom allocator
