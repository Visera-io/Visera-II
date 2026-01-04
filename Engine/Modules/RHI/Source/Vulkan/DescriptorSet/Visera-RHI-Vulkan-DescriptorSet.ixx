module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.DescriptorSet;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Image;
import Visera.RHI.Vulkan.Buffer;
import Visera.RHI.Vulkan.Sampler;
import Visera.RHI.Vulkan.DescriptorSetLayout;
import Visera.Runtime.Log;
import Visera.Core.Types.Array;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanDescriptorSet
    {
    public:
        // Write methods (accept raw pointers)
        VISERA_NOINLINE void
        WriteCombinedImageSampler(UInt32                 I_Binding,
                                  FVulkanImageView*      I_ImageView,
                                  FVulkanSampler*        I_Sampler,
                                  UInt32                 I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageImage(UInt32                 I_Binding,
                          FVulkanImageView*      I_ImageView,
                          UInt32                 I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageBuffer(UInt32                 I_Binding,
                           FVulkanBuffer*         I_Buffer,
                           UInt32                 I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteCombinedImageSamplerArray(UInt32                         I_Binding,
                                       const TArray<FVulkanImageView*>& I_ImageViews,
                                       const TArray<FVulkanSampler*>&   I_Samplers,
                                       UInt32                          I_FirstArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageImageArray(UInt32                         I_Binding,
                               const TArray<FVulkanImageView*>& I_ImageViews,
                               UInt32                         I_FirstArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageBufferArray(UInt32                         I_Binding,
                                const TArray<FVulkanBuffer*>&    I_Buffers,
                                UInt32                          I_FirstArrayElement = 0);
        
        [[nodiscard]] inline vk::DescriptorSet
        GetHandle() const { return Handle; }
        [[nodiscard]] inline FVulkanDescriptorSetLayout*
        GetLayout() const { return Layout; }

    private:
        vk::DescriptorSet                      Handle {nullptr}; // The life cycle is managed by the Pool
        FVulkanDescriptorSetLayout*            Layout {nullptr};

    public:
        FVulkanDescriptorSet() = delete;
        FVulkanDescriptorSet(const vk::raii::DescriptorPool&        I_DescriptorPool,
                             FVulkanDescriptorSetLayout*            I_DescriptorSetLayout);
        ~FVulkanDescriptorSet() = default;
    };

    FVulkanDescriptorSet::
    FVulkanDescriptorSet(const vk::raii::DescriptorPool&        I_DescriptorPool,
                         FVulkanDescriptorSetLayout*            I_DescriptorSetLayout)
    : Layout { I_DescriptorSetLayout }
    {
        VISERA_ASSERT(I_DescriptorSetLayout != nullptr);
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
    WriteCombinedImageSampler(UInt32                 I_Binding,
                              FVulkanImageView*      I_ImageView,
                              FVulkanSampler*        I_Sampler,
                              UInt32                 I_ArrayElement /* = 0 */)
    {
        if (!I_ImageView || !I_Sampler)
        {
            LOG_ERROR("Invalid pointers for combined image sampler write");
            return;
        }

        auto* Image = I_ImageView->GetImage();
        VISERA_ASSERT(Image != nullptr);

        auto ImageInfo = vk::DescriptorImageInfo{}
            .setSampler     (I_Sampler->GetHandle())
            .setImageView   (I_ImageView->GetHandle())
            .setImageLayout (Image->GetLayout())
        ;
        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (1)
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement  (I_ArrayElement)
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
    WriteStorageImage(UInt32                 I_Binding,
                      FVulkanImageView*      I_ImageView,
                      UInt32                 I_ArrayElement /* = 0 */)
    {
        if (!I_ImageView)
        {
            LOG_ERROR("Invalid pointer for storage image write");
            return;
        }

        auto* Image = I_ImageView->GetImage();
        VISERA_ASSERT(Image != nullptr);

        auto ImageInfo = vk::DescriptorImageInfo{}
            .setSampler     (nullptr)
            .setImageView   (I_ImageView->GetHandle())
            .setImageLayout (Image->GetLayout())
        ;
        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (1)
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement  (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageImage)
            .setPImageInfo      (&ImageInfo)
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteStorageBuffer(UInt32                 I_Binding,
                       FVulkanBuffer*         I_Buffer,
                       UInt32                 I_ArrayElement /* = 0 */)
    {
        if (!I_Buffer)
        {
            LOG_ERROR("Invalid buffer pointer for storage buffer write");
            return;
        }

        auto BufferInfo = vk::DescriptorBufferInfo{}
            .setBuffer (I_Buffer->GetHandle())
            .setOffset (0)
            .setRange  (vk::WholeSize)
        ;
        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (1)
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement  (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageBuffer)
            .setPBufferInfo     (&BufferInfo)
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteCombinedImageSamplerArray(UInt32                         I_Binding,
                                   const TArray<FVulkanImageView*>& I_ImageViews,
                                   const TArray<FVulkanSampler*>&   I_Samplers,
                                   UInt32                          I_FirstArrayElement /* = 0 */)
    {
        VISERA_ASSERT(I_ImageViews.size() == I_Samplers.size());
        
        const auto Count = I_ImageViews.size();
        if (Count == 0) { return; }

        TArray<vk::DescriptorImageInfo> ImageInfos;
        ImageInfos.reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            if (!I_ImageViews[Idx] || !I_Samplers[Idx])
            {
                LOG_ERROR("Invalid pointers at index {} for combined image sampler array write", Idx);
                continue;
            }

            auto* Image = I_ImageViews[Idx]->GetImage();
            VISERA_ASSERT(Image != nullptr);

            ImageInfos.emplace_back(vk::DescriptorImageInfo{}
                .setSampler     (I_Samplers[Idx]->GetHandle())
                .setImageView   (I_ImageViews[Idx]->GetHandle())
                .setImageLayout (Image->GetLayout())
            );
        }

        if (ImageInfos.empty()) { return; }

        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (static_cast<UInt32>(ImageInfos.size()))
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement  (I_FirstArrayElement)
            .setDescriptorType  (vk::DescriptorType::eCombinedImageSampler)
            .setPImageInfo      (ImageInfos.data())
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteStorageImageArray(UInt32                         I_Binding,
                           const TArray<FVulkanImageView*>& I_ImageViews,
                           UInt32                         I_FirstArrayElement /* = 0 */)
    {
        const auto Count = I_ImageViews.size();
        if (Count == 0) { return; }

        TArray<vk::DescriptorImageInfo> ImageInfos;
        ImageInfos.reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            if (!I_ImageViews[Idx])
            {
                LOG_ERROR("Invalid pointer at index {} for storage image array write", Idx);
                continue;
            }

            auto* Image = I_ImageViews[Idx]->GetImage();
            VISERA_ASSERT(Image != nullptr);

            ImageInfos.emplace_back(vk::DescriptorImageInfo{}
                .setSampler     (nullptr)
                .setImageView   (I_ImageViews[Idx]->GetHandle())
                .setImageLayout (Image->GetLayout())
            );
        }

        if (ImageInfos.empty()) { return; }

        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (static_cast<UInt32>(ImageInfos.size()))
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement  (I_FirstArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageImage)
            .setPImageInfo      (ImageInfos.data())
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteStorageBufferArray(UInt32                         I_Binding,
                            const TArray<FVulkanBuffer*>&    I_Buffers,
                            UInt32                          I_FirstArrayElement /* = 0 */)
    {
        const auto Count = I_Buffers.size();
        if (Count == 0) { return; }

        TArray<vk::DescriptorBufferInfo> BufferInfos;
        BufferInfos.reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            if (!I_Buffers[Idx])
            {
                LOG_ERROR("Invalid buffer pointer at index {} for storage buffer array write", Idx);
                continue;
            }

            BufferInfos.emplace_back(vk::DescriptorBufferInfo{}
                .setBuffer (I_Buffers[Idx]->GetHandle())
                .setOffset (0)
                .setRange  (vk::WholeSize)
            );
        }

        if (BufferInfos.empty()) { return; }

        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (static_cast<UInt32>(BufferInfos.size()))
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement  (I_FirstArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageBuffer)
            .setPBufferInfo     (BufferInfos.data())
        ;
        const auto& Device = Layout->GetHandle().getDevice();
        Device.updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }
}