module;
#include <Visera-Runtime.hpp>
#define VK_NO_PROTOTYPES
#include <volk.h>
#include <vulkan/vulkan_raii.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
export module Visera.Runtime.RHI.Driver.Vulkan.Allocator;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanAllocator
    {
    public:
        FVulkanAllocator() = delete;
        FVulkanAllocator(UInt32             I_APIVersion,
                         vk::Instance       I_Instance,
                         vk::PhysicalDevice I_GPU,
                         vk::Device         I_Device);
        ~FVulkanAllocator() = default;

    private:
        VmaAllocator	Handle{ nullptr };
    };

    FVulkanAllocator::
    FVulkanAllocator(UInt32             I_APIVersion,
                     vk::Instance       I_Instance,
                     vk::PhysicalDevice I_GPU,
                     vk::Device         I_Device)
    {
        LOG_DEBUG("Creating a Vulkan Allocator.");

        const VmaVulkanFunctions VulkanFunctions
        {
            .vkGetInstanceProcAddr               = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr                 = vkGetDeviceProcAddr,
            .vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements,
            .vkGetDeviceImageMemoryRequirements  = vkGetDeviceImageMemoryRequirements
        };

        const VmaAllocatorCreateInfo CreateInfo
        {
            .physicalDevice     = I_GPU,
            .device             = I_Device,
            .pVulkanFunctions   = &VulkanFunctions,
            .instance           = I_Instance,
            .vulkanApiVersion   = I_APIVersion,
        };
        if (vmaCreateAllocator(&CreateInfo, &Handle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create the Vulkan Memory Allocator!"); }
    }

}
