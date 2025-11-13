module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
#include <vk_mem_alloc.h>
export module Visera.Runtime.RHI.Vulkan.Buffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Core.Log;
import Visera.Core.OS.Memory;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanBuffer : public IVulkanResource
    {
    public:
        [[nodiscard]] inline const vk::Buffer&
        GetHandle() const { return Handle; }
        inline void
        Write(const void* I_Data, UInt64 I_Size);

        [[nodiscard]] inline Bool
        IsMapped() const { return GetAllocationInfo().pMappedData != nullptr; }

    protected:
        vk::Buffer           Handle {nullptr};

    public:
        FVulkanBuffer() : IVulkanResource{EType::Buffer} {}
        FVulkanBuffer(UInt64                I_Size,
                      EVulkanBufferUsage    I_Usage,
                      EVulkanMemoryPoolFlags I_MemoryPoolFlags = EVulkanMemoryPoolFlagBits::eNone);
        ~FVulkanBuffer() override;
        FVulkanBuffer(const FVulkanBuffer&)            = delete;
        FVulkanBuffer& operator=(const FVulkanBuffer&) = delete;
    };
    
    FVulkanBuffer::
    FVulkanBuffer(UInt64                I_Size,
                  EVulkanBufferUsage    I_Usage,
                  EVulkanMemoryPoolFlags I_MemoryPoolFlags /* = EVulkanMemoryPoolFlagBits::eNone */)
    : IVulkanResource {EType::Buffer}
    {
        auto CreateInfo = vk::BufferCreateInfo{}
            .setSize                    (I_Size)
            .setUsage                   (I_Usage)
            .setSharingMode             (EVulkanSharingMode::eExclusive)
            .setQueueFamilyIndexCount   (0)
            .setPQueueFamilyIndices     (nullptr)
        ;
        Allocate(&Handle, &CreateInfo, I_MemoryPoolFlags);
    }

    FVulkanBuffer::
    ~FVulkanBuffer()
    {
        Release(&Handle);
    }

    void FVulkanBuffer::
    Write(const void* I_Data, UInt64 I_Size)
    {
        VISERA_ASSERT(I_Data && I_Size);
        VISERA_ASSERT(I_Size <= GetMemorySize());
        void* MappedMemory = nullptr;

        if (!IsMapped())
        {
            MapMemory(&MappedMemory);
            Memory::Memcpy(MappedMemory, I_Data, I_Size);
            UnmapMemory();
        }
        else
        {
            Memory::Memcpy(MappedMemory, I_Data, I_Size);
        }
    }
}