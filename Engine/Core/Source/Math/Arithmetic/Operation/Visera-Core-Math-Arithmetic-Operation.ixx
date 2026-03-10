module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Arithmetic.Operation;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Constants;
import Visera.Core.Limits.Numeric;

export namespace Visera::Concepts
{
	template<typename NumT> concept
	Arithmetical = std::is_arithmetic_v<NumT>;
}

export namespace Visera::Math
{
	template <Concepts::Arithmetical NumT, Concepts::FloatingPoint ReT = std::conditional_t<std::floating_point<NumT>, NumT, Double>> [[nodiscard]] ReT
	Sqrt(NumT I_Num) noexcept { return static_cast<ReT>(std::sqrt(I_Num)); }

	template <Concepts::Arithmetical NumT> [[nodiscard]] NumT
	Abs(NumT I_Num) noexcept { return std::abs(I_Num); }
	template <Concepts::Arithmetical NumT> [[nodiscard]] NumT
	Max(NumT I_NumA, NumT I_NumB) noexcept { return std::max(I_NumA, I_NumB); }
	template <Concepts::Arithmetical NumT> [[nodiscard]] NumT
	Min(NumT I_NumA, NumT I_NumB) noexcept { return std::min(I_NumA, I_NumB); }

	template<Concepts::Arithmetical NumT> [[nodiscard]] constexpr NumT
	Epsilon() noexcept { return Limits::Epsilon<NumT>(); }
	template<Concepts::Arithmetical NumT> [[nodiscard]] constexpr NumT
	UpperBound() noexcept { return Limits::UpperBound<NumT>(); }
	template<Concepts::Arithmetical NumT> [[nodiscard]] constexpr NumT
	LowerBound() noexcept { return Limits::LowerBound<NumT>(); }
	template<Concepts::Arithmetical BoundT, Concepts::Arithmetical NumT> [[nodiscard]] constexpr Bool
	IsWithinBounds(NumT I_Num) noexcept
	{
		// Avoid lossy implicit conversions when NumT is floating-point and BoundT is a wide integer
		// (e.g., comparing float to Int64 bounds).
		if constexpr (std::is_floating_point_v<NumT> || std::is_floating_point_v<BoundT>)
		{
			using CT    = long double;
			const CT V  = static_cast<CT>(I_Num);
			const CT Lo = static_cast<CT>(LowerBound<BoundT>());
			const CT Hi = static_cast<CT>(UpperBound<BoundT>());
			return Lo <= V && V <= Hi;
		}
		else
		{
			// Integral-only path: signed/unsigned-safe comparisons.
			return LowerBound<BoundT>() <= I_Num && I_Num <= UpperBound<BoundT>();
		}
	}
    template <Concepts::FloatingPoint T> [[nodiscard]] constexpr Bool
	IsNaN(T I_Num) noexcept { return std::isnan(I_Num); }
    template <Concepts::Integral T> [[nodiscard]] constexpr Bool
	IsNaN(T I_Num) noexcept { return False; }
    template <Concepts::FloatingPoint T> [[nodiscard]] constexpr Bool
	IsInfinite(T I_Num) noexcept { return std::isinf(I_Num); }
    template <Concepts::Integral T> [[nodiscard]] constexpr Bool
	IsInfinite(T I_Num) noexcept { return False; }
    template <Concepts::FloatingPoint T> [[nodiscard]] constexpr Bool
	IsFinite(T I_Num) noexcept { return std::isfinite(I_Num); }
    template <Concepts::Integral T> [[nodiscard]] constexpr Bool
	IsFinite(T I_Num) noexcept { return True; }
    [[nodiscard]] inline Bool
	IsNearlyEqual(Float I_NumA, Float I_NumB, Float I_Tolerance = 1E-8f) noexcept { return Abs(I_NumA - I_NumB) <= I_Tolerance; }
	[[nodiscard]] inline Bool
	IsNearlyEqual(Double I_NumA, Double I_NumB, Double I_Tolerance = 1E-8) noexcept { return Abs(I_NumA - I_NumB) <= I_Tolerance; }
	template<Concepts::FloatingPoint T> T
	Round(T I_Value) noexcept { return std::round(I_Value); }
    template<Concepts::FloatingPoint T> T
	Ceil(T I_Value) noexcept { return std::ceil(I_Value); }
    template<Concepts::FloatingPoint T> T
	Floor(T I_Value) noexcept { return std::floor(I_Value); }
	template<Concepts::FloatingPoint T> T
	Truncate(T I_Value) noexcept { return std::trunc(I_Value); }

