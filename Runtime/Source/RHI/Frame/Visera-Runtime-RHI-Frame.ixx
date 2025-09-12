module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Frame;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Driver;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FFrame
    {
    public:
        explicit FFrame() = delete;
        explicit FFrame(const TUniquePtr<IDriver>& I_Driver);
    private:
        const TUniquePtr<IDriver>& Driver;
    };

    FFrame::
    FFrame(const TUniquePtr<IDriver>& I_Driver)
        :Driver{I_Driver}
    {
        VISERA_ASSERT(Driver != nullptr);
    }
}
