module;
#include <Visera-Runtime.hpp>
#define VK_NO_PROTOTYPES
#include <volk.h>
#include <vulkan/vulkan_raii.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
export module Visera.Runtime.RHI.Vulkan.Allocator;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanAllocator
    {
    public:
        [[nodiscard]] inline VmaAllocator
        GetHandle() const { return Handle; }

    private:
        VmaAllocator	        Handle{ nullptr };

    public:
        FVulkanAllocator() = delete;
        FVulkanAllocator(UInt32                          I_APIVersion,
                         const vk::raii::Instance&       I_Instance,
                         const vk::raii::PhysicalDevice& I_GPU,
                         const vk::raii::Device&         I_Device);
        ~FVulkanAllocator() = default;
    };

    FVulkanAllocator::
    FVulkanAllocator(UInt32                          I_APIVersion,
                     const vk::raii::Instance&       I_Instance,
                     const vk::raii::PhysicalDevice& I_GPU,
                     const vk::raii::Device&         I_Device)
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
            .physicalDevice     = *I_GPU,
            .device             = *I_Device,
            .pVulkanFunctions   = &VulkanFunctions,
            .instance           = *I_Instance,
            .vulkanApiVersion   = I_APIVersion,
        };
        if (vmaCreateAllocator(&CreateInfo, &Handle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create the Vulkan Memory Allocator!"); }

        LOG_TRACE("Created a Vulkan Memory Allocator.");
    }

    export inline VISERA_RUNTIME_API TUniquePtr<FVulkanAllocator>
    GVulkanAllocator;

    export class VISERA_RUNTIME_API IVulkanResource
    {
    public:
        enum class EType
        {
            Unknown,
            Image,
            Buffer,
        };

		[[nodiscard]] static inline VmaAllocator
        GetAllocator() { return GVulkanAllocator->GetHandle(); }
        [[nodiscard]] inline VmaAllocation&
        GetAllocation() { return Allocation; }
        [[nodiscard]] inline const VmaAllocation&
        GetAllocation() const { return Allocation; }
		[[nodiscard]] inline VkDeviceSize
        GetMemorySize() const { return Allocation->GetSize(); }
        [[nodiscard]] inline EType
        GetType() const { return Type; }

		[[nodiscard]] inline Bool
        IsEmpty() const { return GetMemorySize() == 0; }

    private:
        EType         Type{ EType::Unknown };
        VmaAllocation Allocation { nullptr };

    public:
        IVulkanResource() = delete;
        IVulkanResource(EType  I_Type) : Type{ I_Type } {}
    };
}
