module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Frame;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface;
import Visera.Core.Log;
import Visera.Core.Math.Arithmetic;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FFrame
    {
    public:
        void inline
        Wait() const { VISERA_WIP; Fence->Wait(Math::UpperBound<UInt64>()); }

    private:
        TUniquePtr<IFence> Fence;

    public:
        explicit FFrame() {};
    };
}
