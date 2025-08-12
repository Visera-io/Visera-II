module;
#include <Visera-Core.hpp>
#include <Eigen/Dense>
export module Visera.Core.Math;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera
{
	namespace Concepts
	{
		template<typename T> concept
		Arithmetic = std::is_arithmetic_v<T>;
	}

	namespace Math
	{
		template <Concepts::Arithmetic NumT, Concepts::Arithmetic ReT = NumT> ReT
		Sqrt(NumT I_Num) { return std::sqrt(I_Num); }

		template <Concepts::Arithmetic NumT> NumT
		Abs(NumT I_Num) { return std::abs(I_Num); }
		template <Concepts::Arithmetic NumT> NumT
		Max(NumT I_NumA, NumT I_NumB) { return std::max(I_NumA, I_NumB); }
		template <Concepts::Arithmetic NumT> NumT
		Min(NumT I_NumA, NumT I_NumB) { return std::min(I_NumA, I_NumB); }

		template<Concepts::Arithmetic T> constexpr auto
		Epsilon() { return std::numeric_limits<T>::epsilon(); }

		template<Concepts::Arithmetic T> constexpr Bool
		IsEqual(T I_NumA, T I_NumB) { return Abs(I_NumA - I_NumB) <= Epsilon<T>(); }

		template<Concepts::Arithmetic T> constexpr auto
		UpperBound() { return std::numeric_limits<T>::max(); }
		template<Concepts::Arithmetic T> constexpr auto
		LowerBound() { return std::numeric_limits<T>::min(); }

		template<Concepts::Arithmetic NumT> constexpr void
		Clamp(NumT* IO_Value, std::pair<NumT, NumT> I_Range)
		{
			if (I_Range.first  > *IO_Value) { *IO_Value = I_Range.first;  return; }
			if (I_Range.second < *IO_Value) { *IO_Value = I_Range.second; return; }
		}
	}

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

	using FMatrixNxNF = Eigen::MatrixX<Float>;
	using FMatrix2x2F = Eigen::Matrix2<Float>;
	using FMatrix3x3F = Eigen::Matrix3<Float>;
	using FMatrix4x4F = Eigen::Matrix4<Float>;

	namespace Concepts
	{
		template<typename T> concept
		Matrical = std::is_class_v<FMatrixNxNF> ||
				   std::is_class_v<FMatrix2x2F> ||
				   std::is_class_v<FMatrix3x3F> ||
				   std::is_class_v<FMatrix4x4F>;
	}

	namespace Math
	{
		FMatrix3x3F inline
		ComputeCofactorMatrix(const FMatrix3x3F& I_Matrix)
		{
			const Eigen::FullPivLU<FMatrix3x3F> LUDecomposition(I_Matrix);

			auto M_inverse = LUDecomposition.inverse();  // This avoids recalculating the inverse multiple times
			Float M_determinant = LUDecomposition.determinant();  // Efficient determinant computation

			return M_determinant * M_inverse.transpose();
		}
	}
}