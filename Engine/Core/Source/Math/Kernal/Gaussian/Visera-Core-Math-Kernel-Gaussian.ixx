module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Kernel.Gaussian;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic.Operation;
import Visera.Core.Types.Array;

export namespace Visera::Math
{
    /**
     * Computes a normalized 1D Gaussian kernel.
     * Kernel size = 2 * I_Radius + 1; weights sum to 1.
     * @param I_Sigma Standard deviation (controls blur amount)
     * @param I_Radius Half-radius (kernel extent each side of center)
     * @return TArray<Float> of length 2*I_Radius+1, normalized so sum == 1
     */
    [[nodiscard]] TArray<Float>
    Gaussian1D(Float I_Sigma, UInt32 I_Radius);
}

namespace Visera::Math
{
    TArray<Float>
    Gaussian1D(Float I_Sigma, UInt32 I_Radius)
    {
        const UInt32 KernelSize = 2 * I_Radius + 1;
        TArray<Float> Kernel(KernelSize);

        if (I_Sigma <= 0.0f)
        {
            Kernel[I_Radius] = 1.0f;
            return Kernel;
        }

        const Float SigmaSq = I_Sigma * I_Sigma;
        const Float InvTwoSigmaSq = 1.0f / (2.0f * SigmaSq);
        Float Sum = 0.0f;

        for (UInt32 i = 0; i < KernelSize; ++i)
        {
            const Int32 Offset = static_cast<Int32>(i) - static_cast<Int32>(I_Radius);
            const Float X = static_cast<Float>(Offset);
            Kernel[i] = static_cast<Float>(Math::Exp(-X * X * InvTwoSigmaSq));
            Sum += Kernel[i];
        }

        const Float InvSum = 1.0f / Sum;
        for (UInt32 i = 0; i < KernelSize; ++i)
        {
            Kernel[i] *= InvSum;
        }

        return Kernel;
    }
}
