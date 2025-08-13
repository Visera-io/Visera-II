module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry.Radian;
#define VISERA_MODULE_NAME "Core.Math.Trigonometry.Radian"
import Visera.Core.Math.Arithmetic;

export namespace Visera
{
    template<Concepts::FloatingPoint T = Float>
    class FRadian
    {
    public:
        T GetValue() { return Value; }

        constexpr FRadian(T Value) : Value{ Value } {};
        explicit  FRadian() = default;
        FRadian&   operator=(T   NewValue)      { Value = NewValue; return *this; }
        FRadian&   operator=(FRadian NewRadian) { Value = NewRadian.Value; return *this; }

        operator Float() const { return Value; }

        FRadian operator*(T  I_Multiplicand)	const { return FRadian(Value * I_Multiplicand); };
        FRadian operator/(T  I_Divisor)		    const { return FRadian(Value / I_Divisor); };

        bool   operator==(FRadian I_Rival)	const { return Math::IsEqual(  Value, I_Rival.Value);}
        bool   operator!=(FRadian I_Rival)	const { return !Math::IsEqual( Value, I_Rival.Value);}
        bool   operator<(FRadian  I_Rival)	const { return Value < I_Rival.Value;}
        bool   operator>(FRadian  I_Rival)	const { return Value > I_Rival.Value;}

    private:
        T Value = 0.0;
    };

}