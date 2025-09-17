module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Frame;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FFrame
    {
    public:
        explicit FFrame() {};

    private:
        TUniquePtr<IFence> Fence;
    };
}
