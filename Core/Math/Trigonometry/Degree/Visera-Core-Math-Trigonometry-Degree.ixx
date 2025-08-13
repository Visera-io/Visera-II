module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry.Degree;
#define VISERA_MODULE_NAME "Core.Math.Trigonometry.Degree"
import Visera.Core.Math.Arithmetic;

export namespace Visera
{
    template<Concepts::FloatingPoint T>
    class TDegree
    {
    public:
        static const inline
        TDegree PI { 180.0 };
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