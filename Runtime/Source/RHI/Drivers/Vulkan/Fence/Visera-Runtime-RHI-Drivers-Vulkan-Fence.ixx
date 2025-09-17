module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Drivers.Vulkan.Fence;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.Fence;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanFence : public IFence
    {
    public:
        Bool
        Wait(UInt64 I_Timeout) const override;
        const void*
        GetHandle() const override { return *Handle; }

    private:
        vk::raii::Fence Handle {nullptr};

    public:
        FVulkanFence(const vk::raii::Device& I_Device, Bool I_bSignaled)
        {
            auto CreateInfo = vk::FenceCreateInfo{};
            if (I_bSignaled)
            { CreateInfo.setFlags(vk::FenceCreateFlagBits::eSignaled); }

            auto Result = I_Device.createFence(CreateInfo);
            if (!Result)
            { LOG_ERROR("Failed to create Vulkan Fence: {}", static_cast<Int32>(Result.error())); }
            else
            { Handle = std::move(*Result); }
        }
        ~FVulkanFence() override {}
    };

    Bool FVulkanFence::
    Wait(UInt64 I_Timeout) const
    {
        vk::Result Result = Handle.getDevice().waitForFences({Handle}, VK_TRUE, I_Timeout);
        if (Result != vk::Result::eSuccess)
        {
            LOG_ERROR("Failed to wait fence (timeout: {}, error:{})!",
                      I_Timeout, static_cast<Int32>(Result));
            return False;
        }
        return True;
    }
}