# Core.Algorithm (Visera.Core.Algorithm)

**Core.Algorithm** provides C++20 ranges-based algorithms and view wrappers under the Visera namespace and constraint style, for find, sort, binary search, and split on any `std::ranges::range` in the engine. This module is an aggregate; the actual API is in [Ranges](Ranges.md).

## Responsibilities
- Thin wrappers over `std::ranges` algorithms with standard behavior and complexity.
- Comparators (e.g. `Less`) and projection support for use in generic code.
- All functions are templates and work with any type satisfying `range`/`viewable_range` (e.g. `TArray`, `FStringView`, `TSpan`).

## Submodules
| Module | Description |
|------|------|
| [Ranges](Ranges.md) | Range algorithms: NoneOf, Sort, FindIf, Split, BinarySearch, and Less comparator. |

## See also
- [Core](../index.md) — Parent module
- [Ranges](Ranges.md) — API reference
- [Types.String](../Types/String.md) — FString/FStringView as range
- [Containers.Array](../Containers/Array/index.md) — TArray satisfies range concept
