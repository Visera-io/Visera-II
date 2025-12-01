module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry;
#define VISERA_MODULE_NAME "Core.Math.Trigonometry"
export import Visera.Core.Math.Trigonometry.Degree;
export import Visera.Core.Math.Trigonometry.Radian;
import Visera.Core.Math.Constants;

export namespace Visera
{
    namespace Math
    {
        [[nodiscard]] constexpr FRadian
        Radian(FDegree I_Degree) noexcept { return FRadian{ Float((I_Degree.Value / 180.0f) * Math::PI) }; }
        [[nodiscard]] constexpr FDegree
        Degree(FRadian I_Radian) noexcept { return FDegree{ Float((I_Radian.Value / Math::PI) * 180.0f) }; }
        [[nodiscard]] inline Float
        Tan(FRadian I_Radian) noexcept { return std::tan(I_Radian.Value); }
        [[nodiscard]] inline Float
        Tan(FDegree I_Degree) noexcept { return Tan(Radian(I_Degree)); }
        [[nodiscard]] inline FRadian
        ATan(Float I_Value) noexcept { return FRadian{ std::atan(I_Value) }; }
        [[nodiscard]] inline FRadian
        ATan(Double I_Value) noexcept { return ATan(static_cast<Float>(I_Value)); }
        [[nodiscard]] inline Float
        Sin(FRadian I_Radian) noexcept { return std::sin(I_Radian.Value); }
        [[nodiscard]] inline Float
        Sin(FDegree I_Degree) noexcept { return Sin(Radian(I_Degree)); }
        [[nodiscard]] inline FRadian
        ASin(Float I_Value) noexcept { VISERA_ASSERT(I_Value >= -1.0f && I_Value <= 1.0f); return FRadian{ std::asin(I_Value) }; }
        [[nodiscard]] inline FRadian
        ASin(Double I_Value) noexcept { return ASin(static_cast<Float>(I_Value)); }
        [[nodiscard]] inline Float
        Cos(FRadian I_Radian) noexcept { return std::cos(I_Radian.Value); }
        [[nodiscard]] inline Float
        Cos(FDegree I_Degree) noexcept { return Cos(Radian(I_Degree)); }
        [[nodiscard]] inline FRadian
        ACos(Float I_Value) noexcept { VISERA_ASSERT(I_Value >= -1.0f && I_Value <= 1.0f); return FRadian{ std::acos(I_Value) }; }
        [[nodiscard]] inline FRadian
        ACos(Double I_Value) noexcept { return ACos(static_cast<Float>(I_Value)); }
    }
}
