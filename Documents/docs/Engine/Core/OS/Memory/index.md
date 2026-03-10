# Core.OS.Memory (Visera.Core.OS.Memory)

**Core.OS.Memory** provides memory ops and allocator abstraction. Global memory copy utilities and [Arena](Arena.md) allocator: a linear (bump) allocator for frame- or phase-local batch allocation, bulk free or reset, less fragmentation and allocator overhead.

## Responsibilities
- Platform-agnostic memory copy, move or zero (if any) for Core and upper layers.
- [Arena](Arena.md): Sequential allocation from a block; no per-block free; release on destroy or reset. For temp buffers, frame data, parse intermediates.

## Submodules
| Module | Description |
|------|------|
| [Arena](Arena.md) | Arena (bump) allocator. |

## See also
- [OS](../index.md) — Parent module
- [Containers](../../Containers/index.md) — Containers can configure allocator
- [Compression](../../Compression/index.md) — Compression buffers may use Arena
