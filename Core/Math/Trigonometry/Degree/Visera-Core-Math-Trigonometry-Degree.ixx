module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry.Degree;
#define VISERA_MODULE_NAME "Core.Math.Trigonometry.Degree"
import Visera.Core.Math.Arithmetic;

export namespace Visera
{
    template<Concepts::FloatingPoint T = Float>
    class FDegree
    {
    public:
        T GetValue() {return Value; }

        constexpr FDegree(T Value) :Value{ Value } {};
        explicit  FDegree() = default;
        FDegree& operator=(T  NewValue)       { Value = NewValue; return *this; }
        FDegree& operator=(FDegree NewDegree) { Value = NewDegree.Value; return *this; }

        operator Float() const { return Value; }

        FDegree operator*(T  I_Multiplier)	const;
        FDegree operator/(T  Divisor)	    const;
        bool   operator==(FDegree Rival)	const { return Math::IsEqual(  Value, Rival.Value);}
        bool   operator!=(FDegree Rival)	const { return !Math::IsEqual( Value, Rival.Value);}
        bool   operator<(FDegree  Rival)	const { return Value < Rival.Value;}
        bool   operator>(FDegree  Rival)	const { return Value < Rival.Value;}

    private:
        T Value = 0.0;
    };
}