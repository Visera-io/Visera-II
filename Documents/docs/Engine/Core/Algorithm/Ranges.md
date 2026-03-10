# Core.Algorithm.Ranges (Visera.Core.Algorithm.Ranges)

**Core.Algorithm.Ranges** provides C++20 ranges-based algorithms and views: predicate checks, sort, find, split view, and binary search, plus the default comparator `Less`. All APIs are under `Visera::Algorithm` and accept any type satisfying the relevant range concept.

## Responsibilities
- **Predicates and find**: `NoneOf` checks if no element satisfies the predicate; `FindIf` returns the iterator to the first satisfying element.
- **Sort**: `Sort` sorts a range in place with optional comparator and projection.
- **Views**: `Split` splits a range by a pattern into subrange views without copying.
- **Binary search**: `BinarySearch` finds the subrange equal to a value in a sorted range (equal_range), O(log N), with projection and custom compare; default uses `Less` (operator<).
- **Comparator**: `Less` is a binary comparator using operator<, used as default for BinarySearch etc.

## API overview

| API | Description |
|-----|------|
| `NoneOf(range, pred)` | Returns true if no element makes pred true. |
| `Sort(range, comp)` | Sorts range in place; comp defaults to std::ranges::less. |
| `FindIf(range, pred)` | Returns iterator to first element that makes pred true. |
| `Split(range, pattern)` | Returns a view of subranges split by pattern. |
| `Less` | Functor: operator()(a, b) is a < b. |
| `BinarySearch(range, value, proj, comp)` | Finds subrange equal to value in sorted range; proj is projection, comp defaults to Less. |

## Example

```cpp
import Visera.Core.Algorithm.Ranges;
import Visera.Core.Containers.Array;

using namespace Visera;
using namespace Visera::Algorithm;

TArray<int> arr = { 3, 1, 4, 1, 5 };
Sort(arr);  // sort in place
auto [lo, hi] = BinarySearch(arr, 1);  // subrange of all elements equal to 1
Bool none = NoneOf(arr, [](int x) { return x > 10; });  // true
```

## See also
- [Algorithm](index.md) — Parent module
- [Core](../index.md)
- [Containers.Array](../Containers/Array.md) — TArray satisfies random_access_range etc.
