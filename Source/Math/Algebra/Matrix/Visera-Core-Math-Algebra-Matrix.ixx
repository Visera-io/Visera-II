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

template <>
struct fmt::formatter<Visera::FMatrix2x2F>
{
	// Parse format specifiers (if any)
	constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
	{
		return I_Context.begin();  // No custom formatting yet
	}

	// Corrected format function with const-correctness
	template <typename FormatContext>
	auto format(const Visera::FMatrix2x2F& I_Mat2x2F, FormatContext& I_Context) const
	-> decltype(I_Context.out())
	{
		return fmt::format_to(
		I_Context.out(),
		"\n"
			"|   {:<10.6f}, {:<10.6f} |\n"
			"|   {:<10.6f}, {:<10.6f} |_Matrix2x2F",
			I_Mat2x2F(0, 0), I_Mat2x2F(0, 1),
			I_Mat2x2F(1, 0), I_Mat2x2F(1, 1)
		);
	}
};

template <>
struct fmt::formatter<Visera::FMatrix3x3F>
{
	// Parse format specifiers (if any)
	constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
	{
		return I_Context.begin();  // No custom formatting yet
	}

	// Corrected format function with const-correctness
	template <typename FormatContext>
	auto format(const Visera::FMatrix3x3F& I_Mat3x3F, FormatContext& I_Context) const
	-> decltype(I_Context.out())
	{
		return fmt::format_to(
		I_Context.out(),
		"\n"
			"|   {:<10.6f}, {:<10.6f}, {:<10.6f} |\n"
			"|   {:<10.6f}, {:<10.6f}, {:<10.6f} |\n"
			"|   {:<10.6f}, {:<10.6f}, {:<10.6f} |_Matrix3x3F",
			I_Mat3x3F(0, 0), I_Mat3x3F(0, 1), I_Mat3x3F(0, 2),
			I_Mat3x3F(1, 0), I_Mat3x3F(1, 1), I_Mat3x3F(1, 2),
			I_Mat3x3F(2, 0), I_Mat3x3F(2, 1), I_Mat3x3F(2, 2)
		);
	}
};

template <>
struct fmt::formatter<Visera::FMatrix4x4F>
{
	// Parse format specifiers (if any)
	constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
	{
		return I_Context.begin();  // No custom formatting yet
	}

	// Corrected format function with const-correctness
	template <typename FormatContext>
	auto format(const Visera::FMatrix4x4F& I_Mat4x4F, FormatContext& I_Context) const
	-> decltype(I_Context.out())
	{
		return fmt::format_to(
			I_Context.out(),
			"\n"
			"|   {:<10.6f}, {:<10.6f}, {:<10.6f}, {:<10.6f} |\n"
			"|   {:<10.6f}, {:<10.6f}, {:<10.6f}, {:<10.6f} |\n"
			"|   {:<10.6f}, {:<10.6f}, {:<10.6f}, {:<10.6f} |\n"
			"|   {:<10.6f}, {:<10.6f}, {:<10.6f}, {:<10.6f} |_Matrix4x4F",
			I_Mat4x4F(0, 0), I_Mat4x4F(0, 1), I_Mat4x4F(0, 2), I_Mat4x4F(0, 3),
			I_Mat4x4F(1, 0), I_Mat4x4F(1, 1), I_Mat4x4F(1, 2), I_Mat4x4F(1, 3),
			I_Mat4x4F(2, 0), I_Mat4x4F(2, 1), I_Mat4x4F(2, 2), I_Mat4x4F(2, 3),
			I_Mat4x4F(3, 0), I_Mat4x4F(3, 1), I_Mat4x4F(3, 2), I_Mat4x4F(3, 3)
		);
	}
};