module;
#include <Visera-RHI.hpp>
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
export module Visera.RHI.Vulkan.Allocator;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.Core.Traits.Flags;
import Visera.Global.Log;
import vulkan_hpp;

export namespace Visera
{
    enum EVMAMemoryProperty : UInt32
    {
        None                           = 0,
        Mapped                         = VMA_ALLOCATION_CREATE_MAPPED_BIT,
        HostAccessAllowTransferInstead = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
        HostAccessSequentialWrite      = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        Aliasable                      = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT,
    };
    VISERA_MAKE_FLAGS(EVMAMemoryProperty);

    class VISERA_RHI_API FVulkanAllocator
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
            .vkGetInstanceProcAddr                  = vkGetInstanceProcAddr,
            .vkGetDeviceProcAddr                    = vkGetDeviceProcAddr,
            // I_Other functions can be null (@ref: vk_mem_alloc.h line 2927)
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
    }

    FVulkanAllocator::
    ~FVulkanAllocator()
    {
        vmaDestroyAllocator(Handle);
    }

    class VISERA_RHI_API IVulkanResource
    {
    public:
        enum class EType : UInt8
        {
            Unknown,
            Image,
            Buffer,
            SwapChainImage, // Special Image
        };
		[[nodiscard]] inline VkDeviceSize
        GetMemorySize() const { return Allocation->GetSize(); }
        [[nodiscard]] inline EType
        GetResourceType() const { return Type; }
        [[nodiscard]] inline const FVulkanAllocator*
        GetAllocator() const { return Allocator; }

        [[nodiscard]] inline Bool
        IsAllocated() const { return Allocation != nullptr; }
		[[nodiscard]] inline Bool
        IsEmpty() const { return GetMemorySize() == 0; }
        [[nodiscard]] inline Bool
        IsAliased() const { return !bOwnsAllocation; }

    protected:
        inline void
        Allocate(void*                  I_Handle,
                 const void*            I_CreateInfo,
                 const IVulkanResource* I_Owner,
                 EVMAMemoryProperty     I_MemoryProperties);
        inline void
        Release(void* I_Handle);
        [[nodiscard]] inline const VmaAllocation&
        GetAllocation() const { return Allocation; }
        [[nodiscard]] inline VmaAllocation&
        GetAllocation() { return Allocation; }
        [[nodiscard]] VmaAllocationInfo
        GetAllocationInfo() const;
        void
        MapMemory(void** I_UnmappedMemory);
        void
        UnmapMemory();

    private:
        EType             Type              { EType::Unknown };
        FVulkanAllocator* Allocator         { nullptr };
        VmaAllocation     Allocation        { nullptr };
        Bool              bOwnsAllocation   { True    };

    public:
        IVulkanResource() = delete;
        IVulkanResource(FVulkanAllocator* I_Allocator, EType I_Type)
        : Allocator {I_Allocator}, Type{ I_Type } {}
        virtual ~IVulkanResource() = default;
        IVulkanResource(const IVulkanResource&) = delete;
        IVulkanResource& operator=(const IVulkanResource&) = delete;

        IVulkanResource(IVulkanResource&& I_Other) noexcept
        : Type              (I_Other.Type)
        , Allocator         (I_Other.Allocator)
        , Allocation        (I_Other.Allocation)
        , bOwnsAllocation   (I_Other.bOwnsAllocation)
        {
            I_Other.Allocation      = nullptr;
            I_Other.bOwnsAllocation = False;
        }

        IVulkanResource& operator=(IVulkanResource&& I_Other) noexcept
        {
            if (this != &I_Other)
            {
                Type             = I_Other.Type;
                Allocator        = I_Other.Allocator;
                Allocation       = I_Other.Allocation;
                bOwnsAllocation  = I_Other.bOwnsAllocation;

                I_Other.Allocation      = nullptr;
                I_Other.bOwnsAllocation = False;
            }
            return *this;
        }
    };

    void IVulkanResource::
    Allocate(void*                  I_Handle,
             const void*            I_CreateInfo,
             const IVulkanResource* I_Owner,
             EVMAMemoryProperty     I_MemoryProperties)
    {
        if (I_Owner)
        {
            bOwnsAllocation = False;
            Allocation      = I_Owner->GetAllocation();
        }

        VmaAllocationCreateInfo AllocationCreateInfo
        {
            .flags          = I_MemoryProperties,
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

            VkResult Result = VK_SUCCESS;
            if (IsAliased())
            {
                Result = vmaCreateAliasingImage(Allocator->GetHandle(),
                    I_Owner->GetAllocation(),
                    CreateInfo,
                    ImageHandle);
            }
            else
            {
                Result = vmaCreateImage(Allocator->GetHandle(),
                    CreateInfo,
                    &AllocationCreateInfo,
                    ImageHandle,
                    &Allocation,
                    nullptr);
            }
            if (Result != VK_SUCCESS)
            { LOG_FATAL("Failed to allocate the Vulkan Image Memory (error:{})!", static_cast<Int32>(Result)); }
            break;
        }
        case EType::Buffer:
        {
            auto CreateInfo  = static_cast<const VkBufferCreateInfo*>(I_CreateInfo);
            auto BufferHandle = static_cast<VkBuffer*>(I_Handle);

            VkResult Result = VK_SUCCESS;
            if (IsAliased())
            {
                Result = vmaCreateAliasingBuffer(Allocator->GetHandle(),
                    I_Owner->GetAllocation(),
                    CreateInfo,
                    BufferHandle);
            }
            else
            {
                Result = vmaCreateBuffer(Allocator->GetHandle(),
                    CreateInfo,
                    &AllocationCreateInfo,
                    BufferHandle,
                    &Allocation,
                    nullptr);
            }
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
        VISERA_ASSERT(I_Handle != nullptr);

        switch (Type)
        {
        case EType::Image:
        {
            auto ImageHandle = *static_cast<VkImage*>(I_Handle);
            if (!ImageHandle) { return; }

            if (!IsAliased())
            {
                vmaDestroyImage(Allocator->GetHandle(),
                                ImageHandle,
                                Allocation);
            }
            else
            {
                vkDestroyImage(*Allocator->GetDevice(),
                               ImageHandle, nullptr);
            }
            break;
        }
        case EType::Buffer:
        {
            auto BufferHandle = *static_cast<VkBuffer*>(I_Handle);
            if (!BufferHandle) { return; }

            if (!IsAliased())
            {
                vmaDestroyBuffer(Allocator->GetHandle(),
                                 BufferHandle,
                                 Allocation);
            }
            else
            {
                vkDestroyBuffer(*Allocator->GetDevice(),
                                BufferHandle, nullptr);
            }

            break;
        }
        case EType::SwapChainImage:
            // Do nothing
            break;
        default:
            VISERA_UNIMPLEMENTED_API;
        }
        Allocation = nullptr;
    }

    VmaAllocationInfo IVulkanResource::
    GetAllocationInfo() const
    {
        VISERA_ASSERT(IsAllocated());
        VmaAllocationInfo AllocationInfo{};
        vmaGetAllocationInfo(Allocator->GetHandle(), Allocation, &AllocationInfo);
        return AllocationInfo;
    }

    void IVulkanResource::
    MapMemory(void** I_UnmappedMemory)
    {
        vmaMapMemory(Allocator->GetHandle(), Allocation, I_UnmappedMemory);
    }

    void IVulkanResource::
    UnmapMemory()
    {
        vmaUnmapMemory(Allocator->GetHandle(), Allocation);
    }

}