module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Arithmetic.Interval;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;

export namespace Visera
{
	template <Concepts::Arithmetical T>
	struct FClosedInterval
	{
		T Left{};
		T Right{};

		constexpr FClosedInterval() noexcept = default;

		constexpr FClosedInterval(T I_Left, T I_Right) noexcept
        : Left  (I_Left),
          Right (I_Right)
		{ VISERA_ASSERT(Left <= Right); }

		[[nodiscard]] constexpr Bool
		IsDegenerate() const noexcept { return Left == Right; }
		[[nodiscard]] constexpr T
		Length() const noexcept { return Right - Left; }
		[[nodiscard]] constexpr Bool
		Contains(T I_Value) const noexcept { return (Left <= I_Value) && (I_Value <= Right); }
		[[nodiscard]] constexpr T
		Clamp(T I_Value) const noexcept { return Math::Min(Right, Math::Max(Left, I_Value)); }
		[[nodiscard]] constexpr Bool
		Overlaps(const FClosedInterval& I_Other) const noexcept { return !(I_Other.Right < Left || Right < I_Other.Left); }
		
        [[nodiscard]] constexpr std::optional<FClosedInterval>
		Intersect(const FClosedInterval& I_Other) const noexcept
		{
			T Lo = Math::Max(Left, I_Other.Left);
			T Hi = Math::Min(Right, I_Other.Right);
			if (Hi < Lo) return std::nullopt;
			return FClosedInterval(Lo, Hi);
		}
		[[nodiscard]] constexpr FClosedInterval
		Union(const FClosedInterval& I_Other) const noexcept
		{ return FClosedInterval(Math::Min(Left, I_Other.Left), Math::Max(Right, I_Other.Right)); }

		[[nodiscard]] constexpr Bool
		operator==(const FClosedInterval& I_Rhs) const noexcept
		{ return Left == I_Rhs.Left && Right == I_Rhs.Right; }
		[[nodiscard]] constexpr Bool
		operator!=(const FClosedInterval& I_Rhs) const noexcept
		{ return !(*this == I_Rhs); }
	};
}