    // More Precise (runtime) A * B + C. Falls back during constant-evaluation.
    template <Concepts::FloatingPoint T> [[nodiscard]] constexpr T
	MulAdd(T I_A, T I_B, T I_C) noexcept
	{
		if consteval { return I_A * I_B + I_C; }
		return std::fma(I_A, I_B, I_C);
	}

	template<Concepts::Arithmetical NumT> constexpr void
	Clamp(NumT* IO_Value, NumT I_Min, NumT I_Max) noexcept
	{
		if (I_Min > *IO_Value) { *IO_Value = I_Min; return; }
		if (I_Max < *IO_Value) { *IO_Value = I_Max; return; }
	}

    template<Concepts::Arithmetical NumT> constexpr NumT
	Clamp(NumT I_Value, NumT I_Min, NumT I_Max) noexcept
	{
		if (I_Min > I_Value) { return I_Min; }
		if (I_Max < I_Value) { return I_Max; }
		return I_Value;
	}

	/// Exponential function: e^x
	template<Concepts::FloatingPoint T> [[nodiscard]] T
	Exp(T I_X) noexcept { return static_cast<T>(std::exp(I_X)); }

	/// Base-2 exponential function: 2^x
	template<Concepts::FloatingPoint T> [[nodiscard]] T
	Exp2(T I_X) noexcept { return static_cast<T>(std::exp2(I_X)); }

	// Fast exponentiation (binary exponentiation). Constexpr for use at compile time and runtime.
	template<Concepts::Arithmetical NumT, Concepts::Integral IntT, Concepts::FloatingPoint ReT = std::conditional_t<std::floating_point<NumT>, NumT, Double>>
	[[nodiscard]] constexpr ReT
	Pow(NumT I_Base, IntT I_Exp) noexcept
	{
		if constexpr (std::is_signed_v<IntT>) if (I_Exp < 0)
		{
			ReT Base = static_cast<ReT>(I_Base);

			if (I_Exp == LowerBound<IntT>())
			{
				// -(INT_MIN) Overflow
				ReT P = Pow(Base, -(I_Exp + 1));
				return ReT(1) / (P * Base);
			}
			return ReT(1) / Pow(Base, -I_Exp);
		}

		if (I_Base == 2) if constexpr (sizeof(IntT) <= sizeof(Int32))
		{
			return std::ldexp(ReT(1), static_cast<Int32>(I_Exp));
		}

		auto Result = ReT(1);
		auto Base = static_cast<ReT>(I_Base);
		auto Exp = static_cast<std::make_unsigned_t<IntT>>(I_Exp);
		while (Exp)
		{
			if (Exp & 1) { Result *= Base; }
			Base *=  Base;
			Exp  >>= 1;
		}
		return Result;
	}

    template<Concepts::Arithmetical NumT, Concepts::FloatingPoint FloatT, Concepts::FloatingPoint ReT = std::conditional_t<std::floating_point<NumT>, NumT, Double>>
    [[nodiscard]] ReT
	Pow(NumT I_Base, FloatT I_Exp) noexcept
	{
		if (I_Base == 2) { return Exp2(I_Exp); }

		// NaN / Inf to libm
		if (!IsFinite(I_Exp))
		{ return static_cast<ReT>(std::pow(static_cast<ReT>(I_Base), static_cast<ReT>(I_Exp))); }

		// Fast Pow
		if (auto TruncatedExp = Truncate(I_Exp); TruncatedExp == I_Exp && IsWithinBounds<Int64>(TruncatedExp))
		{
			return Pow<NumT, Int64, ReT>(I_Base, static_cast<Int64>(TruncatedExp));
		}

		// Fallback
		return static_cast<ReT>(std::pow(static_cast<ReT>(I_Base), I_Exp));
	}
}