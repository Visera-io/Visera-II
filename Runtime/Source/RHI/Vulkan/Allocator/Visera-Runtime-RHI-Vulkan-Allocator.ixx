module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
export module Visera.Runtime.RHI.Vulkan.Allocator;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Log;

namespace Visera::RHI
{
    export enum EVulkanMemoryPoolFlag
    {
        //[TODO]: Define in Common
        None                           = 0,

        Mapped                         = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        HostAccessAllowTransferInstead = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
        HostAccessSequentialWrite      = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    };

    export class VISERA_RUNTIME_API FVulkanAllocator
    {
    public:
        [[nodiscard]] inline VmaAllocator
        GetHandle() const { return Handle; }
        [[nodiscard]] inline const vk::raii::Device&
        GetDevice() const { return Device; }

    private:
        VmaAllocator	        Handle{ nullptr };
        const vk::raii::Device& Device;

    public:
        FVulkanAllocator() = delete;
        FVulkanAllocator(UInt32                          I_APIVersion,
                         const vk::raii::Instance&       I_Instance,
                         const vk::raii::PhysicalDevice& I_GPU,
                         const vk::raii::Device&         I_Device);
        ~FVulkanAllocator();
    };

    FVulkanAllocator::
    FVulkanAllocator(UInt32                          I_APIVersion,
                     const vk::raii::Instance&       I_Instance,
                     const vk::raii::PhysicalDevice& I_GPU,
                     const vk::raii::Device&         I_Device)
    : Device {I_Device}
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
            .device             = *Device,
            .pVulkanFunctions   = &VulkanFunctions,
            .instance           = *I_Instance,
            .vulkanApiVersion   = I_APIVersion,
        };
        if (vmaCreateAllocator(&CreateInfo, &Handle) != VK_SUCCESS)
        { LOG_FATAL("Failed to create the Vulkan Memory Allocator!"); }

        LOG_TRACE("Created a Vulkan Memory Allocator.");
    }

    FVulkanAllocator::
    ~FVulkanAllocator()
    {
        vmaDestroyAllocator(Handle);
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
        GetResourceType() const { return Type; }

		[[nodiscard]] inline Bool
        IsEmpty() const { return GetMemorySize() == 0; }

    protected:
        inline void
        Allocate(void*       I_Handle,
                 const void* I_CreateInfo,
                 EPoolFlag   I_PoolFlags = EPoolFlag::None);
        inline void
        Release(void* I_Handle);

    private:
        EType         Type{ EType::Unknown };
        VmaAllocation Allocation { nullptr };

    public:
        IVulkanResource() = delete;
        IVulkanResource(EType  I_Type) : Type{ I_Type } {}
        virtual ~IVulkanResource() = default;
    };

    void IVulkanResource::
    Allocate(void*       I_Handle,
             const void* I_CreateInfo,
             EPoolFlag   I_PoolFlags /* = EPoolFlag::None*/)
    {
        VmaAllocationCreateInfo AllocationCreateInfo
        {
            .flags          = static_cast<VmaAllocationCreateFlags>(I_PoolFlags),
            .usage          = VMA_MEMORY_USAGE_AUTO,
            .requiredFlags  = 0x0,
            .preferredFlags = 0x0,
            .memoryTypeBits = 0x0,
            .pool           = nullptr,
            .pUserData      = nullptr,
            .priority       = 0.0, // It is used only when #VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT flag was used during creation of the #VmaAllocator object
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
        case EType::Buffer:
        {
            auto CreateInfo  = static_cast<const VkBufferCreateInfo*>(I_CreateInfo);
            auto BufferHandle = static_cast<VkBuffer*>(I_Handle);

            VkResult Result = vmaCreateBuffer(
                GVulkanAllocator->GetHandle(),
                CreateInfo,
                &AllocationCreateInfo,
                BufferHandle,
                &Allocation,
                nullptr);

            if (Result != VK_SUCCESS)
            { LOG_FATAL("Failed to allocate the Vulkan Buffer Memory (error:{})!", static_cast<Int32>(Result)); }
            break;
        }
        default:
            VISERA_UNIMPLEMENTED_API;
        }
    }

    void IVulkanResource::
    Release(void* I_Handle)
    {
        switch (Type)
        {
        case EType::Image:
        {
            auto ImageHandle = *static_cast<vk::Image*>(I_Handle);

            vmaDestroyImage(
                GVulkanAllocator->GetHandle(),
                ImageHandle,
                Allocation);
            break;
        }
        case EType::Buffer:
        {
            auto BufferHandle = *static_cast<vk::Buffer*>(I_Handle);

            vmaDestroyBuffer(
                GVulkanAllocator->GetHandle(),
                BufferHandle,
                Allocation);
            break;
        }
        default:
            VISERA_UNIMPLEMENTED_API;
        }
        Allocation = nullptr;
    }
}
