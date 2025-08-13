module;
#include <Visera-Core.hpp>
#include <Eigen/Dense>
export module Visera.Core.Math.Algebra.Matrix;
#define VISERA_MODULE_NAME "Core.Math.Algebra.Matrix"

export namespace Visera
{
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