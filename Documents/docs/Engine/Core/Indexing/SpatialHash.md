# Core.Indexing.SpatialHash (FSpatialHash2D)

**FSpatialHash2D** is a fixed-size hashed bucket broadphase for 2D point-like entities. It is data-oriented, avoids runtime allocation, and is cache-friendly (e.g. for Vampire Survivors–style dense entities).

## Usage

- Call **Clear()** each frame, then **Insert(entityId, position)** for all entities.
- **QueryRange(box, callback)** or **QueryRange(circle, callback)** invokes the callback for each entity ID in cells overlapping the range. The same entity may be reported more than once (multiple cells hash to the same bucket); callers should post-filter (e.g. distance or AABB) and deduplicate (e.g. frame stamp or visited set by entity ID) as needed.
- **GetFirstInCell(position)** and **GetNext(currentId)** iterate a single bucket by cell position.

## Types

- **FEntityID**: `UInt32`; **InvalidID**: `0xFFFFFFFFu`.
- Constructor: `FSpatialHash2D(cellSize, numBuckets, maxEntities)`; `numBuckets` must be a power of two.

## See also

- [Indexing](index.md) — Parent module
- [Math.Geometry.Intersection](../Math/Geometry/index.md) — Used for circle–box overlap in circle QueryRange
