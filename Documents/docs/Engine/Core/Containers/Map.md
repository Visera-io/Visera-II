# Core.Containers.Map (Visera.Core.Containers.Map)

**Core.Containers.Map** provides associative container `TMap` (key-value map). O(1) or O(log n) lookup, insert, and remove by key; keys are unique. Implementation may be hash-based or ordered (see source). For resource tables, name-to-handle, config entries, and other key-value scenarios.

## Responsibilities
- Stores key-value pairs; key and value types from template parameters.
- Provides Find, Contains, Insert, Remove, iteration; consistent with other engine containers.
- Shares key constraint (uniqueness) with [Set](Set.md); use `TSet` for key-only set.

## See also
- [Containers](index.md) — Parent module
- [Set](Set.md) — Set (no value, keys only)
- [Types.Name](../Types/Name/index.md) — often used as Map key type
