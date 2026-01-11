module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Sync.Fence;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.Global.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanFence
    {
    public:
        [[nodiscard]] VISERA_NOINLINE Bool
        Wait(UInt64 I_Timeout = ~0U) const;
        [[nodiscard]] VISERA_NOINLINE Bool
        Reset() const;

        [[nodiscard]] inline vk::Fence
        GetHandle() const { return Handle; }

        [[nodiscard]] Bool
        IsTimeout() const { return Handle.getStatus() == vk::Result::eTimeout;  };
        [[nodiscard]] Bool
        IsNotReady() const { return Handle.getStatus() == vk::Result::eNotReady;  };
    private:
        vk::raii::Fence Handle {nullptr};

    public:
        FVulkanFence() = default;
        FVulkanFence(const vk::raii::Device& I_Device,
                     Bool                    I_bSignaled)
        {
            auto CreateInfo = vk::FenceCreateInfo{};
            if (I_bSignaled)
            { CreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); }

            auto Result = I_Device.createFence(CreateInfo);
            if (!Result.has_value())
            { LOG_ERROR("Failed to create Vulkan Fence - {}!", Result->getStatus()); }
            else
            { Handle = std::move(*Result); }
        }
        FVulkanFence(FVulkanFence&&) = default;
        FVulkanFence& operator=(FVulkanFence&&) = default;
        ~FVulkanFence() {}
    };

    Bool FVulkanFence::
    Wait(UInt64 I_Timeout /* = ~0U */) const
    {
        vk::Result Result = Handle.getDevice().waitForFences(*Handle, vk::True, I_Timeout);
        if (Result != vk::Result::eSuccess)
        {
            LOG_ERROR("Failed to wait fence (timeout: {}, error: \"{}\")!",
                      I_Timeout, Result);
            return False;
        }
        return True;
    }

    Bool FVulkanFence::
    Reset() const
    {
        auto Result = Handle.getDevice().resetFences(1, &(*Handle));
        if (Result != vk::Result::eSuccess)
        {
            LOG_ERROR("Failed to reset fence (error:{})!", Result);
            return False;
        }
        return True;
    }
}