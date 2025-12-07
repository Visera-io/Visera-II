module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Algebra.Vector;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic;

//#define VISERA_SAFE_MODE;
#if defined(VISERA_SAFE_MODE)
#define CHECK(I_Statement) VISERA_ASSERT(I_Statement)
#else
#define CHECK(I_Statement) VISERA_NO_OPERATION
#endif

export namespace Visera
{
	class VISERA_CORE_API FVector2F
	{
	public:
		union
		{
			struct { Float X, Y; };
			Float Data[2]{0.0f, 0.0f};
		};

		[[nodiscard]] inline Bool
		IsZero() const noexcept { return X == 0.0f && Y == 0.0f; }
		[[nodiscard]] constexpr Bool
		IsNearlyZero() const noexcept { return Math::IsNearlyEqual(X, 0.0f) && Math::IsNearlyEqual(Y, 0.0f); }
		[[nodiscard]] inline Float
		Dot(const FVector2F& I_Vector) const { return Math::MulAdd(X, I_Vector.X, Y * I_Vector.Y); }
		[[nodiscard]] inline Float
		SquaredNorm() const noexcept { return Math::MulAdd(X, X, Y * Y); }
		[[nodiscard]] inline Float
		L2Norm() const noexcept { return Math::Sqrt(SquaredNorm()); }
		[[nodiscard]] inline Bool
		Normalize() noexcept { Float SqrNorm = SquaredNorm(); if(Math::IsNearlyEqual(SqrNorm, 0.0f)) { return False; } Float InvNorm = 1.0f / Math::Sqrt(SqrNorm); X *= InvNorm; Y *= InvNorm; return True; }

		constexpr Float&
		operator[](UInt32 I_Index)		 noexcept { CHECK(I_Index < 2); return (&X)[I_Index]; }
		constexpr const Float&
		operator[](UInt32 I_Index) const noexcept{ CHECK(I_Index < 2); return (&X)[I_Index]; }
		constexpr FVector2F
		operator+(const FVector2F& I_Vector) const noexcept { return {X + I_Vector.X , Y + I_Vector.Y}; }
		constexpr FVector2F
		operator-(const FVector2F& I_Vector) const noexcept { return {X - I_Vector.X , Y - I_Vector.Y}; }
		constexpr FVector2F
		operator*(Float I_Factor) const noexcept { return {X * I_Factor , Y * I_Factor}; }
		constexpr FVector2F
		operator/(Float I_Factor) const noexcept { CHECK(!Math::IsNearlyZero(I_Factor, 0.0f)); return {X / I_Factor , Y / I_Factor}; }
		constexpr FVector2F&
		operator+=(const FVector2F& I_Vector) noexcept { X += I_Vector.X; Y += I_Vector.Y; return *this; }
		constexpr FVector2F&
		operator-=(const FVector2F& I_Vector) noexcept { X -= I_Vector.X; Y -= I_Vector.Y; return *this; }
		constexpr FVector2F&
		operator*=(Float I_Factor) noexcept { X *= I_Factor; Y *= I_Factor; return *this; }
		constexpr FVector2F&
		operator/=(Float I_Factor) noexcept { CHECK(!Math::IsNearlyZero(I_Factor, 0.0f)); X /= I_Factor; Y /= I_Factor; return *this; }

		constexpr FVector2F() noexcept = default;
		constexpr FVector2F(Float I_X, Float I_Y) noexcept : Data{I_X, I_Y} {}
	};
	static_assert(sizeof(FVector2F) == 8);
	static_assert(std::is_standard_layout_v<FVector2F>);

	class VISERA_CORE_API FVector3F
	{
	public:
		union
		{
			struct { Float X, Y, Z; };
			Float Data[3]{0.0f, 0.0f, 0.0f};
		};

