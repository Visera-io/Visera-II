module;
#include <Visera-Runtime.hpp>
#include <vulkan/vulkan_raii.hpp>
export module Visera.Runtime.RHI.Vulkan.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Image;
import Visera.Runtime.RHI.Vulkan.Sampler;
import Visera.Runtime.RHI.Vulkan.DescriptorSetLayout;
import Visera.Core.Log;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API FVulkanDescriptorSet
    {
    public:
        inline void
        WriteImage(UInt32                       I_Binding,
                   TSharedRef<FVulkanImageView> I_ImageView,
                   TSharedRef<FVulkanSampler>   I_Sampler);
        [[nodiscard]] inline vk::DescriptorSet
        GetHandle() const { return Handle; }
        [[nodiscard]] inline const TSharedRef<FVulkanDescriptorSetLayout>
        GetLayout() const { return Layout; }

    private:
        vk::DescriptorSet                      Handle {nullptr}; // The life cycle is managed by the Pool
        TSharedPtr<FVulkanDescriptorSetLayout> Layout {nullptr};

    public:
        FVulkanDescriptorSet() = delete;
        FVulkanDescriptorSet(const vk::raii::DescriptorPool&        I_DescriptorPool,
                             TSharedRef<FVulkanDescriptorSetLayout> I_DescriptorSetLayout);
        ~FVulkanDescriptorSet() = default;
    };

    FVulkanDescriptorSet::
    FVulkanDescriptorSet(const vk::raii::DescriptorPool&        I_DescriptorPool,
                         TSharedRef<FVulkanDescriptorSetLayout> I_DescriptorSetLayout)
    {
        VISERA_ASSERT(I_DescriptorPool != nullptr);

        auto Layout = *I_DescriptorSetLayout->GetHandle();
        auto AllocateInfo = vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool      (I_DescriptorPool)
            .setDescriptorSetCount  (1)
            .setPSetLayouts         (&Layout)
        ;
        auto Result = I_DescriptorPool.getDevice().allocateDescriptorSets(AllocateInfo);
        if (Result.has_value())
        { Handle = (*Result)[0]; }
        else
        { LOG_FATAL("Failed to allocate the Vulkan Descriptor Set!"); }
    }

    void FVulkanDescriptorSet::
    WriteImage(UInt32                       I_Binding,
               TSharedRef<FVulkanImageView> I_ImageView,
               TSharedRef<FVulkanSampler>   I_Sampler)
    {
        auto Image = I_ImageView->GetImage();

        auto ImageInfo = vk::DescriptorImageInfo{}
            .setSampler     (I_Sampler->GetHandle())
            .setImageView   (I_ImageView->GetHandle())
            .setImageLayout (Image->GetLayout())
        ;
        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDstSet      (Handle)
            .setDstBinding  (I_Binding)
            .setPImageInfo  (&ImageInfo)
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }
}