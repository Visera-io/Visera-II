# Core.Indexing (Visera.Core.Indexing)

**Core.Indexing** provides set-membership and spatial indexing: probabilistic (approximate) membership, and 2D spatial structures for broadphase and range queries.

## Responsibilities

- **Probabilistic**: Cuckoo filter and related types for membership queries with configurable bits-per-item and capacity; small false-positive rate, optional deletion.
- **Spatial**: `FSpatialHash2D` for 2D point-like entities: fixed-size hashed buckets, insert by position, query by axis-aligned box or circle; suitable for collision broadphase (e.g. clear each frame, insert all, then query range).
- **Use cases**: Deduplication, cache/session “seen” sets; 2D broadphase, cursor hit-test, and range-based culling.

## Submodules

| Module | Description |
|--------|--------------|
| [CuckooFilter](CuckooFilter.md) | Probabilistic approximate set: Insert / MayContain / Erase; space-efficient, supports deletion. |
| [SpatialHash](SpatialHash.md) | 2D spatial hash: Insert(entity, position), QueryRange(box or circle, callback); bucket iteration via GetFirstInCell / GetNext. |

## See also

- [Core](../index.md) — Parent module
- [Containers](../Containers/index.md) — Exact set (`TSet`) when false positives are not acceptable
- [Math.Hash](../Math/Hash/index.md) — Hashing used by membership filters