		[[nodiscard]] constexpr Bool
		IsZero() const noexcept { return X == 0.0f && Y == 0.0f && Z == 0.0f; }
		[[nodiscard]] constexpr Bool
		IsNearlyZero() const noexcept { return Math::IsNearlyEqual(X, 0.0f) && Math::IsNearlyEqual(Y, 0.0f) && Math::IsNearlyEqual(Z, 0.0f); }
		[[nodiscard]] inline Float
		Dot(const FVector3F& I_Vector) const { return Math::MulAdd(X, I_Vector.X, Math::MulAdd(Y, I_Vector.Y, Z * I_Vector.Z)); }
		[[nodiscard]] inline Float
		SquaredNorm() const noexcept { return Math::MulAdd(X, X, Math::MulAdd(Y, Y, Z * Z)); }
		[[nodiscard]] inline Float
		L2Norm() const noexcept { return Math::Sqrt(SquaredNorm()); }
		[[nodiscard]] inline Bool
		Normalize() noexcept { Float SqrNorm = SquaredNorm(); if(Math::IsNearlyEqual(SqrNorm, 0.0f)) { return False; } Float InvNorm = 1.0f / Math::Sqrt(SqrNorm); X *= InvNorm; Y *= InvNorm; Z *= InvNorm; return True; }

