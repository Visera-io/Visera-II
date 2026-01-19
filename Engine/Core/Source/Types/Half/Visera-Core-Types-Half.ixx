module;
#include <Visera-Core.hpp>
#include <Imath/half.h>
export module Visera.Core.Types.Half;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    /// @brief 16-bit floating point type based on Imath::half (IEEE 754-2008 binary16)
    /// 
    /// FHalf can represent positive and negative numbers whose magnitude is between
    /// roughly 6.1e-5 and 6.5e+4 with a relative error of 9.8e-4.
    /// All integers from -2048 to +2048 can be represented exactly.
    class VISERA_CORE_API FHalf
    {
    public:
        [[nodiscard]] static FHalf
        FromBits(UInt16 I_Bits) noexcept
        {
            FHalf Result;
            Result.Value = Imath::half(Imath::half::FromBits, I_Bits);
            return Result;
        }
        [[nodiscard]] UInt16
        ToBits() const noexcept { return Value.bits(); }
        [[nodiscard]] Bool
        IsZero() const noexcept { return Value.isZero(); }
        [[nodiscard]] Bool
        IsFinite() const noexcept { return Value.isFinite(); }
        [[nodiscard]] Bool
        IsNaN() const noexcept { return Value.isNan(); }
        [[nodiscard]] Bool
        IsInfinity() const noexcept { return Value.isInfinity(); }
        [[nodiscard]] Bool
        IsNegative() const noexcept { return Value.isNegative(); }

        Imath::half Value;

        operator Float() const noexcept { return static_cast<Float>(Value); }

        operator Double() const noexcept { return static_cast<Double>(static_cast<Float>(Value)); }

        [[nodiscard]] FHalf
        operator-() const noexcept
        {
            FHalf Result;
            Result.Value = -Value;
            return Result;
        }

        FHalf& operator+=(const FHalf& I_Other) noexcept
        {
            Value += I_Other.Value;
            return *this;
        }

        FHalf& operator+=(Float I_Value) noexcept
        {
            Value += I_Value;
            return *this;
        }

        FHalf& operator-=(const FHalf& I_Other) noexcept
        {
            Value -= I_Other.Value;
            return *this;
        }

        FHalf& operator-=(Float I_Value) noexcept
        {
            Value -= I_Value;
            return *this;
        }

        FHalf& operator*=(const FHalf& I_Other) noexcept
        {
            Value *= I_Other.Value;
            return *this;
        }

        FHalf& operator*=(Float I_Value) noexcept
        {
            Value *= I_Value;
            return *this;
        }

        FHalf& operator/=(const FHalf& I_Other) noexcept
        {
            Value /= I_Other.Value;
            return *this;
        }

        FHalf& operator/=(Float I_Value) noexcept
        {
            Value /= I_Value;
            return *this;
        }

        FHalf() = default;

        FHalf(Float I_Value) noexcept : Value(I_Value) {}

        FHalf(Double I_Value) noexcept : Value(static_cast<Float>(I_Value)) {}

        FHalf(const FHalf&) noexcept = default;

        FHalf(FHalf&&) noexcept = default;

        ~FHalf() noexcept = default;

        FHalf& operator=(const FHalf&) noexcept = default;

        FHalf& operator=(FHalf&&) noexcept = default;

        FHalf& operator=(Float I_Value) noexcept
        {
            Value = I_Value;
            return *this;
        }

        FHalf& operator=(Double I_Value) noexcept
        {
            Value = static_cast<Float>(I_Value);
            return *this;
        }
    };

    [[nodiscard]] inline FHalf operator+(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator+(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result += I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator+(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result = I_Rhs;
        Result += I_Lhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator-(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result -= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator-(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result -= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator-(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result(I_Lhs);
        Result -= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator*(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result *= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator*(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result *= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator*(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result = I_Rhs;
        Result *= I_Lhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator/(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result /= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator/(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        FHalf Result = I_Lhs;
        Result /= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline FHalf operator/(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        FHalf Result(I_Lhs);
        Result /= I_Rhs;
        return Result;
    }

    [[nodiscard]] inline Bool operator==(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) == static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator==(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) == I_Rhs;
    }

    [[nodiscard]] inline Bool operator==(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return I_Lhs == static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator!=(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator!=(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return !(I_Lhs == I_Rhs);
    }

    [[nodiscard]] inline Bool operator<(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) < static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator<(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) < I_Rhs;
    }

    [[nodiscard]] inline Bool operator<(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return I_Lhs < static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator<=(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) <= static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator<=(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) <= I_Rhs;
    }

    [[nodiscard]] inline Bool operator<=(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return I_Lhs <= static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator>(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) > static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator>(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) > I_Rhs;
    }

    [[nodiscard]] inline Bool operator>(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return I_Lhs > static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator>=(const FHalf& I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) >= static_cast<Float>(I_Rhs.Value);
    }

    [[nodiscard]] inline Bool operator>=(const FHalf& I_Lhs, Float I_Rhs) noexcept
    {
        return static_cast<Float>(I_Lhs.Value) >= I_Rhs;
    }

    [[nodiscard]] inline Bool operator>=(Float I_Lhs, const FHalf& I_Rhs) noexcept
    {
        return I_Lhs >= static_cast<Float>(I_Rhs.Value);
    }
}
VISERA_MAKE_HASH(Visera::FHalf, return I_Object.ToBits(); );
VISERA_MAKE_FORMATTER(Visera::FHalf, {}, "{} (Float16)", static_cast<Visera::Float>(I_Formatee));
