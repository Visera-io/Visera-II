module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Buffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Allocator;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanBuffer : IVulkanResource
    {
    public:
        [[nodiscard]] inline const vk::Buffer&
        GetHandle() const { return Handle; }

    protected:
        vk::Buffer           Handle {nullptr};

    public:
        FVulkanBuffer() : IVulkanResource{EType::Buffer} {}
        FVulkanBuffer(UInt64             I_Size,
                      EVulkanBufferUsage I_Usage,
                      EPoolFlag          I_PoolFlag = EPoolFlag::None);
        ~FVulkanBuffer() override;
        FVulkanBuffer(const FVulkanBuffer&)            = delete;
        FVulkanBuffer& operator=(const FVulkanBuffer&) = delete;
    };
    
    FVulkanBuffer::
    FVulkanBuffer(UInt64             I_Size,
                  EVulkanBufferUsage I_Usage,
                  EPoolFlag          I_PoolFlag /* = EPoolFlag::None */)
    : IVulkanResource {EType::Buffer}
    {
        auto CreateInfo = vk::BufferCreateInfo{}
            .setSize                    (I_Size)
            .setUsage                   (I_Usage)
            .setSharingMode             (EVulkanSharingMode::eExclusive)
            .setQueueFamilyIndexCount   (0)
            .setPQueueFamilyIndices     (nullptr)
        ;
        Allocate(&Handle, &CreateInfo, I_PoolFlag);
    }

    FVulkanBuffer::
    ~FVulkanBuffer()
    {
        Release(&Handle);
    }
}