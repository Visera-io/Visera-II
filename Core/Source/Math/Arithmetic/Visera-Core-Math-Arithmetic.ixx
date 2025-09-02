module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Arithmetic;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera
{
    namespace Concepts
	{
		template<typename T> concept
		Arithmetical = std::is_arithmetic_v<T>;
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

		template<Concepts::Arithmetical T> constexpr auto
		Epsilon() { return std::numeric_limits<T>::epsilon(); }

		template<Concepts::Arithmetical T> constexpr Bool
		IsEqual(T I_NumA, T I_NumB) { return Abs(I_NumA - I_NumB) <= Epsilon<T>(); }

		template<Concepts::Arithmetical T> constexpr auto
		UpperBound() { return std::numeric_limits<T>::max(); }
		template<Concepts::Arithmetical T> constexpr auto
		LowerBound() { return std::numeric_limits<T>::min(); }

		template<Concepts::Arithmetical NumT> constexpr void
		Clamp(TMutable<NumT> IO_Value, NumT I_Min, NumT I_Max)
		{
			if (I_Min > *IO_Value) { *IO_Value = I_Min; return; }
			if (I_Max < *IO_Value) { *IO_Value = I_Max; return; }
		}
	}
}