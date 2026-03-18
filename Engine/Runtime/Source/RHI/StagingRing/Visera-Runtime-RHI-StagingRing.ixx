module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.StagingRing;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Buffer;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Runtime.RHI.Vulkan;
import Visera.Core.OS.Memory;
import Visera.Core.Log;
import vulkan_hpp;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHIStagingRingBuffer
    {
    public:
        struct FAllocation
        {
            UInt64 Offset {0};
            UInt64 Size   {0};

            [[nodiscard]] Bool IsValid() const { return Size > 0; }
        };

        [[nodiscard]] FAllocation
        Allocate(UInt64 I_Size, UInt64 I_Alignment = 256);

        void
        Write(const FAllocation& I_Alloc, const void* I_Data, UInt64 I_Size);

        void
        AdvanceFence(UInt64 I_CompletedOffset);

        [[nodiscard]] UInt64
        GetCapacity() const { return Capacity; }
        [[nodiscard]] UInt64
        GetFreeSpace() const;
        [[nodiscard]] FVulkanBuffer*
        GetVulkanBuffer() { return &Buffer; }

    private:
        FVulkanBuffer Buffer;
        FByte*        MappedPtr    {nullptr};
        UInt64        Capacity     {0};
        UInt64        WriteOffset  {0};
        UInt64        FenceOffset  {0};

    public:
        FRHIStagingRingBuffer() = default;
        FRHIStagingRingBuffer(FVulkanDriver* I_Driver, UInt64 I_CapacityBytes);
        ~FRHIStagingRingBuffer() = default;

        FRHIStagingRingBuffer(const FRHIStagingRingBuffer&) = delete;
        FRHIStagingRingBuffer& operator=(const FRHIStagingRingBuffer&) = delete;
        FRHIStagingRingBuffer(FRHIStagingRingBuffer&&) = default;
        FRHIStagingRingBuffer& operator=(FRHIStagingRingBuffer&&) = default;
    };

    FRHIStagingRingBuffer::
    FRHIStagingRingBuffer(FVulkanDriver* I_Driver, UInt64 I_CapacityBytes)
    : Capacity{I_CapacityBytes}
    {
        VISERA_ASSERT(I_Driver && I_CapacityBytes > 0);

        auto BufferCreateInfo = vk::BufferCreateInfo{}
            .setSize        (I_CapacityBytes)
            .setUsage       (vk::BufferUsageFlagBits::eTransferSrc)
            .setSharingMode (vk::SharingMode::eExclusive);

        Buffer = I_Driver->CreateBuffer(
            BufferCreateInfo,
            EVMAMemoryProperty::Mapped |
            EVMAMemoryProperty::HostAccessSequentialWrite |
            EVMAMemoryProperty::HostAccessAllowTransferInstead);

        MappedPtr = static_cast<FByte*>(Buffer.GetMappedPtr());
        VISERA_ASSERT(MappedPtr);

        LOG_DEBUG("StagingRingBuffer created: {} bytes, persistently mapped.", I_CapacityBytes);
    }

    FRHIStagingRingBuffer::FAllocation FRHIStagingRingBuffer::
    Allocate(UInt64 I_Size, UInt64 I_Alignment)
    {
        if (I_Size == 0 || I_Size > Capacity) { return {}; }

        const UInt64 AlignedOffset = Memory::Align(WriteOffset, I_Alignment);
        UInt64 End = AlignedOffset + I_Size;

        if (End <= Capacity)
        {
            WriteOffset = End;
            return FAllocation{ .Offset = AlignedOffset, .Size = I_Size };
        }

        // Wrap around: try from the beginning
        const UInt64 WrappedEnd = I_Size;
        if (WrappedEnd <= FenceOffset)
        {
            WriteOffset = WrappedEnd;
            return FAllocation{ .Offset = 0, .Size = I_Size };
        }

        LOG_ERROR("StagingRingBuffer exhausted: requested={} bytes, capacity={}, write={}, fence={}.",
            I_Size, Capacity, WriteOffset, FenceOffset);
        return {};
    }

    void FRHIStagingRingBuffer::
    Write(const FAllocation& I_Alloc, const void* I_Data, UInt64 I_Size)
    {
        VISERA_ASSERT(I_Alloc.IsValid() && I_Data && I_Size <= I_Alloc.Size);
        VISERA_ASSERT(I_Alloc.Offset + I_Size <= Capacity);
        Memory::Memcpy(MappedPtr + I_Alloc.Offset, I_Data, I_Size);
    }

    void FRHIStagingRingBuffer::
    AdvanceFence(UInt64 I_CompletedOffset)
    {
        // Never decrease: after wrap-around, multiple regions can complete out of order;
        // we must retain the furthest completed offset so all freed space remains reusable.
        if (I_CompletedOffset > FenceOffset)
            FenceOffset = I_CompletedOffset;
    }

    UInt64 FRHIStagingRingBuffer::
    GetFreeSpace() const
    {
        if (WriteOffset >= FenceOffset)
        { return Capacity - WriteOffset + FenceOffset; }
        return FenceOffset - WriteOffset;
    }
}
