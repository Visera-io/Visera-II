module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Constants;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera::Math
{
	constexpr Double PI = 3.14159265358979323846264338327950288; // PI
	constexpr Double E  = 2.71828182845904523536028747135266250; // e

	constexpr Double InvPI     = 0.31830988618379067154; // 1.0 / PI
	constexpr Double HalfPI    = 1.57079632679489661923; // PI / 2.0
	constexpr Double TwoPI     = 6.28318530717958647692; // PI * 2.0
	constexpr Double PISquared = 9.86960440108935861883; // PI * PI
	
	constexpr Double Ln2       = 0.69314718055994530941723212145818; // ln(2)

	constexpr Double Sqrt2     = 1.4142135623730950488016887242097;  // sqrt(2)
	constexpr Double Sqrt3     = 1.7320508075688772935274463415059;  // sqrt(3)
	constexpr Double InvSqrt2  = 0.70710678118654752440084436210485; // 1.0 / sqrt(2)
	constexpr Double InvSqrt3  = 0.57735026918962576450914878050196; // 1.0 / sqrt(3)
	constexpr Double HalfSqrt2 = 0.70710678118654752440084436210485; // sqrt(2) / 2.0
	constexpr Double HalfSqrt3 = 0.86602540378443864676372317075294; // sqrt(3) / 2.0
}