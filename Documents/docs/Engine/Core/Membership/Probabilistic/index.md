# Core.Membership.Probabilistic (Visera.Core.Membership.Probabilistic)

**Core.Membership.Probabilistic** provides probabilistic set-membership structures that support insert, membership query, and (where applicable) delete, with a small false-positive rate and compact storage.

## Submodules

| Module | Description |
|--------|--------------|
| [CuckooFilter](CuckooFilter.md) | Cuckoo filter: approximate set with Insert / MayContain / Erase; space-efficient, supports deletion. |

## See also

- [Membership](../index.md) — Parent module
- [Containers.Set](../../Containers/Set.md) — Exact set when no false positives are allowed
