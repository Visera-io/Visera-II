module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Driver;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Driver.Interface;
       import Visera.Runtime.RHI.Driver.Vulkan;

namespace Visera::RHI
{
    export inline VISERA_RUNTIME_API TUniquePtr<IDriver>
    GDriver = MakeUnique<FVulkan>();
}
