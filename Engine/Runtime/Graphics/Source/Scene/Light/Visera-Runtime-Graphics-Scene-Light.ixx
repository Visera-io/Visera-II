module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Scene.Light;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Core.Math.Algebra;
import Visera.Core.Math.Color.Linear;

export namespace Visera
{
    struct VISERA_RUNTIME_API FLight
    {
        FVector3F    Direction {0.f, -1.f, 0.f};
        FLinearColor Color     {1.f, 1.f, 1.f, 1.f};
        Float        Intensity {1.0f};
    };
}
