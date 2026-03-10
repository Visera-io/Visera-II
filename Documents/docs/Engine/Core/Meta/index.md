# Core.Meta (Visera.Core.Meta)

**Core.Meta** provides metaprogramming and type conversion utilities for compile-time type checks and safe/unsafe runtime casts. With [Traits](../Traits/index.md) it supports type-trait queries and down/cross casts; used in engine internals and generic utilities.

## Responsibilities
- Type-trait-based queries and branching (e.g. if constexpr with trait checks).
- [Cast](Cast.md) provides safe downcast (dynamic_cast style) or unsafe pointer/reference cast for polymorphic hierarchies and low-level interop.
- Optional non-RTTI implementation may be provided by this module or Traits for RTTI-disabled builds.

## Submodules
| Module | Description |
|------|------|
| [Cast](Cast.md) | Safe and unsafe cast helpers. |

## See also
- [Core](../index.md) — Parent module
- [Traits](../Traits/index.md) — Type traits and Flags
- [Types](../Types/index.md) — Type system
