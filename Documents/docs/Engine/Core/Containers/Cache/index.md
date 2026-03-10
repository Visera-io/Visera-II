# Core.Containers.Cache (Visera.Core.Containers.Cache)

**Core.Containers.Cache** provides cache container abstraction; current implementation is [LRU](LRU.md) (Least Recently Used). For capacity limits and eviction policy: resource handle cache, recent-file list, etc.; when full, evicts least recently used entries.

## Responsibilities
- Fixed or maximum capacity; evicts old entries by policy when over limit.
- LRU implementation maintains access order and evicts least recently used; supports find, insert, update, remove.
- Can use [Containers.Map](../Map.md) or internal hash for O(1) lookup and order maintenance.

## Submodules
| Module | Description |
|------|------|
| [LRU](LRU.md) | LRU cache implementation. |

## See also
- [Containers](../index.md) — Parent module
- [Map](../Map.md) — Key-value storage
- [Runtime.AssetHub](../../../Runtime/AssetHub/index.md) — Resource cache may use Cache
