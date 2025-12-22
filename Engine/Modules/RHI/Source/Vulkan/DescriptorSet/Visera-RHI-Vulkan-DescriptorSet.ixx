module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.DescriptorSet;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Image;
import Visera.RHI.Vulkan.Sampler;
import Visera.RHI.Vulkan.DescriptorSetLayout;
import Visera.Runtime.Log;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanDescriptorSet
    {
    public:
        VISERA_NOINLINE void
        WriteCombinedImageSampler(UInt32                       I_Binding,
                                  TSharedRef<FVulkanImageView> I_ImageView,
                                  TSharedRef<FVulkanSampler>   I_Sampler);
        VISERA_NOINLINE void
        WriteStorageImage(UInt32                       I_Binding,
                          TSharedRef<FVulkanImageView> I_ImageView);
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
    : Layout { I_DescriptorSetLayout }
    {
        auto LayoutHandle = *Layout->GetHandle();
        auto AllocateInfo = vk::DescriptorSetAllocateInfo{}
            .setDescriptorPool      (I_DescriptorPool)
            .setDescriptorSetCount  (1)
            .setPSetLayouts         (&LayoutHandle)
        ;
        auto Result = I_DescriptorPool.getDevice().allocateDescriptorSets(AllocateInfo);
        if (Result.has_value())
        { Handle = (*Result)[0]; }
        else
        { LOG_FATAL("Failed to allocate the Vulkan Descriptor Set!"); }
    }

    void FVulkanDescriptorSet::
    WriteCombinedImageSampler(UInt32                       I_Binding,
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
            .setDescriptorCount (1)
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDescriptorType  (vk::DescriptorType::eCombinedImageSampler)
            .setPImageInfo      (&ImageInfo)
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteStorageImage(UInt32                       I_Binding,
                      TSharedRef<FVulkanImageView> I_ImageView)
    {
        auto Image = I_ImageView->GetImage();

        auto ImageInfo = vk::DescriptorImageInfo{}
            .setSampler     (nullptr)
            .setImageView   (I_ImageView->GetHandle())
            .setImageLayout (I_ImageView->GetImage()->GetLayout())
        ;
        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (1)
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDescriptorType  (vk::DescriptorType::eStorageImage)
            .setPImageInfo      (&ImageInfo)
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }
}