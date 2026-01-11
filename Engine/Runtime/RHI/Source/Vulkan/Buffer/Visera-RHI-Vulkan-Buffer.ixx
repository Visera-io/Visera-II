module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Buffer;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Allocator;
import Visera.Global.Log;
import Visera.Core.OS.Memory;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanBuffer : public IVulkanResource
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
        vk::BufferCreateInfo Info;

    public:
        FVulkanBuffer() : IVulkanResource{nullptr, EType::Buffer} {}
        FVulkanBuffer(TUniqueRef<FVulkanAllocator> I_Allocator,
                      const vk::BufferCreateInfo&  I_CreateInfo,
                      EVMAMemoryProperty           I_MemoryProperties);
        ~FVulkanBuffer() override;
        FVulkanBuffer(const FVulkanBuffer&)            = delete;
        FVulkanBuffer& operator=(const FVulkanBuffer&) = delete;
        FVulkanBuffer(FVulkanBuffer&& I_Other) noexcept
        : IVulkanResource(std::move(I_Other)),
          Handle(std::exchange(I_Other.Handle, vk::Buffer{}))
        { }

        FVulkanBuffer& operator=(FVulkanBuffer&& I_Other) noexcept
        {
            if (this != &I_Other)
            {
                if (Handle != nullptr) { Release(&Handle); }
                IVulkanResource::operator=(std::move(I_Other));
                Handle = std::exchange(I_Other.Handle, vk::Buffer{});
            }
            return *this;
        }
    };
    
    FVulkanBuffer::
    FVulkanBuffer(TUniqueRef<FVulkanAllocator> I_Allocator,
                  const vk::BufferCreateInfo&  I_CreateInfo,
                  EVMAMemoryProperty           I_MemoryProperties)
    : IVulkanResource {I_Allocator.get(), EType::Buffer},
      Info            {I_CreateInfo}
    {
        Allocate(&Handle, &Info, nullptr, I_MemoryProperties);
    }

    FVulkanBuffer::
    ~FVulkanBuffer()
    {
        Release(&Handle);
    }

    void FVulkanBuffer::
    Write(const void* I_Data, UInt64 I_Size)
    {
        if(!I_Data || I_Size == 0) { return; }

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