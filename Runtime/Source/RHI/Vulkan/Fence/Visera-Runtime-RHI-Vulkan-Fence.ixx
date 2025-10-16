module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.Fence;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanFence
    {
    public:
        [[nodiscard]] inline Bool
        Wait(UInt64 I_Timeout = ~0U) const;
        [[nodiscard]] inline Bool
        Reset() const;

        [[nodiscard]] inline const vk::raii::Fence&
        GetHandle() const { return Handle; }

        [[nodiscard]] Bool
        IsTimeout() const { return Handle.getStatus() == vk::Result::eTimeout;  };
        [[nodiscard]] Bool
        IsNotReady() const { return Handle.getStatus() == vk::Result::eNotReady;  };

    private:
        vk::raii::Fence Handle {nullptr};

    public:
        FVulkanFence() = default;
        FVulkanFence(const vk::raii::Device& I_Device, Bool I_bSignaled)
        {
            auto CreateInfo = vk::FenceCreateInfo{};
            if (I_bSignaled)
            { CreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); }

            auto Result = I_Device.createFence(CreateInfo);
            if (!Result.has_value())
            { LOG_ERROR("Failed to create Vulkan Fence: {}", static_cast<Int32>(Result->getStatus())); }
            else
            { Handle = std::move(*Result); }
        }
        ~FVulkanFence() {}
    };

    Bool FVulkanFence::
    Wait(UInt64 I_Timeout /* = ~0U */) const
    {
        vk::Result Result = Handle.getDevice().waitForFences(*Handle, vk::True, I_Timeout);
        if (Result != vk::Result::eSuccess)
        {
            LOG_ERROR("Failed to wait fence (timeout: {}) -- {}!",
                      I_Timeout, static_cast<Int32>(Result));
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
            LOG_ERROR("Failed to reset fence -- {}!", Result);
            return False;
        }
        return True;
    }
}