module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Interpolation;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Interval;
import Visera.Core.Math.Arithmetic.Operation;

export namespace Visera::Math
{
	/// Saturate: clamps to [0, 1] (commonly used for normalized params).
	template<Concepts::Arithmetical T>
	[[nodiscard]] constexpr T
	Saturate(T I_X) noexcept
	{
		return Clamp<T>(I_X, T(0), T(1));
	}

	/// Step: returns 0 if x < edge else 1. Matches typical shader semantics.
	template<Concepts::Arithmetical T>
	[[nodiscard]] constexpr T
	Step(T I_Edge, T I_X) noexcept
	{
		return (I_X < I_Edge) ? T(0) : T(1);
	}

	/// SmoothStep: classic smooth Hermite interpolation within interval.
	/// If interval is degenerate, returns 0 (or 1 depending on x).
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	SmoothStep(const TClosedInterval<T>& I_Interval, T I_X) noexcept
	{
		if (I_Interval.IsDegenerate())
		{
			// Degenerate range: behave like a step at interval.Left.
			return (I_X < I_Interval.Left) ? T(0) : T(1);
		}

		const T t = Saturate((I_X - I_Interval.Left) / I_Interval.Length());
		// t*t*(3-2*t)
		return t * t * (T(3) - T(2) * t);
	}

	// ------------------------------------------------------------
	// Interpolation / mapping
	// ------------------------------------------------------------

	/// Lerp: linear interpolation.
	/// Uses MulAdd for better precision: a + (b-a)*t
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	Lerp(T I_A, T I_B, T I_T) noexcept
	{
		// (b - a) * t + a
		return MulAdd((I_B - I_A), I_T, I_A);
	}

	/// InverseLerp: returns t such that Lerp(interval,t)=x (unclamped).
	/// If interval is degenerate, returns 0.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	InverseLerp(const TClosedInterval<T>& I_Interval, T I_X) noexcept
	{
		if (I_Interval.IsDegenerate()) { return T(0); }
		return (I_X - I_Interval.Left) / I_Interval.Length();
	}

	/// InverseLerpClamped: same but clamps result to [0,1]
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	InverseLerpClamped(const TClosedInterval<T>& I_Interval, T I_X) noexcept
	{
		return Saturate(InverseLerp(I_Interval, I_X));
	}

	/// Remap: maps x from input interval to output interval (unclamped)
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	Remap(T I_Value, const TClosedInterval<T>& I_SrcInterval, const TClosedInterval<T>& I_DstInterval) noexcept
	{
		return Lerp(I_DstInterval.Left, I_DstInterval.Right, InverseLerp(I_SrcInterval, I_Value));
	}

	/// RemapClamped: remap but clamps t to [0,1]
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	RemapClamped(const TClosedInterval<T>& I_InputInterval, const TClosedInterval<T>& I_OutputInterval, T I_X) noexcept
	{
		const T t = InverseLerpClamped(I_InputInterval, I_X);
		return Lerp(I_OutputInterval.Left, I_OutputInterval.Right, t);
	}

	/// Quad ease in: slow start, fast end.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	EaseInQuad(T I_T) noexcept
	{
		return I_T * I_T;
	}

	/// Quad ease out: fast start, slow end.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	EaseOutQuad(T I_T) noexcept
	{
		// 1 - (1 - t)^2
		const T U = T(1) - I_T;
		return T(1) - U * U;
	}

	/// Quad ease in/out: slow start & end, fast middle.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	EaseInOutQuad(T I_T) noexcept
	{
		// piecewise for better shape
		if (I_T < T(0.5))
		{ return T(2) * I_T * I_T; }

		// 1 - 2*(1-t)^2
		const T U = T(1) - I_T;
		return T(1) - T(2) * U * U;
	}

	/// Cubic ease out: stronger "snap" than quad.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	EaseOutCubic(T I_T) noexcept
	{
		// 1 - (1 - t)^3
		const  T U = T(1) - I_T;
		return T(1) - U * U * U;
	}

	/// Back ease out: overshoots then returns. Great for UI.
	/// s controls overshoot amount. Unity-like default ~1.70158.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] constexpr T
	EaseOutBack(T I_T, T I_S = T(1.70158)) noexcept
	{
		// 1 + (s+1)*(t-1)^3 + s*(t-1)^2
		const T U = I_T - T(1);
		return MulAdd((I_S + T(1)), U * U * U,  MulAdd(I_S, U * U, T(1)));
	}

	/// Expo ease out: almost instant to near-1, then slowly finishes.
	/// Defined so EaseOutExpo(0)=0, EaseOutExpo(1)=1.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] inline T
	EaseOutExp(T I_T) noexcept
	{
		// NOTE: uses Exp2 -> not constexpr on most libs; keep as inline runtime.
		if (I_T <= T(0)) { return T(0); }
		if (I_T >= T(1)) { return T(1); }
		// 1 - 2^(-10t)
		return T(1) - Exp2(-T(10) * I_T);
	}

	/// Smooth damp-ish follow (simple exponential decay)
	/// lambda: higher = faster convergence. Typical 8~20 for UI, 4~12 for camera follow.
	template<Concepts::FloatingPoint T>
	[[nodiscard]] inline T
	DecayExp(T I_Current, T I_Target, T I_Lambda, T I_Dt) noexcept
	{
		// current + (target-current) * (1 - exp(-lambda*dt))
		// Not constexpr due to exp; keep runtime.
		const  T Alpha = T(1) - Exp(-I_Lambda * I_Dt);
		return I_Current + (I_Target - I_Current) * Alpha;
	}
} // namespace Visera::Math