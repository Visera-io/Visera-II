module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry.Degree;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
    class VISERA_CORE_API FDegree
    {
    public:
        Float Value{0.0f};

        constexpr FDegree() noexcept = default;
        explicit constexpr FDegree(Float I_Value) noexcept : Value{ I_Value } {}

        constexpr FDegree&
        operator=(Float I_NewValue) noexcept { Value = I_NewValue; return *this; }
        constexpr FDegree&
        operator=(const FDegree& I_NewDegree) noexcept { Value = I_NewDegree.Value; return *this; }

        [[nodiscard]] constexpr FDegree
        operator*(Float I_Multiplicand) const noexcept { return FDegree{ Value * I_Multiplicand }; }
        [[nodiscard]] constexpr FDegree
        operator/(Float I_Divisor) const noexcept
        {
            if consteval { /* no CHECK in constant evaluation */ }
            else { CHECK(!Math::IsNearlyEqual(I_Divisor, 0.0f)); }
            return FDegree{ Value / I_Divisor };
        }

        [[nodiscard]] constexpr Bool
        operator==(FDegree I_Rival) const noexcept { return Math::IsNearlyEqual(Value, I_Rival.Value); }
        [[nodiscard]] constexpr Bool
        operator!=(FDegree I_Rival) const noexcept { return !Math::IsNearlyEqual(Value, I_Rival.Value); }
        [[nodiscard]] constexpr Bool
        operator<(FDegree I_Rival) const noexcept { return Value < I_Rival.Value; }
        [[nodiscard]] constexpr Bool
        operator>(FDegree I_Rival) const noexcept { return Value > I_Rival.Value; }
    };
}
VISERA_MAKE_FORMATTER(Visera::FDegree, {}, "{}°", I_Formatee.Value);