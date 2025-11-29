module;
#include <Visera-Core.hpp>
#include <Eigen/Core>
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
		Float X{0}, Y{0};

		[[nodiscard]] inline Bool
		IsZero() const noexcept { return X == 0.0f && Y == 0.0f; }
		[[nodiscard]] constexpr Bool
		IsNearlyZero() const noexcept { return Math::IsNearlyEqual(X, 0.0f) && Math::IsNearlyEqual(Y, 0.0f); }
		[[nodiscard]] constexpr Float
		Dot(const FVector2F& I_Vector) const { return X * I_Vector.X + Y * I_Vector.Y; }
		[[nodiscard]] constexpr Float
		SquaredNorm() const noexcept { return X * X + Y * Y; }
		[[nodiscard]] inline Float
		L2Norm() const noexcept { return Math::Sqrt(SquaredNorm()); }
		[[nodiscard]] inline Bool
		Normalize() noexcept { Float Norm = L2Norm(); if(Math::IsNearlyEqual(Norm, 0.0f)) { return False; } Float InvNorm = 1.0f / Norm; X *= InvNorm; Y *= InvNorm; return True; }

		constexpr Float&
		operator[](UInt32 I_Index)       { CHECK(I_Index < 2); return (&X)[I_Index]; }
		constexpr const Float&
		operator[](UInt32 I_Index) const { CHECK(I_Index < 2); return (&X)[I_Index]; }
		constexpr FVector2F
		operator+(const FVector2F& I_Vector) const { return {X + I_Vector.X , Y + I_Vector.Y}; }
		constexpr FVector2F
		operator-(const FVector2F& I_Vector) const { return {X - I_Vector.X , Y - I_Vector.Y}; }
		constexpr FVector2F
		operator*(Float I_Factor) const { return {X * I_Factor , Y * I_Factor}; }
		constexpr FVector2F
		operator/(Float I_Factor) const { return {X / I_Factor , Y / I_Factor}; }

		FVector2F() noexcept = default;
		FVector2F(Float I_X, Float I_Y) noexcept : X(I_X), Y(I_Y) {}

		[[nodiscard]] Eigen::Map<Eigen::Vector2f>
		GetView()       { return Eigen::Map<Eigen::Vector2f, Eigen::Unaligned>(&X); }
		[[nodiscard]] Eigen::Map<const Eigen::Vector2f>
		GetView() const { return Eigen::Map<const Eigen::Vector2f, Eigen::Unaligned>(&X); }
	};
	static_assert(sizeof(FVector2F) == 8);

	class VISERA_CORE_API FVector3F
	{
	public:
		Float X{0}, Y{0}, Z{0};

		[[nodiscard]] constexpr Bool
		IsZero() const noexcept { return X == 0.0f && Y == 0.0f && Z == 0.0f; }
		[[nodiscard]] constexpr Bool
		IsNearlyZero() const noexcept { return Math::IsNearlyEqual(X, 0.0f) && Math::IsNearlyEqual(Y, 0.0f) && Math::IsNearlyEqual(Z, 0.0f); }
		[[nodiscard]] constexpr Float
		Dot(const FVector3F& I_Vector) const { return X * I_Vector.X + Y * I_Vector.Y + Z * I_Vector.Z; }
		[[nodiscard]] constexpr Float
		SquaredNorm() const noexcept { return X * X + Y * Y + Z * Z; }
		[[nodiscard]] inline Float
		L2Norm() const noexcept { return Math::Sqrt(SquaredNorm()); }
		[[nodiscard]] inline Bool
		Normalize() noexcept { Float Norm = L2Norm(); if(Math::IsNearlyEqual(Norm, 0.0f)) { return False; } Float InvNorm = 1.0f / Norm; X *= InvNorm; Y *= InvNorm; Z *= InvNorm; return True; }

		constexpr Float&
		operator[](UInt32 I_Index)       { CHECK(I_Index < 3); return (&X)[I_Index]; }
		constexpr const Float&
		operator[](UInt32 I_Index) const { CHECK(I_Index < 3); return (&X)[I_Index]; }
		constexpr FVector3F
		operator+(const FVector3F& I_Vector) const { return {X + I_Vector.X , Y + I_Vector.Y , Z + I_Vector.Z}; }
		constexpr FVector3F
		operator-(const FVector3F& I_Vector) const { return {X - I_Vector.X , Y - I_Vector.Y , Z - I_Vector.Z}; }
		constexpr FVector3F
		operator*(Float I_Factor) const { return {X * I_Factor , Y * I_Factor , Z * I_Factor}; }
		constexpr FVector3F
		operator/(Float I_Factor) const { return {X / I_Factor , Y / I_Factor , Z / I_Factor}; }

		constexpr FVector3F() noexcept = default;
		constexpr FVector3F(Float I_X, Float I_Y, Float I_Z) noexcept : X(I_X), Y(I_Y), Z(I_Z) {}

		[[nodiscard]] Eigen::Map<Eigen::Vector3f>
		GetView()       { return Eigen::Map<Eigen::Vector3f, Eigen::Unaligned>(&X); }
		[[nodiscard]] Eigen::Map<const Eigen::Vector3f>
		GetView() const { return Eigen::Map<const Eigen::Vector3f, Eigen::Unaligned>(&X); }
	};
	static_assert(sizeof(FVector3F) == 12);

	class VISERA_CORE_API FVector4F
	{
	public:
		Float X{0}, Y{0}, Z{0}, W{0};

		[[nodiscard]] inline Bool
		IsZero() const noexcept { return X == 0.0f && Y == 0.0f && Z == 0.0f && W == 0.0f; }
		[[nodiscard]] constexpr Bool
		IsNearlyZero() const noexcept { return Math::IsNearlyEqual(X, 0.0f) && Math::IsNearlyEqual(Y, 0.0f) && Math::IsNearlyEqual(Z, 0.0f) && Math::IsNearlyEqual(W, 0.0f); }
		[[nodiscard]] constexpr Float
		Dot(const FVector4F& I_Vector) const { return X * I_Vector.X + Y * I_Vector.Y + Z * I_Vector.Z + W * I_Vector.W; }
		[[nodiscard]] constexpr Float
		SquaredNorm() const noexcept { return X * X + Y * Y + Z * Z + W * W; }
		[[nodiscard]] inline Float
		L2Norm() const noexcept { return Math::Sqrt(SquaredNorm()); }
		[[nodiscard]] inline Bool
		Normalize() noexcept { Float Norm = L2Norm(); if(Math::IsNearlyEqual(Norm, 0.0f)) { return False; } Float InvNorm = 1.0f / Norm; X *= InvNorm; Y *= InvNorm; Z *= InvNorm; W *= InvNorm; return True; }

		constexpr Float&
		operator[](UInt32 I_Index)       { CHECK(I_Index < 4); return (&X)[I_Index]; }
		constexpr const Float&
		operator[](UInt32 I_Index) const { CHECK(I_Index < 4); return (&X)[I_Index]; }
		constexpr FVector4F
		operator+(const FVector4F& I_Vector) const { return {X + I_Vector.X , Y + I_Vector.Y , Z + I_Vector.Z, W + I_Vector.W}; }
		constexpr FVector4F
		operator-(const FVector4F& I_Vector) const { return {X - I_Vector.X , Y - I_Vector.Y , Z - I_Vector.Z, W - I_Vector.W}; }
		constexpr FVector4F
		operator*(Float I_Factor) const { return {X * I_Factor , Y * I_Factor , Z * I_Factor, W * I_Factor}; }
		constexpr FVector4F
		operator/(Float I_Factor) const { return {X / I_Factor , Y / I_Factor , Z / I_Factor, W / I_Factor}; }

		constexpr FVector4F() noexcept = default;
		constexpr FVector4F(Float I_X, Float I_Y, Float I_Z, Float I_W) noexcept : X(I_X), Y(I_Y), Z(I_Z), W(I_W) {}

		[[nodiscard]] Eigen::Map<Eigen::Vector4f>
		GetView()       { return Eigen::Map<Eigen::Vector4f, Eigen::Unaligned>(&X); }
		[[nodiscard]] Eigen::Map<const Eigen::Vector4f>
		GetView() const { return Eigen::Map<const Eigen::Vector4f, Eigen::Unaligned>(&X); }
	};
	static_assert(sizeof(FVector4F) == 16);

	namespace Concepts
	{
		template<typename T> concept
		Vectorial = std::is_class_v<FVector2F> ||
					std::is_class_v<FVector3F> ||
					std::is_class_v<FVector4F>;
	}

	namespace Math
	{
	// 	template<Concepts::Vectorial T> T
	// 	Normalized(const T& I_Vector) { return I_Vector.normalized(); }
	//
	// 	template<Concepts::Vectorial T> void
	// 	Normalize(T* IO_Vector) { *IO_Vector = GetNormalized(IO_Vector); }
	//
	// 	template<Concepts::Vectorial T> Bool
	// 	IsUnit(const T& I_Vector) { return IsEqual(1.0f, I_Vector.norm()); }
	//
	// 	template<Concepts::Vectorial T> Bool
	// 	IsZero(const T& I_Vector) { return I_Vector.isZero(); }
	//
	// 	template<Concepts::Vectorial T> Bool
	// 	IsIdentity(const T& I_Vector) { return I_Vector.isIdentity(); }
	//
	// 	//Even though this looks like it returns a new vector, Eigen is smart: if a is on both sides, Eigen avoids allocating a temporary and does it in-place behind the scenes (thanks to expression templates and lazy evaluation).
	// 	template<Concepts::Vectorial T> T
	// 	ComponentwiseMax(const T& I_VecA, const T& I_VecB) { return I_VecA.cwiseMax(I_VecB); }
	// 	template<Concepts::Vectorial T> T
	// 	ComponentwiseMin(const T& I_VecA, const T& I_VecB) { return I_VecA.cwiseMin(I_VecB); }
	}

}
VISERA_MAKE_FORMATTER(Visera::FVector2F, {}, "[{}, {}]^T",			I_Formatee.X, I_Formatee.Y)
VISERA_MAKE_FORMATTER(Visera::FVector3F, {}, "[{}, {}, {}]^T",		I_Formatee.X, I_Formatee.Y, I_Formatee.Z)
VISERA_MAKE_FORMATTER(Visera::FVector4F, {}, "[{}, {}, {}, {}]^T",	I_Formatee.X, I_Formatee.Y, I_Formatee.Z, I_Formatee.W)