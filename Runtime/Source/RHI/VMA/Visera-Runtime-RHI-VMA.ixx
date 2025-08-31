module;
#include <Visera-Runtime.hpp>
#define VK_NO_PROTOTYPES
#include <volk.h>
#define VMA_IMPLEMENTATION
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <vk_mem_alloc.h>
#else
#include <vma/vk_mem_alloc.h>
#endif
export module Visera.Runtime.RHI.VMA;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;
import Visera.Runtime.RHI.Vulkan;

export namespace Visera
{
    class VISERA_RUNTIME_API FVulkanMemoryAllocator
    {
    public:
        FVulkanMemoryAllocator() = delete;
        FVulkanMemoryAllocator(const TUniquePtr<FVulkan>& I_Vulkan);
        ~FVulkanMemoryAllocator() = default;

    private:
        VmaAllocator	Handle{ nullptr };
    };

    FVulkanMemoryAllocator::
    FVulkanMemoryAllocator(const TUniquePtr<FVulkan>& I_Vulkan)
    {
        const VmaVulkanFunctions VulkanFunctions
        {
            .vkGetInstanceProcAddr               = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr                 = vkGetDeviceProcAddr,
            .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
            .vkGetDeviceImageMemoryRequirements  = vkGetDeviceImageMemoryRequirements
        };

        const VmaAllocatorCreateInfo CreateInfo
        {
            .physicalDevice     = *I_Vulkan->GPU.Handle,
            .device             = *I_Vulkan->Device.Handle,
            .pVulkanFunctions   = &VulkanFunctions,
            .instance           = *I_Vulkan->Instance,
            .vulkanApiVersion   = I_Vulkan->AppInfo.apiVersion,
        };
        if (vmaCreateAllocator(&CreateInfo, &Handle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create the Vulkan Memory Allocator!"); }
    }

}
