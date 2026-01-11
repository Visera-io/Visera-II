module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.CommandPool;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.CommandBuffer;
import Visera.Global.Log;
import vulkan_hpp;

export namespace Visera
{
    template<EVulkanQueueFamily QueueFamily>
    class VISERA_RHI_API FVulkanCommandPool
    {
    public:
        [[nodiscard]] inline vk::CommandPool
        GetHandle() const { return Handle; }
        [[nodiscard]] inline EVulkanQueueFamily
        GetQueueFamily() const { return QueueFamily; }
        [[nodiscard]] FVulkanCommandBuffer<QueueFamily>
        CreateCommandBuffer(Bool I_bPrimary)
        {
            vk::CommandBufferAllocateInfo AllocInfo{};
            AllocInfo.commandPool = *Handle;
            AllocInfo.level = I_bPrimary ? vk::CommandBufferLevel::ePrimary : vk::CommandBufferLevel::eSecondary;
            AllocInfo.commandBufferCount = 1;

            return FVulkanCommandBuffer<QueueFamily>(Handle, AllocInfo);
        }

    private:
        vk::raii::CommandPool     Handle      {nullptr};
        vk::CommandPoolCreateInfo Info;

    public:
        FVulkanCommandPool() = default;
        FVulkanCommandPool(const vk::raii::Device&          I_Device,
                           const vk::CommandPoolCreateInfo& I_CreateInfo)
        : Info(I_CreateInfo)
        {
            auto Result = I_Device.createCommandPool(Info);
            if (Result.has_value())
            { Handle = std::move(*Result); }
            else
            { LOG_FATAL("Failed to create the Vulkan CommandPool!"); }
        }
        ~FVulkanCommandPool() {}
        FVulkanCommandPool(const FVulkanCommandPool&)            = delete;
        FVulkanCommandPool(FVulkanCommandPool&&)                 = default;
        FVulkanCommandPool& operator=(const FVulkanCommandPool&) = delete;
        FVulkanCommandPool& operator=(FVulkanCommandPool&&)      = default;
    };
}