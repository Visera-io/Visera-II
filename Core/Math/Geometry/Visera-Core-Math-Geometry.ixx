module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Geometry;
#define VISERAII_MODULEII_NAME "Core.Math.Geometry"
import Visera.Core.Math.Algebra.Vector;

export namespace Visera
{
    namespace Math
    {
        Float inline
        Area(const FVector3F& I_Pa, const FVector3F& I_Pb, const FVector3F& I_Pc)
        {
            // A_triangle = 1/2 * Cross(Vab, Vac) = sqrt( ||Vab||^2 * ||Vac||^2 - (Dot(Vab, Vac)^2) )
            FVector3F Vab = I_Pb - I_Pa;
            FVector3F Vac = I_Pc - I_Pa;
            Float DotVabVac = Vab.dot(Vac);
            Float SqrArea = Vab.squaredNorm() * Vac.squaredNorm() - (DotVabVac * DotVabVac);
            return 0.5f * std::sqrt(SqrArea);
        }
    }
}