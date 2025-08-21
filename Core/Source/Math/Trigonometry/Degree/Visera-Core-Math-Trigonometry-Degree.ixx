module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry.Degree;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic;

export namespace Visera
{
    template<Concepts::FloatingPoint T>
    class TDegree
    {
    public:
        T GetValue() const { return Value; }

        explicit  constexpr TDegree(T I_Value) : Value{ I_Value } {};
        explicit  TDegree() = default;
        TDegree& operator=(T  I_NewValue)       { Value = I_NewValue; return *this; }
        TDegree& operator=(TDegree I_NewDegree) { Value = I_NewDegree.Value; return *this; }

        explicit constexpr operator T() const { return static_cast<T>(Value); }

        TDegree operator*(T  I_Multiplicand)const { return TDegree(Value * I_Multiplicand); };
        TDegree operator/(T  I_Divisor)	    const { return TDegree(Value / I_Divisor);      };
        bool   operator==(TDegree I_Rival)	const { return Math::IsEqual(  Value, I_Rival.Value);}
        bool   operator!=(TDegree I_Rival)	const { return !Math::IsEqual( Value, I_Rival.Value);}
        bool   operator<(TDegree  I_Rival)	const { return Value < I_Rival.Value;}
        bool   operator>(TDegree  I_Rival)	const { return Value < I_Rival.Value;}

    private:
        T Value = 0.0;
    };

}

template <Visera::Concepts::FloatingPoint T>
struct fmt::formatter<Visera::TDegree<T>>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::TDegree<T>& I_Degree, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        return fmt::format_to(
            I_Context.out(),
            "{}°",
            I_Degree.GetValue()
        );
    }
};