# Core.Traits (Visera.Core.Traits)

**Core.Traits** provides type traits and Flags utilities for generic programming and enum bitmasks. Type traits for compile-time type properties; Flags for combinable enum bits (e.g. `EFileMode::Read | EFileMode::Binary`) with operator|, operator& and VISERA_MAKE_FLAGS macro.

## Responsibilities
- Define or extend standard type traits for use in Core and upper modules in templates.
- [Flags](Flags.md) provides enum flag type and bitwise ops for multi-option enums (e.g. file open mode, stream mode).

## Submodules
| Module | Description |
|------|------|
| [Flags](Flags.md) | Enum flag type and operations. |

## See also
- [Core](../index.md) — Parent module
- [Meta](../Meta/index.md) — Metaprogramming and cast
- [OS.FileSystem](../OS/FileSystem/index.md) — Uses EFileMode, EStreamMode Flags
