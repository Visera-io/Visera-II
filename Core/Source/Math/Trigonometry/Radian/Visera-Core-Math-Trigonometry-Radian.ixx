module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry.Radian;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic;

export namespace Visera
{
    template<Concepts::FloatingPoint T>
    class TRadian
    {
    public:
        T GetValue() const { return Value; }

        explicit constexpr TRadian(T Value) : Value{ Value } {};
        explicit  TRadian() = default;
        TRadian&   operator=(T   NewValue)      { Value = NewValue; return *this; }
        TRadian&   operator=(TRadian NewRadian) { Value = NewRadian.Value; return *this; }

        TRadian operator*(T  I_Multiplicand)	const { return TRadian(Value * I_Multiplicand); };
        TRadian operator/(T  I_Divisor)		    const { return TRadian(Value / I_Divisor); };

        explicit constexpr operator T() const { return static_cast<T>(Value); }

        bool   operator==(TRadian I_Rival)	const { return Math::IsNearlyEqual(  Value, I_Rival.Value);}
        bool   operator!=(TRadian I_Rival)	const { return !Math::IsNearlyEqual( Value, I_Rival.Value);}
        bool   operator<(TRadian  I_Rival)	const { return Value < I_Rival.Value;}
        bool   operator>(TRadian  I_Rival)	const { return Value > I_Rival.Value;}

    private:
        T Value = 0.0;
    };

}

template <Visera::Concepts::FloatingPoint T>
struct fmt::formatter<Visera::TRadian<T>>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::TRadian<T>& I_Radian, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        return fmt::format_to(
            I_Context.out(),
            "{}rad",
            I_Radian.GetValue()
        );
    }
};