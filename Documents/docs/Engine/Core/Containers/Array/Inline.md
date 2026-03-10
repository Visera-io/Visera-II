# Core.Containers.Array.Inline (Visera.Core.Containers.Array.Inline)

Inline-capacity array **TInlineArray&lt;T, N&gt;**; storage is inline (no heap). Logical size 0..N; `GetCapacity()` / `GetMaxSize()` both equal `N`. `operator[]` asserts index &lt; Size; `At(I_Index)` returns `TOptional<std::reference_wrapper<T>>`. Modifiers: `Clear()`, `PushBack()`, `EmplaceBack()`, `PopBack()`, `RemoveAtSwap()`, `Swap()`. Use when the maximum count is known at compile time (e.g. per-frame buffers, small fixed slot sets).

## See also

- [Array](index.md) — parent module
- [Containers](../index.md)
