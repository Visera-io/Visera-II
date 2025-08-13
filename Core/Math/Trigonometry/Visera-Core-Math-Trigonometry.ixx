module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Trigonometry;
#define VISERAI_MODULEI_NAME "Core.Math.Trigonometry"
export import Visera.Core.Math.Trigonometry.Degree;
export import Visera.Core.Math.Trigonometry.Radian;

export namespace Visera
{
    using FDegree = TDegree<Float>;
    using FRadian = TRadian<Float>;

    namespace Math
    {
        FRadian constexpr
        DegreeToRadian(FDegree I_Degree) { return FRadian{Float((I_Degree / 180.0f) * FRadian::PI.GetValue()) }; }
        FDegree constexpr
        RadianToDegree(FRadian I_Radian) { return FDegree{Float((I_Radian / FRadian::PI.GetValue()) * 180.0f) }; }

        Float inline Tan(FRadian I_Radian)
        {
            return std::tan(static_cast<Float>(I_Radian));
        }

        Float inline Tan(FDegree I_Degree)
        {
            return Tan(DegreeToRadian(I_Degree));
        }

        FRadian inline ATan(Float I_Value)
        {
            return FRadian{std::atan(I_Value)};
        }

        FRadian inline ATan(Double I_Value)
        {
            return ATan(static_cast<Float>(I_Value));
        }

        Float inline Sin(FRadian I_Radian)
        {
            return std::sin(static_cast<Float>(I_Radian));
        }

        Float inline Sin(FDegree I_Degree)
        {
            return Sin(DegreeToRadian(I_Degree));
        }

        FRadian inline ASin(Float I_Value)
        {
            VISERA_ASSERT(I_Value >= -1.0f && I_Value <= 1.0f);
            return FRadian{ std::asin(I_Value) };
        }

        FRadian inline ASin(Double I_Value)
        {
            return ASin(static_cast<Float>(I_Value));
        }

        Float inline Cos(FRadian I_Radian)
        {
            return std::cos(static_cast<Float>(I_Radian));
        }

        Float inline Cos(FDegree I_Degree)
        {
            return Cos(DegreeToRadian(I_Degree));
        }

        FRadian inline ACos(Float I_Value)
        {
            VISERA_ASSERT(I_Value >= -1.0f && I_Value <= 1.0f);
            return FRadian{ std::acos(I_Value) };
        }

        FRadian inline ACos(Double I_Value)
        {
            return FRadian{ ACos(static_cast<Float>(I_Value)) };
        }
    }
}