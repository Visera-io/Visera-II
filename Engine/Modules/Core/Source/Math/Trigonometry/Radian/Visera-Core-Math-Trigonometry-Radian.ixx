module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry.Radian;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
    class VISERA_CORE_API FRadian
    {
    public:
        Float Value{0.0f};

        constexpr FRadian() noexcept = default;
        explicit constexpr FRadian(Float I_Value) noexcept : Value{ I_Value } {}

        constexpr FRadian&
        operator=(Float I_NewValue) noexcept { Value = I_NewValue; return *this; }
        constexpr FRadian&
        operator=(const FRadian& I_NewRadian) noexcept { Value = I_NewRadian.Value; return *this; }

        [[nodiscard]] constexpr FRadian
        operator*(Float I_Multiplicand) const noexcept { return FRadian{ Value * I_Multiplicand }; }
        [[nodiscard]] constexpr FRadian
        operator/(Float I_Divisor) const noexcept
        {
            if consteval { /* no CHECK in constant evaluation */ }
            else { CHECK(!Math::IsNearlyEqual(I_Divisor, 0.0f)); }
            return FRadian{ Value / I_Divisor };
        }

        [[nodiscard]] constexpr Bool
        operator==(FRadian I_Rival) const noexcept { return Math::IsNearlyEqual(Value, I_Rival.Value); }
        [[nodiscard]] constexpr Bool
        operator!=(FRadian I_Rival) const noexcept { return !Math::IsNearlyEqual(Value, I_Rival.Value); }
        [[nodiscard]] constexpr Bool
        operator<(FRadian I_Rival) const noexcept { return Value < I_Rival.Value; }
        [[nodiscard]] constexpr Bool
        operator>(FRadian I_Rival) const noexcept { return Value > I_Rival.Value; }
    };
}
VISERA_MAKE_FORMATTER(Visera::FRadian, {}, "{}rad", I_Formatee.Value);