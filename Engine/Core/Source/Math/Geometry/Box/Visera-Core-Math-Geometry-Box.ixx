module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Geometry.Box;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Math.Algebra.Vector;

export namespace Visera
{
    struct VISERA_CORE_API FBox2F
    {
        FVector2F Min {0.0f, 0.0f};
        FVector2F Max {0.0f, 0.0f};
        [[nodiscard]] constexpr FVector2F
        Center() const noexcept { return (Min + Max) / 2.0f; }
        [[nodiscard]] constexpr Float
        Area() const noexcept { return (Max.X - Min.X) * (Max.Y - Min.Y); }
    };
}