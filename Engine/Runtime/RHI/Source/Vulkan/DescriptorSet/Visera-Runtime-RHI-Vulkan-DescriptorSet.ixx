module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Vulkan.DescriptorSet;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Vulkan.Common;
import Visera.Runtime.RHI.Vulkan.Image;
import Visera.Runtime.RHI.Vulkan.Buffer;
import Visera.Runtime.RHI.Vulkan.Sampler;
import Visera.Runtime.RHI.Vulkan.DescriptorSetLayout;
import Visera.Core.Log;
import Visera.Core.Types.Array;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RUNTIME_API FVulkanDescriptorSet
    {
    public:
        // Write methods (accept raw pointers)
        VISERA_NOINLINE void
        WriteCombinedImageSampler(UInt32                 I_Binding,
                                  FVulkanImageView*      I_ImageView,
                                  FVulkanSampler*        I_Sampler,
                                  UInt32                 I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteSampledImage(UInt32                 I_Binding,
                          FVulkanImageView*      I_ImageView,
                          UInt32                 I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteSampler(UInt32                 I_Binding,
                     FVulkanSampler*        I_Sampler,
                     UInt32                 I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageImage(UInt32                 I_Binding,
                          FVulkanImageView*      I_ImageView,
                          UInt32                 I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteUniformBuffer(UInt32                 I_Binding,
                           FVulkanBuffer*         I_Buffer,
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

    private:
        const vk::raii::DescriptorPool&        Pool;
        vk::DescriptorSet                      Handle {nullptr}; // The life cycle is managed by the Pool

    public:
        FVulkanDescriptorSet() = delete;
        FVulkanDescriptorSet(const vk::raii::DescriptorPool&      I_DescriptorPool,
                             const vk::DescriptorSetAllocateInfo& I_AllocateInfo)
        : Pool  { I_DescriptorPool }
        {
            auto Results = I_DescriptorPool.getDevice().allocateDescriptorSets(I_AllocateInfo);
            if (!Results.has_value())
            { LOG_FATAL("Failed to allocate descriptor set!"); }
            else
            { Handle = std::move(Results->front()); }
        }
        ~FVulkanDescriptorSet() = default;
    };

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
            .setDstArrayElement (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eCombinedImageSampler)
            .setPImageInfo      (&ImageInfo)
        ;
        Pool.getDevice().updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteSampledImage(UInt32                 I_Binding,
                      FVulkanImageView*      I_ImageView,
                      UInt32                 I_ArrayElement /* = 0 */)
    {
        if (!I_ImageView)
        {
            LOG_ERROR("Invalid pointer for sampled image write");
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
            .setDstArrayElement (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eSampledImage)
            .setPImageInfo      (&ImageInfo)
        ;
        
        Pool.getDevice().updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteSampler(UInt32                 I_Binding,
                 FVulkanSampler*        I_Sampler,
                 UInt32                 I_ArrayElement /* = 0 */)
    {
        if (!I_Sampler)
        {
            LOG_ERROR("Invalid pointer for sampler write");
            return;
        }

        auto ImageInfo = vk::DescriptorImageInfo{}
            .setSampler     (I_Sampler->GetHandle())
            .setImageView   (nullptr)
            .setImageLayout (vk::ImageLayout::eUndefined)
        ;
        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (1)
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eSampler)
            .setPImageInfo      (&ImageInfo)
        ;
        
        Pool.getDevice().updateDescriptorSets(
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
            .setDstArrayElement (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageImage)
            .setPImageInfo      (&ImageInfo)
        ;
        
        Pool.getDevice().updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteUniformBuffer(UInt32                 I_Binding,
                       FVulkanBuffer*         I_Buffer,
                       UInt32                 I_ArrayElement /* = 0 */)
    {
        if (!I_Buffer)
        {
            LOG_ERROR("Invalid buffer pointer for uniform buffer write");
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
            .setDstArrayElement (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eUniformBuffer)
            .setPBufferInfo     (&BufferInfo)
        ;
        
        Pool.getDevice().updateDescriptorSets(
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
            .setDstArrayElement (I_ArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageBuffer)
            .setPBufferInfo     (&BufferInfo)
        ;
        
        Pool.getDevice().updateDescriptorSets(
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
        VISERA_ASSERT(I_ImageViews.GetSize() == I_Samplers.GetSize());
        
        const auto Count = I_ImageViews.GetSize();
        if (Count == 0) { return; }

        TArray<vk::DescriptorImageInfo> ImageInfos;
        ImageInfos.Reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            if (!I_ImageViews[Idx] || !I_Samplers[Idx])
            {
                LOG_ERROR("Invalid pointers at index {} for combined image sampler array write", Idx);
                continue;
            }

            auto* Image = I_ImageViews[Idx]->GetImage();
            VISERA_ASSERT(Image != nullptr);

            ImageInfos.EmplaceBack(vk::DescriptorImageInfo{}
                .setSampler     (I_Samplers[Idx]->GetHandle())
                .setImageView   (I_ImageViews[Idx]->GetHandle())
                .setImageLayout (Image->GetLayout())
            );
        }

        if (ImageInfos.IsEmpty()) { return; }

        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (static_cast<UInt32>(ImageInfos.GetSize()))
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement (I_FirstArrayElement)
            .setDescriptorType  (vk::DescriptorType::eCombinedImageSampler)
            .setPImageInfo      (ImageInfos.Data())
        ;
        
        Pool.getDevice().updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteStorageImageArray(UInt32                         I_Binding,
                           const TArray<FVulkanImageView*>& I_ImageViews,
                           UInt32                         I_FirstArrayElement /* = 0 */)
    {
        const auto Count = I_ImageViews.GetSize();
        if (Count == 0) { return; }

        TArray<vk::DescriptorImageInfo> ImageInfos;
        ImageInfos.Reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            if (!I_ImageViews[Idx])
            {
                LOG_ERROR("Invalid pointer at index {} for storage image array write", Idx);
                continue;
            }

            auto* Image = I_ImageViews[Idx]->GetImage();
            VISERA_ASSERT(Image != nullptr);

            ImageInfos.EmplaceBack(vk::DescriptorImageInfo{}
                .setSampler     (nullptr)
                .setImageView   (I_ImageViews[Idx]->GetHandle())
                .setImageLayout (Image->GetLayout())
            );
        }

        if (ImageInfos.IsEmpty()) { return; }

        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (static_cast<UInt32>(ImageInfos.GetSize()))
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement (I_FirstArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageImage)
            .setPImageInfo      (ImageInfos.Data())
        ;
        
        Pool.getDevice().updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }

    void FVulkanDescriptorSet::
    WriteStorageBufferArray(UInt32                         I_Binding,
                            const TArray<FVulkanBuffer*>&    I_Buffers,
                            UInt32                          I_FirstArrayElement /* = 0 */)
    {
        const auto Count = I_Buffers.GetSize();
        if (Count == 0) { return; }

        TArray<vk::DescriptorBufferInfo> BufferInfos;
        BufferInfos.Reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            if (!I_Buffers[Idx])
            {
                LOG_ERROR("Invalid buffer pointer at index {} for storage buffer array write", Idx);
                continue;
            }

            BufferInfos.EmplaceBack(vk::DescriptorBufferInfo{}
                .setBuffer (I_Buffers[Idx]->GetHandle())
                .setOffset (0)
                .setRange  (vk::WholeSize)
            );
        }

        if (BufferInfos.IsEmpty()) { return; }

        auto WriteInfo = vk::WriteDescriptorSet{}
            .setDescriptorCount (static_cast<UInt32>(BufferInfos.GetSize()))
            .setDstSet          (Handle)
            .setDstBinding      (I_Binding)
            .setDstArrayElement (I_FirstArrayElement)
            .setDescriptorType  (vk::DescriptorType::eStorageBuffer)
            .setPBufferInfo     (BufferInfos.Data())
        ;
        
        Pool.getDevice().updateDescriptorSets(
            1, &WriteInfo,
            0, nullptr
        );
    }
}