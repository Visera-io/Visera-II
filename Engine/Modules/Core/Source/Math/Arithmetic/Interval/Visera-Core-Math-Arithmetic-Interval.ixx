module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Arithmetic.Interval;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;

export namespace Visera
{
	template <Concepts::Arithmetical T>
	struct TClosedInterval
	{
		T Left{};
		T Right{};

		constexpr TClosedInterval() noexcept = default;

		constexpr TClosedInterval(T I_Left, T I_Right) noexcept
        : Left  (I_Left),
          Right (I_Right)
		{ if !consteval { VISERA_ASSERT(I_Left <= I_Right); } }

		[[nodiscard]] constexpr Bool
		IsDegenerate() const noexcept { return Left == Right; }
		[[nodiscard]] constexpr T
		Length() const noexcept { return Right - Left; }
		[[nodiscard]] constexpr Bool
		Contains(T I_Value) const noexcept { return (Left <= I_Value) && (I_Value <= Right); }
		[[nodiscard]] constexpr T
		Clamp(T I_Value) const noexcept { return Math::Min(Right, Math::Max(Left, I_Value)); }
		[[nodiscard]] constexpr Bool
		Overlaps(const TClosedInterval& I_Other) const noexcept { return !(I_Other.Right < Left || Right < I_Other.Left); }
		
        [[nodiscard]] constexpr TOptional<TClosedInterval>
		Intersect(const TClosedInterval& I_Other) const noexcept
		{
			T Lo = Math::Max(Left, I_Other.Left);
			T Hi = Math::Min(Right, I_Other.Right);
			if (Hi < Lo) { return std::nullopt; }
			return TClosedInterval(Lo, Hi);
		}
		[[nodiscard]] constexpr TClosedInterval
		Union(const TClosedInterval& I_Other) const noexcept
		{ return TClosedInterval(Math::Min(Left, I_Other.Left), Math::Max(Right, I_Other.Right)); }

		[[nodiscard]] constexpr Bool
		operator==(const TClosedInterval& I_Rhs) const noexcept
		{ return Left == I_Rhs.Left && Right == I_Rhs.Right; }
		[[nodiscard]] constexpr Bool
		operator!=(const TClosedInterval& I_Rhs) const noexcept
		{ return !(*this == I_Rhs); }
	};
}