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
		template <Concepts::Arithmetical NumT, Concepts::FloatingPoint ReT = std::conditional_t<std::floating_point<NumT>, NumT, Double>> [[nodiscard]] ReT
		Sqrt(NumT I_Num) { return static_cast<ReT>(std::sqrt(I_Num)); }

		template <Concepts::Arithmetical NumT> [[nodiscard]] NumT
		Abs(NumT I_Num) { return std::abs(I_Num); }
		template <Concepts::Arithmetical NumT> [[nodiscard]] NumT
		Max(NumT I_NumA, NumT I_NumB) { return std::max(I_NumA, I_NumB); }
		template <Concepts::Arithmetical NumT> [[nodiscard]] NumT
		Min(NumT I_NumA, NumT I_NumB) { return std::min(I_NumA, I_NumB); }

		template<Concepts::Arithmetical NumT> [[nodiscard]] constexpr NumT
    	Epsilon() { return std::numeric_limits<NumT>::epsilon(); }
    	template<Concepts::Arithmetical NumT> [[nodiscard]] constexpr NumT
		UpperBound() { return std::numeric_limits<NumT>::max(); }
    	template<Concepts::Arithmetical NumT> [[nodiscard]] constexpr NumT
		LowerBound() { return std::numeric_limits<NumT>::min(); }
    	template <Concepts::FloatingPoint T> [[nodiscard]] Bool
		IsNaN(T I_Num) { return std::isnan(I_Num); }
    	template <Concepts::Integeral T> [[nodiscard]] Bool
		IsNaN(T I_Num) { return False; }
    	template <Concepts::FloatingPoint T> [[nodiscard]] Bool
		IsInfinite(T I_Num) { return std::isinf(I_Num); }
    	template <Concepts::Integeral T> [[nodiscard]] Bool
		IsInfinite(T I_Num) { return False; }
    	template <Concepts::FloatingPoint T> [[nodiscard]] Bool
		IsFinite(T I_Num) { return std::isfinite(I_Num); }
    	template <Concepts::Integeral T> [[nodiscard]] Bool
		IsFinite(T I_Num) { return true; }
    	template<Concepts::FloatingPoint NumT> [[nodiscard]] constexpr Bool
		IsNearlyEqual(NumT I_NumA, NumT I_NumB) { return Abs(I_NumA - I_NumB) <= Epsilon<NumT>(); }

    	// More Precise (runtime) A * B + C. Falls back during constant-evaluation.
    	template <Concepts::FloatingPoint T> [[nodiscard]] constexpr T
		MulAdd(T I_A, T I_B, T I_C) noexcept
		{
			if consteval { return I_A * I_B + I_C; }
			return std::fma(I_A, I_B, I_C);
		}

		template<Concepts::Arithmetical NumT> constexpr void
		Clamp(TMutable<NumT> IO_Value, NumT I_Min, NumT I_Max)
		{
			if (I_Min > *IO_Value) { *IO_Value = I_Min; return; }
			if (I_Max < *IO_Value) { *IO_Value = I_Max; return; }
		}

    	template<Concepts::Arithmetical NumT, Concepts::Integeral IntT> [[nodiscard]] Double
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
    
			Double Result = 1.0;
			while (I_Exp)
			{
				if (I_Exp & 1) { Result *= I_Base; }

				I_Base *=  I_Base;
				I_Exp  >>= 1;
			}
			return Result;
		}

    	template<Concepts::Arithmetical NumT, Concepts::FloatingPoint FloatT, Concepts::FloatingPoint ReT = std::conditional_t<std::floating_point<NumT>, NumT, Double>> [[nodiscard]] ReT
		Pow(NumT I_Base, FloatT I_Exp)
		{ return static_cast<ReT>(std::pow(I_Base, I_Exp)); }
	}
}