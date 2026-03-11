# Core.Indexing.CuckooFilter (FCuckooFilter)

**Cuckoo filter** is a probabilistic set that supports insert, membership query, and delete. It is more space-efficient than classic Bloom filters for low false-positive rates and supports deletion without a counting variant.

- **Original**: [efficient/cuckoofilter](https://github.com/efficient/cuckoofilter); paper "Cuckoo Filter: Practically Better Than Bloom" (ACM CoNEXT 2014), Bin Fan, Dave Andersen, Michael Kaminsky.
- **Module**: `Visera.Core.Indexing.CuckooFilter` (exported via `Visera.Core.Indexing`).

## API overview

| Method | Description |
|--------|-------------|
| `explicit FCuckooFilter(UInt64 I_MaxNumKeys)` | Construct with expected max number of keys; table size is derived (power-of-two, load factor &le; 0.96). |
| `Bool Insert(const ItemType& I_Item)` | Insert item. Returns `False` only when the victim cache is already full; otherwise always succeeds (item may be stored in table or in victim). |
| `Bool MayContain(const ItemType& I_Item) const` | Membership query. May return true for items never inserted (false positive). |
| `Bool Erase(const ItemType& I_Item)` | Remove one matching fingerprint. Only call for items known to have been inserted; erasing on a false-positive match can remove another item's fingerprint. |
| `UInt64 GetSize() const` | Current number of items (logical count). |
| `UInt64 GetSizeInBytes() const` | Table size in bytes. |
| `FString GetInfo() const` | Debug string: items, capacity, load factor, victim_used, table_bytes. |

## Template parameters

- **ItemType**: Element type (e.g. `UInt64`, `FStringView`). Must work with the hash family.
- **BitsPerItem**: Fingerprint size in bits; in `[4, 32]`. Larger values reduce false positives and increase size.
- **HashFamily**: Functor with `UInt64 operator()(const ItemType&) const`. Default `DefaultCuckooHasher<ItemType>` uses `GoldenRatioHash` (seeded) or `CityHash64` for `FStringView` (unseeded).

## Semantics

- **Duplicate inserts**: Allowed; multiple fingerprints for the same item can be stored. `Erase` removes one matching fingerprint per call.
- **False positives**: `MayContain` can return `True` for items never inserted. Do not rely on `Erase` for items that might be false positives, or another item's fingerprint may be removed.
- **Insert failure**: `Insert` returns `False` only when the internal victim cache is already full (eviction chain could not place the item). Otherwise it always returns `True`.

## Constraints

- **Platform**: Requires little-endian (enforced by `#error` if `VISERA_ON_LITTLE_ENDIAN_PLATFORM` is not defined).
- **Capacity**: Bucket count is at most 2^32 (asserted in constructor).

## Example

```cpp
import Visera.Core.Indexing;

FCuckooFilter<UInt64, 12> Filter(1000000);
Filter.Insert(42);
if (Filter.MayContain(42)) { /* true (or false positive) */ }
Filter.Erase(42);  // only if 42 was actually inserted
```

## See also

- [Indexing](index.md) — Indexing overview
- [Math.Hash](../../Math/Hash/index.md) — GoldenRatio, CityHash used by default hasher
