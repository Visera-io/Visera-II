module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.VMA;
#define VISERA_MODULE_NAME "Runtime.RHI"

export namespace Visera
{
    class VISERA_RUNTIME_API FVulkanMemoryAllocator
    {
    public:
        FVulkanMemoryAllocator();
        ~FVulkanMemoryAllocator();
    };
}
