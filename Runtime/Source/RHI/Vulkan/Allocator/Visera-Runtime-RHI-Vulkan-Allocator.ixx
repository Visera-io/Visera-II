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
		[[nodiscard]] inline VkDeviceSize
        GetMemorySize() const { return Allocation->GetSize(); }
        [[nodiscard]] inline EType
        GetType() const { return Type; }

		[[nodiscard]] inline Bool
        IsEmpty() const { return GetMemorySize() == 0; }

    protected:
        inline void
        Allocate(void* I_Handle, const void* I_CreateInfo);

    private:
        EType         Type{ EType::Unknown };
        VmaAllocation Allocation { nullptr };

    public:
        IVulkanResource() = delete;
        IVulkanResource(EType  I_Type) : Type{ I_Type } {}
    };

    void IVulkanResource::
    Allocate(void* I_Handle, const void* I_CreateInfo)
    {
        VmaAllocationCreateInfo AllocationCreateInfo
        {
            .flags = 0x0,
            .usage = VMA_MEMORY_USAGE_AUTO,
        };

        switch (Type)
        {
        case EType::Image:
        {
            auto CreateInfo  = static_cast<const VkImageCreateInfo*>(I_CreateInfo);
            auto ImageHandle = static_cast<VkImage*>(I_Handle);

            VkResult Result = vmaCreateImage(
                GVulkanAllocator->GetHandle(),
                CreateInfo,
                &AllocationCreateInfo,
                ImageHandle,
                &Allocation,
                nullptr);

            if (Result != VK_SUCCESS)
            { LOG_FATAL("Failed to allocate the Vulkan Image Memory (error:{})!", static_cast<Int32>(Result)); }
            break;
        }
        default:
            VISERA_WIP;
        }
    }
}
