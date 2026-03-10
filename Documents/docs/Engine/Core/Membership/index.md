# Core.Membership (Visera.Core.Membership)

**Core.Membership** provides approximate set-membership data structures. These are space-efficient containers for answering “is this item in the set?” with a small probability of false positives; some support insertion and deletion.

## Responsibilities

- **Probabilistic structures**: Cuckoo filter and related types for membership queries with configurable bits-per-item and capacity.
- **Use cases**: Deduplication, cache/session “seen” sets, and anywhere a compact approximate set is needed with optional deletion.

## Submodules

| Module | Description |
|--------|--------------|
| [Probabilistic](Probabilistic/index.md) | Probabilistic membership: [CuckooFilter](Probabilistic/CuckooFilter.md). |

## See also

- [Core](../index.md) — Parent module
- [Containers](../Containers/index.md) — Exact set (`TSet`) when false positives are not acceptable
- [Math.Hash](../Math/Hash/index.md) — Hashing used by membership filters
