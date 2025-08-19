module;
#include <Visera-Core.hpp>
#include <Eigen/Dense>
export module Visera.Core.Math.Algebra.Vector;
#define VISERA_MODULE_NAME "Core.Math.Algebra.Vector"

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