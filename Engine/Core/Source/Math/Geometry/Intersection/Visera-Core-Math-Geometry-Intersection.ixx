module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Geometry.Intersection;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Geometry.Box;
import Visera.Core.Math.Geometry.Circle;

export namespace Visera
{
	[[nodiscard]] constexpr Bool
	Overlaps(const FBox2F& I_Box, const FCircle2F& I_Circle) noexcept
	{
		const Float ClosestX = Math::Clamp(I_Circle.Center.X, I_Box.Min.X, I_Box.Max.X);
		const Float ClosestY = Math::Clamp(I_Circle.Center.Y, I_Box.Min.Y, I_Box.Max.Y);
		const Float Dx = I_Circle.Center.X - ClosestX;
		const Float Dy = I_Circle.Center.Y - ClosestY;
		return Math::MulAdd(Dx, Dx, Dy * Dy) <= I_Circle.Radius * I_Circle.Radius;
	}

	[[nodiscard]] constexpr Bool
	Overlaps(const FCircle2F& I_Circle, const FBox2F& I_Box) noexcept
	{
		return Overlaps(I_Box, I_Circle);
	}

	[[nodiscard]] constexpr Bool
	Overlaps(const FBox2F& I_A, const FBox2F& I_B) noexcept
	{
		return I_A.Min.X <= I_B.Max.X && I_A.Max.X >= I_B.Min.X
			&& I_A.Min.Y <= I_B.Max.Y && I_A.Max.Y >= I_B.Min.Y;
	}

	[[nodiscard]] constexpr Bool
	Overlaps(const FCircle2F& I_A, const FCircle2F& I_B) noexcept
	{
		const Float Dx = I_A.Center.X - I_B.Center.X;
		const Float Dy = I_A.Center.Y - I_B.Center.Y;
		const Float SumR = I_A.Radius + I_B.Radius;
		return Math::MulAdd(Dx, Dx, Dy * Dy) <= SumR * SumR;
	}
}