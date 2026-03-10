# Core.Limits.Numeric (Visera.Core.Limits.Numeric)

**Limits.Numeric** exposes numeric bounds and epsilon for arithmetic types in namespace `Visera::Limits`, wrapping `std::numeric_limits`.

## API overview

| Function | Description |
|----------|-------------|
| `Limits::Epsilon<NumT>()` | `std::numeric_limits<NumT>::epsilon()`; meaningful for floating-point types. |
| `Limits::UpperBound<NumT>()` | `std::numeric_limits<NumT>::max()`. |
| `Limits::LowerBound<NumT>()` | `std::numeric_limits<NumT>::lowest()`. |

All are `constexpr` and `noexcept`. `NumT` must satisfy `std::is_arithmetic_v<NumT>`.

## Usage

```cpp
import Visera.Core.Limits.Numeric;

// Overflow check
VISERA_ASSERT(I_NumBuckets <= (Limits::UpperBound<UInt64>() - sizeof(UInt64)) / kBytesPerBucket);

// Signed range from unsigned RNG
if (V <= static_cast<UInt32>(Limits::UpperBound<Int32>()))
    return static_cast<Int32>(V);
return V - Limits::UpperBound<Int32>() + Limits::LowerBound<Int32>();

// Float epsilon
Float Tol = Limits::Epsilon<Float>();
```

## See also

- [Limits](index.md) — Parent module
- [Math.Arithmetic.Operation](../Math/Arithmetic/Operation.md) — `Math::Epsilon`, `Math::UpperBound`, `Math::LowerBound` delegate to Limits
