module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics.Scene.Light;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Core.Math.Algebra;
import Visera.Core.Math.Color.Linear;

export namespace Visera
{
    struct VISERA_RUNTIME_API FLight
    {
        enum class EType : UInt8 { Directional, Point, Spot };

        EType        Type        {EType::Directional};
        FVector3F    Position    {0.f, 0.f, 0.f};
        FVector3F    Direction   {0.f, -1.f, 0.f};
        FLinearColor Color       {1.f, 1.f, 1.f, 1.f};
        Float        Intensity   {1.0f};
        Float        Range       {10.0f};
        Float        SpotAngle   {45.0f};
        Bool         bCastShadow {False};
    };
}