		constexpr Float&
		operator[](UInt32 I_Index)       noexcept { CHECK(I_Index < 3); return (&X)[I_Index]; }
		constexpr const Float&
		operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 3); return (&X)[I_Index]; }
		constexpr FVector3F
		operator+(const FVector3F& I_Vector) const noexcept { return {X + I_Vector.X , Y + I_Vector.Y , Z + I_Vector.Z}; }
		constexpr FVector3F
		operator-(const FVector3F& I_Vector) const noexcept { return {X - I_Vector.X , Y - I_Vector.Y , Z - I_Vector.Z}; }
		constexpr FVector3F
		operator*(Float I_Factor) const noexcept { return {X * I_Factor , Y * I_Factor , Z * I_Factor}; }
		constexpr FVector3F
		operator/(Float I_Factor) const noexcept { CHECK(!Math::IsNearlyZero(I_Factor, 0.0f)); return {X / I_Factor , Y / I_Factor , Z / I_Factor}; }
		constexpr FVector3F&
		operator+=(const FVector3F& I_Vector) noexcept { X += I_Vector.X; Y += I_Vector.Y; Z += I_Vector.Z; return *this; }
		constexpr FVector3F&
		operator-=(const FVector3F& I_Vector) noexcept { X -= I_Vector.X; Y -= I_Vector.Y; Z -= I_Vector.Z; return *this; }
		constexpr FVector3F&
		operator*=(Float I_Factor) noexcept { X *= I_Factor; Y *= I_Factor; Z *= I_Factor; return *this; }
		constexpr FVector3F&
		operator/=(Float I_Factor) noexcept { CHECK(!Math::IsNearlyZero(I_Factor, 0.0f)); X /= I_Factor; Y /= I_Factor; Z /= I_Factor; return *this; }

		constexpr FVector3F() noexcept = default;
		constexpr FVector3F(Float I_X, Float I_Y, Float I_Z) noexcept : Data{I_X, I_Y, I_Z} {}
	};
	static_assert(sizeof(FVector3F) == 12);
	static_assert(std::is_standard_layout_v<FVector3F>);

	class VISERA_CORE_API FVector4F
	{
	public:
		union
		{
			struct { Float X, Y, Z, W; };
			Float Data[4]{0.0f, 0.0f, 0.0f, 0.0f};
		};

		[[nodiscard]] inline Bool
		IsZero() const noexcept { return X == 0.0f && Y == 0.0f && Z == 0.0f && W == 0.0f; }
		[[nodiscard]] constexpr Bool
		IsNearlyZero() const noexcept { return Math::IsNearlyEqual(X, 0.0f) && Math::IsNearlyEqual(Y, 0.0f) && Math::IsNearlyEqual(Z, 0.0f) && Math::IsNearlyEqual(W, 0.0f); }
		[[nodiscard]] inline Float
		Dot(const FVector4F& I_Vector) const { return Math::MulAdd(X, I_Vector.X, Math::MulAdd(Y, I_Vector.Y, Math::MulAdd(Z, I_Vector.Z, W * I_Vector.W))); }
		[[nodiscard]] inline Float
		SquaredNorm() const noexcept { return Math::MulAdd(X, X, Math::MulAdd(Y, Y, Math::MulAdd(Z, Z, W * W))); }
		[[nodiscard]] inline Float
		L2Norm() const noexcept { return Math::Sqrt(SquaredNorm()); }
		[[nodiscard]] inline Bool
		Normalize() noexcept { Float SqrNorm = SquaredNorm(); if(Math::IsNearlyEqual(SqrNorm, 0.0f)) { return False; } Float InvNorm = 1.0f / Math::Sqrt(SqrNorm); X *= InvNorm; Y *= InvNorm; Z *= InvNorm; W *= InvNorm; return True; }

		constexpr Float&
		operator[](UInt32 I_Index)       noexcept { CHECK(I_Index < 4); return (&X)[I_Index]; }
		constexpr const Float&
		operator[](UInt32 I_Index) const noexcept { CHECK(I_Index < 4); return (&X)[I_Index]; }
		constexpr FVector4F
		operator+(const FVector4F& I_Vector) const noexcept { return {X + I_Vector.X , Y + I_Vector.Y , Z + I_Vector.Z, W + I_Vector.W}; }
		constexpr FVector4F
		operator-(const FVector4F& I_Vector) const noexcept { return {X - I_Vector.X , Y - I_Vector.Y , Z - I_Vector.Z, W - I_Vector.W}; }
		constexpr FVector4F
		operator*(Float I_Factor) const noexcept { return {X * I_Factor , Y * I_Factor , Z * I_Factor, W * I_Factor}; }
		constexpr FVector4F
		operator/(Float I_Factor) const noexcept { CHECK(!Math::IsNearlyZero(I_Factor, 0.0f)); return {X / I_Factor , Y / I_Factor , Z / I_Factor, W / I_Factor}; }
		constexpr FVector4F&
		operator+=(const FVector4F& I_Vector) noexcept { X += I_Vector.X; Y += I_Vector.Y; Z += I_Vector.Z; W += I_Vector.W; return *this; }
		constexpr FVector4F&
		operator-=(const FVector4F& I_Vector) noexcept { X -= I_Vector.X; Y -= I_Vector.Y; Z -= I_Vector.Z; W -= I_Vector.W; return *this; }
		constexpr FVector4F&
		operator*=(Float I_Factor) noexcept { X *= I_Factor; Y *= I_Factor; Z *= I_Factor; W *= I_Factor; return *this; }
		constexpr FVector4F&
		operator/=(Float I_Factor) noexcept { CHECK(!Math::IsNearlyZero(I_Factor, 0.0f)); X /= I_Factor; Y /= I_Factor; Z /= I_Factor; W /= I_Factor; return *this; }

		constexpr FVector4F() noexcept = default;
		constexpr FVector4F(Float I_X, Float I_Y, Float I_Z, Float I_W) noexcept : Data{I_X, I_Y, I_Z, I_W} {}
	};
	static_assert(sizeof(FVector4F) == 16);
	static_assert(std::is_standard_layout_v<FVector4F>);

	namespace Concepts
	{
		template<class T>
		concept Vectorial = std::same_as<std::remove_cvref_t<T>, FVector2F> ||
							std::same_as<std::remove_cvref_t<T>, FVector3F> ||
							std::same_as<std::remove_cvref_t<T>, FVector4F>;
	}
}
VISERA_MAKE_FORMATTER(Visera::FVector2F, {}, "\n| {:>10.6f} |\n| {:>10.6f} |_Vector2F", I_Formatee.X, I_Formatee.Y);
VISERA_MAKE_FORMATTER(Visera::FVector3F, {}, "\n| {:>10.6f} |\n| {:>10.6f} |\n| {:>10.6f} |_Vector3F", I_Formatee.X, I_Formatee.Y, I_Formatee.Z);
VISERA_MAKE_FORMATTER(Visera::FVector4F, {}, "\n| {:>10.6f} |\n| {:>10.6f} |\n| {:>10.6f} |\n| {:>10.6f} |_Vector4F", I_Formatee.X, I_Formatee.Y, I_Formatee.Z, I_Formatee.W);