# Core.Limits (Visera.Core.Limits)

**Core.Limits** provides numeric bounds and epsilon helpers, replacing direct use of `std::numeric_limits` with a single module so the rest of the engine can depend on a stable API.

## Responsibilities

- **Numeric**: `Epsilon<NumT>()`, `UpperBound<NumT>()`, `LowerBound<NumT>()` for arithmetic types; wraps `std::numeric_limits` in namespace `Visera::Limits`.
- **Use cases**: Overflow checks, range validation, and anywhere a type’s max/min or epsilon is needed without including `<limits>` or `Math.Arithmetic.Operation`.

## Submodules

| Module | Description |
|--------|-------------|
| [Numeric](Numeric.md) | `Epsilon`, `UpperBound`, `LowerBound` for arithmetic types. |

## See also

- [Core](../index.md) — Parent module
- [Math.Arithmetic.Operation](../Math/Arithmetic/Operation.md) — Re-exposes Limits for backward compatibility as `Math::Epsilon`, `Math::UpperBound`, `Math::LowerBound`
