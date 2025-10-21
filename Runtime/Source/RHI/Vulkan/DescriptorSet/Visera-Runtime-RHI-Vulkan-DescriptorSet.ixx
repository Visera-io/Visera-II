module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanDescriptorSetLayout
    {

    };

    export class VISERA_RUNTIME_API FVulkanDescriptorSet
    {
    public:
        [[nodiscard]] inline const vk::raii::DescriptorSet&
        GetHandle() const { return Handle; }

    private:
        vk::raii::DescriptorSet Handle {nullptr};

    public:
        FVulkanDescriptorSet() = delete;
        FVulkanDescriptorSet(vk::raii::DescriptorSet&& I_DescriptorSet);
        ~FVulkanDescriptorSet() = default;
    };

    FVulkanDescriptorSet::
    FVulkanDescriptorSet(vk::raii::DescriptorSet&& I_DescriptorSet)
    : Handle{ std::move(I_DescriptorSet) }
    {
        VISERA_ASSERT(Handle != nullptr);
    }
}