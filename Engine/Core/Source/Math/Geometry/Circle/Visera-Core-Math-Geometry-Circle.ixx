module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Geometry.Circle;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Constants;

export namespace Visera
{
    struct VISERA_CORE_API FCircle2F
    {
        FVector2F Center {0.0f, 0.0f};
        Float     Radius {0.0f};
        [[nodiscard]] constexpr Float
        Area() const noexcept { return Math::PI * Radius * Radius; }
    };
}