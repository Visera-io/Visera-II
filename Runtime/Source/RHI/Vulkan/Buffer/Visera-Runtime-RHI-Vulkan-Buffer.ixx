module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Buffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Core.Log;
import Visera.Core.OS.Memory;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanBuffer : public IVulkanResource
    {
    public:
        [[nodiscard]] inline const vk::Buffer&
        GetHandle() const { return Handle; }
        void
        Write(const void* I_Data, UInt64 I_Size);
        template<class T> void
        Write(const T& I_Data) { Write(&I_Data, sizeof(T)); }

        [[nodiscard]] inline Bool
        IsMapped() const { return GetAllocation()->GetMappedData() != nullptr; }

    protected:
        vk::Buffer           Handle {nullptr};

    public:
        FVulkanBuffer() : IVulkanResource{EType::Buffer} {}
        FVulkanBuffer(UInt64                 I_Size,
                      vk::BufferUsageFlags   I_Usages,
                      EVMAMemoryPoolFlags    I_MemoryPoolFlags = EVMAMemoryPoolFlags::None);
        ~FVulkanBuffer() override;
        FVulkanBuffer(const FVulkanBuffer&)            = delete;
        FVulkanBuffer& operator=(const FVulkanBuffer&) = delete;
    };
    
    FVulkanBuffer::
    FVulkanBuffer(UInt64                I_Size,
                  vk::BufferUsageFlags  I_Usages,
                  EVMAMemoryPoolFlags   I_MemoryPoolFlags /* = EVulkanMemoryPoolFlags::None */)
    : IVulkanResource {EType::Buffer}
    {
        auto CreateInfo = vk::BufferCreateInfo{}
            .setSize                    (I_Size)
            .setUsage                   (I_Usages)
            .setSharingMode             (vk::SharingMode::eExclusive)
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
        void* MappedMemory = GetAllocation()->GetMappedData();

        if (!MappedMemory)
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