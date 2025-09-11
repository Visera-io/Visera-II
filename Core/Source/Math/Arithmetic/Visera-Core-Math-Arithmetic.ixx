module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Arithmetic;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera
{
    namespace Concepts
	{
		template<typename NumT> concept
		Arithmetical = std::is_arithmetic_v<NumT>;
	}

	namespace Math
	{
		template <Concepts::Arithmetical NumT, Concepts::Arithmetical ReT = NumT> ReT
		Sqrt(NumT I_Num) { return std::sqrt(I_Num); }

		template <Concepts::Arithmetical NumT> NumT
		Abs(NumT I_Num) { return std::abs(I_Num); }
		template <Concepts::Arithmetical NumT> NumT
		Max(NumT I_NumA, NumT I_NumB) { return std::max(I_NumA, I_NumB); }
		template <Concepts::Arithmetical NumT> NumT
		Min(NumT I_NumA, NumT I_NumB) { return std::min(I_NumA, I_NumB); }

		template<Concepts::Arithmetical NumT> constexpr auto
		Epsilon() { return std::numeric_limits<NumT>::epsilon(); }

		template<Concepts::Arithmetical NumT> constexpr Bool
		IsEqual(NumT I_NumA, NumT I_NumB) { return Abs(I_NumA - I_NumB) <= Epsilon<NumT>(); }

		template<Concepts::Arithmetical NumT> constexpr auto
		UpperBound() { return std::numeric_limits<NumT>::max(); }
		template<Concepts::Arithmetical NumT> constexpr auto
		LowerBound() { return std::numeric_limits<NumT>::min(); }

		template<Concepts::Arithmetical NumT> constexpr void
		Clamp(TMutable<NumT> IO_Value, NumT I_Min, NumT I_Max)
		{
			if (I_Min > *IO_Value) { *IO_Value = I_Min; return; }
			if (I_Max < *IO_Value) { *IO_Value = I_Max; return; }
		}

    	template<Concepts::Integeral NumT> Bool
		IsPowerOfTwo(NumT I_Number) { return (I_Number > 0) && ((I_Number & (I_Number - 1)) == 0); }

    	template<Concepts::Arithmetical NumT, Concepts::Integeral IntT> NumT
		Pow(NumT I_Base, IntT I_Exp)
    	{
			if (I_Exp < 0)
			{
				// For integer types, negative exponents would truncate to 0, so we use double
				if constexpr (std::is_integral_v<NumT>)
				{ return static_cast<NumT>(1.0 / Pow(static_cast<Double>(I_Base), -I_Exp)); }
				else
				{ return static_cast<NumT>(1.0 / Pow(I_Base, -I_Exp)); }
			}
    
			NumT Result = 1;
			while (I_Exp)
			{
				if (I_Exp & 1) { Result *= I_Base; }

				I_Base *= I_Base;
				I_Exp  >>= 1;
			}
			return Result;
		}

    	template<Concepts::Arithmetical NumT, Concepts::FloatingPoint FloatT> NumT
		Pow(NumT I_Base, FloatT I_Exp)
		{ return static_cast<NumT>(std::pow(I_Base, I_Exp)); }
	}
}