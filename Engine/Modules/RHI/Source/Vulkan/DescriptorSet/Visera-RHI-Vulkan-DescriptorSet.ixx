module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.DescriptorSet;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Image;
import Visera.RHI.Vulkan.Sampler;
import Visera.RHI.Vulkan.DescriptorSetLayout;
import Visera.RHI.Vulkan.Registry;
import Visera.Runtime.Log;
import Visera.Core.Types.Array;
import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FVulkanDescriptorSet
    {
    public:
        // Write methods (accept handles and lookup from registry)
        VISERA_NOINLINE void
        WriteCombinedImageSampler(const FVulkanResourceRegistry& I_Registry,
                                  UInt32                         I_Binding,
                                  FVulkanImageViewHandle         I_ImageViewHandle,
                                  FVulkanSamplerHandle           I_SamplerHandle,
                                  UInt32                         I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageImage(const FVulkanResourceRegistry& I_Registry,
                         UInt32                         I_Binding,
                         FVulkanImageViewHandle         I_ImageViewHandle,
                         UInt32                         I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageBuffer(const FVulkanResourceRegistry& I_Registry,
                           UInt32                         I_Binding,
                           FVulkanBufferHandle            I_BufferHandle,
                           UInt32                         I_ArrayElement = 0);
        VISERA_NOINLINE void
        WriteCombinedImageSamplerArray(const FVulkanResourceRegistry& I_Registry,
                                       UInt32                         I_Binding,
                                       const TArray<FVulkanImageViewHandle>& I_ImageViewHandles,
                                       const TArray<FVulkanSamplerHandle>&   I_SamplerHandles,
                                       UInt32                          I_FirstArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageImageArray(const FVulkanResourceRegistry& I_Registry,
                               UInt32                         I_Binding,
                               const TArray<FVulkanImageViewHandle>& I_ImageViewHandles,
                               UInt32                         I_FirstArrayElement = 0);
        VISERA_NOINLINE void
        WriteStorageBufferArray(const FVulkanResourceRegistry& I_Registry,
                                UInt32                         I_Binding,
                                const TArray<FVulkanBufferHandle>&    I_BufferHandles,
                                UInt32                          I_FirstArrayElement = 0);
        
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
    WriteCombinedImageSampler(const FVulkanResourceRegistry& I_Registry,
                              UInt32                         I_Binding,
                              FVulkanImageViewHandle         I_ImageViewHandle,
                              FVulkanSamplerHandle           I_SamplerHandle,
                              UInt32                         I_ArrayElement /* = 0 */)
    {
        auto ImageView = I_Registry.GetImageView(I_ImageViewHandle);
        auto Sampler = I_Registry.GetSampler(I_SamplerHandle);
        
        if (!ImageView || !Sampler)
        {
            LOG_ERROR("Invalid handles for combined image sampler write (imageView:{}, sampler:{})",
                      I_ImageViewHandle, I_SamplerHandle);
            return;
        }

        auto Image = ImageView->GetImage();
        auto ImageInfo = vk::DescriptorImageInfo{}
            .setSampler     (Sampler->GetHandle())
            .setImageView   (ImageView->GetHandle())
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
    WriteStorageImage(const FVulkanResourceRegistry& I_Registry,
                      UInt32                         I_Binding,
                      FVulkanImageViewHandle         I_ImageViewHandle,
                      UInt32                         I_ArrayElement /* = 0 */)
    {
        auto ImageView = I_Registry.GetImageView(I_ImageViewHandle);
        
        if (!ImageView)
        {
            LOG_ERROR("Invalid image view handle for storage image write: {}", I_ImageViewHandle);
            return;
        }

        auto ImageInfo = vk::DescriptorImageInfo{}
            .setSampler     (nullptr)
            .setImageView   (ImageView->GetHandle())
            .setImageLayout (ImageView->GetImage()->GetLayout())
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
    WriteStorageBuffer(const FVulkanResourceRegistry& I_Registry,
                       UInt32                         I_Binding,
                       FVulkanBufferHandle            I_BufferHandle,
                       UInt32                         I_ArrayElement /* = 0 */)
    {
        auto Buffer = I_Registry.GetBuffer(I_BufferHandle);
        
        if (!Buffer)
        {
            LOG_ERROR("Invalid buffer handle for storage buffer write: {}", I_BufferHandle);
            return;
        }

        auto BufferInfo = vk::DescriptorBufferInfo{}
            .setBuffer (Buffer->GetHandle())
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
    WriteCombinedImageSamplerArray(const FVulkanResourceRegistry& I_Registry,
                                   UInt32                         I_Binding,
                                   const TArray<FVulkanImageViewHandle>& I_ImageViewHandles,
                                   const TArray<FVulkanSamplerHandle>&   I_SamplerHandles,
                                   UInt32                          I_FirstArrayElement /* = 0 */)
    {
        VISERA_ASSERT(I_ImageViewHandles.size() == I_SamplerHandles.size());
        
        const auto Count = I_ImageViewHandles.size();
        if (Count == 0) { return; }

        TArray<vk::DescriptorImageInfo> ImageInfos;
        ImageInfos.reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            auto ImageView = I_Registry.GetImageView(I_ImageViewHandles[Idx]);
            auto Sampler = I_Registry.GetSampler(I_SamplerHandles[Idx]);
            
            if (!ImageView || !Sampler)
            {
                LOG_ERROR("Invalid handles at index {} for combined image sampler array write (imageView:{}, sampler:{})",
                          Idx, I_ImageViewHandles[Idx], I_SamplerHandles[Idx]);
                continue;
            }

            auto Image = ImageView->GetImage();
            ImageInfos.emplace_back(vk::DescriptorImageInfo{}
                .setSampler     (Sampler->GetHandle())
                .setImageView   (ImageView->GetHandle())
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
    WriteStorageImageArray(const FVulkanResourceRegistry& I_Registry,
                           UInt32                         I_Binding,
                           const TArray<FVulkanImageViewHandle>& I_ImageViewHandles,
                           UInt32                         I_FirstArrayElement /* = 0 */)
    {
        const auto Count = I_ImageViewHandles.size();
        if (Count == 0) { return; }

        TArray<vk::DescriptorImageInfo> ImageInfos;
        ImageInfos.reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            auto ImageView = I_Registry.GetImageView(I_ImageViewHandles[Idx]);
            
            if (!ImageView)
            {
                LOG_ERROR("Invalid image view handle at index {} for storage image array write: {}",
                          Idx, I_ImageViewHandles[Idx]);
                continue;
            }

            ImageInfos.emplace_back(vk::DescriptorImageInfo{}
                .setSampler     (nullptr)
                .setImageView   (ImageView->GetHandle())
                .setImageLayout (ImageView->GetImage()->GetLayout())
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
    WriteStorageBufferArray(const FVulkanResourceRegistry& I_Registry,
                            UInt32                         I_Binding,
                            const TArray<FVulkanBufferHandle>&    I_BufferHandles,
                            UInt32                          I_FirstArrayElement /* = 0 */)
    {
        const auto Count = I_BufferHandles.size();
        if (Count == 0) { return; }

        TArray<vk::DescriptorBufferInfo> BufferInfos;
        BufferInfos.reserve(Count);

        for (UInt32 Idx = 0; Idx < Count; ++Idx)
        {
            auto Buffer = I_Registry.GetBuffer(I_BufferHandles[Idx]);
            
            if (!Buffer)
            {
                LOG_ERROR("Invalid buffer handle at index {} for storage buffer array write: {}",
                          Idx, I_BufferHandles[Idx]);
                continue;
            }

            BufferInfos.emplace_back(vk::DescriptorBufferInfo{}
                .setBuffer (Buffer->GetHandle())
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