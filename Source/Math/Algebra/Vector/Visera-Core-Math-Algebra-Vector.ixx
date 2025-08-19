module;
#include <Visera-Core.hpp>
#include <Eigen/Dense>
export module Visera.Core.Math.Algebra.Vector;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera
{
	using FVectorNF = Eigen::VectorX<Float>;
	using FVector2F = Eigen::Vector2<Float>;
	using FVector3F = Eigen::Vector3<Float>;
	using FVector4F = Eigen::Vector4<Float>;

	namespace Concepts
	{
		template<typename T> concept
		Vectorial = std::is_class_v<FVectorNF> ||
					std::is_class_v<FVector2F> ||
					std::is_class_v<FVector3F> ||
					std::is_class_v<FVector4F>;
	}

	namespace Math
	{
		template<Concepts::Vectorial T> T
		Normalized(const T& I_Vector) { return I_Vector.normalized(); }

		template<Concepts::Vectorial T> void
		Normalize(T* IO_Vector) { *IO_Vector = GetNormalized(IO_Vector); }

		template<Concepts::Vectorial T> Bool
		IsUnit(const T& I_Vector) { return IsEqual(1.0f, I_Vector.norm()); }

		template<Concepts::Vectorial T> Bool
		IsZero(const T& I_Vector) { return I_Vector.isZero(); }

		template<Concepts::Vectorial T> Bool
		IsIdentity(const T& I_Vector) { return I_Vector.isIdentity(); }

		//Even though this looks like it returns a new vector, Eigen is smart: if a is on both sides, Eigen avoids allocating a temporary and does it in-place behind the scenes (thanks to expression templates and lazy evaluation).
		template<Concepts::Vectorial T> T
		ComponentwiseMax(const T& I_VecA, const T& I_VecB) { return I_VecA.cwiseMax(I_VecB); }
		template<Concepts::Vectorial T> T
		ComponentwiseMin(const T& I_VecA, const T& I_VecB) { return I_VecA.cwiseMin(I_VecB); }
	}

}

template <>
struct fmt::formatter<Visera::FVector2F>
{
	// Parse format specifiers (if any)
	constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
	{
		return I_Context.begin();  // No custom formatting yet
	}

	// Corrected format function with const-correctness
	template <typename FormatContext>
	auto format(const Visera::FVector2F& I_Vec2F, FormatContext& I_Context) const
	-> decltype(I_Context.out())
	{
		return fmt::format_to(
			I_Context.out(),
			"[{}, {}]^T",
			I_Vec2F[0], I_Vec2F[1]
		);
	}
};

template <>
struct fmt::formatter<Visera::FVector3F>
{
	// Parse format specifiers (if any)
	constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
	{
		return I_Context.begin();  // No custom formatting yet
	}

	// Corrected format function with const-correctness
	template <typename FormatContext>
	auto format(const Visera::FVector3F& I_Vec3F, FormatContext& I_Context) const
	-> decltype(I_Context.out())
	{
		return fmt::format_to(
			I_Context.out(),
			"[{}, {}, {}]^T",
			I_Vec3F[0], I_Vec3F[1], I_Vec3F[2]
		);
	}
};

template <>
struct fmt::formatter<Visera::FVector4F>
{
	// Parse format specifiers (if any)
	constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
	{
		return I_Context.begin();  // No custom formatting yet
	}

	// Corrected format function with const-correctness
	template <typename FormatContext>
	auto format(const Visera::FVector4F& I_Vec4F, FormatContext& I_Context) const
	-> decltype(I_Context.out())
	{
		return fmt::format_to(
			I_Context.out(),
			"[{}, {}, {}, {}]^T",
			I_Vec4F[0], I_Vec4F[1], I_Vec4F[2], I_Vec4F[3]
		);
	}
};