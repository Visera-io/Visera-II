module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Geometry.Area;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Arithmetic;

export namespace Visera
{
    namespace Math
    {
        [[nodiscard]] Float inline
        Area(const FVector3F& I_Pa, const FVector3F& I_Pb, const FVector3F& I_Pc)
        {
            // A_triangle = 1/2 * Cross(Vab, Vac) = sqrt( ||Vab||^2 * ||Vac||^2 - (Dot(Vab, Vac)^2) )
            auto Vab = I_Pb - I_Pa;
            auto Vac = I_Pc - I_Pa;
            Float DotVabVac = Vab.Dot(Vac);
            Float SqrArea = Vab.SquaredNorm() * Vac.SquaredNorm() - (DotVabVac * DotVabVac);
            return 0.5f * Sqrt(SqrArea);
        }
    }
}