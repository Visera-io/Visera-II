module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Vulkan.Registry;
#define VISERA_MODULE_NAME "RHI.Vulkan"
import Visera.RHI.Vulkan.Common;
import Visera.RHI.Vulkan.Image;
import Visera.RHI.Vulkan.Buffer;
import Visera.RHI.Vulkan.Sampler;
import Visera.Runtime.Log;
import Visera.Core.Types.Array;

namespace Visera
{
    export class VISERA_RHI_API FVulkanResourceRegistry
    {
    public:
        // Image registration
        [[nodiscard]] FVulkanImageHandle
        RegisterImage(TSharedPtr<FVulkanImage> I_Image);
        [[nodiscard]] TSharedPtr<FVulkanImage>
        GetImage(FVulkanImageHandle I_Handle) const;
        void
        UnregisterImage(FVulkanImageHandle I_Handle);

        // Buffer registration
        [[nodiscard]] FVulkanBufferHandle
        RegisterBuffer(TSharedPtr<FVulkanBuffer> I_Buffer);
        [[nodiscard]] TSharedPtr<FVulkanBuffer>
        GetBuffer(FVulkanBufferHandle I_Handle) const;
        void
        UnregisterBuffer(FVulkanBufferHandle I_Handle);

        // ImageView registration
        [[nodiscard]] FVulkanImageViewHandle
        RegisterImageView(TSharedPtr<FVulkanImageView> I_ImageView);
        [[nodiscard]] TSharedPtr<FVulkanImageView>
        GetImageView(FVulkanImageViewHandle I_Handle) const;
        void
        UnregisterImageView(FVulkanImageViewHandle I_Handle);

        // Sampler registration
        [[nodiscard]] FVulkanSamplerHandle
        RegisterSampler(TSharedPtr<FVulkanSampler> I_Sampler);
        [[nodiscard]] TSharedPtr<FVulkanSampler>
        GetSampler(FVulkanSamplerHandle I_Handle) const;
        void
        UnregisterSampler(FVulkanSamplerHandle I_Handle);

        // Validation
        [[nodiscard]] Bool
        IsValidImageHandle(FVulkanImageHandle I_Handle) const;
        [[nodiscard]] Bool
        IsValidBufferHandle(FVulkanBufferHandle I_Handle) const;
        [[nodiscard]] Bool
        IsValidImageViewHandle(FVulkanImageViewHandle I_Handle) const;
        [[nodiscard]] Bool
        IsValidSamplerHandle(FVulkanSamplerHandle I_Handle) const;

        // Cleanup - destroys all registered resources
        void
        Clear();

    private:
        TArray<TSharedPtr<FVulkanImage>>     Images;
        TArray<TSharedPtr<FVulkanBuffer>>    Buffers;
        TArray<TSharedPtr<FVulkanImageView>> ImageViews;
        TArray<TSharedPtr<FVulkanSampler>>   Samplers;

        FVulkanImageHandle      NextImageHandle      {1};
        FVulkanBufferHandle     NextBufferHandle     {1};
        FVulkanImageViewHandle  NextImageViewHandle  {1};
        FVulkanSamplerHandle    NextSamplerHandle    {1};
    };

    FVulkanImageHandle FVulkanResourceRegistry::
    RegisterImage(TSharedPtr<FVulkanImage> I_Image)
    {
        VISERA_ASSERT(I_Image != nullptr);
        const auto Handle = NextImageHandle++;
        if (Handle >= Images.size())
        {
            Images.resize(Handle + 1);
        }
        Images[Handle] = I_Image;
        return Handle;
    }

    TSharedPtr<FVulkanImage> FVulkanResourceRegistry::
    GetImage(FVulkanImageHandle I_Handle) const
    {
        if (!IsValidImageHandle(I_Handle))
        {
            LOG_ERROR("Invalid image handle: {}", I_Handle);
            return nullptr;
        }
        return Images[I_Handle];
    }

    void FVulkanResourceRegistry::
    UnregisterImage(FVulkanImageHandle I_Handle)
    {
        if (IsValidImageHandle(I_Handle))
        {
            Images[I_Handle] = nullptr;
        }
    }

    FVulkanBufferHandle FVulkanResourceRegistry::
    RegisterBuffer(TSharedPtr<FVulkanBuffer> I_Buffer)
    {
        VISERA_ASSERT(I_Buffer != nullptr);
        const auto Handle = NextBufferHandle++;
        if (Handle >= Buffers.size())
        {
            Buffers.resize(Handle + 1);
        }
        Buffers[Handle] = I_Buffer;
        return Handle;
    }

    TSharedPtr<FVulkanBuffer> FVulkanResourceRegistry::
    GetBuffer(FVulkanBufferHandle I_Handle) const
    {
        if (!IsValidBufferHandle(I_Handle))
        {
            LOG_ERROR("Invalid buffer handle: {}", I_Handle);
            return nullptr;
        }
        return Buffers[I_Handle];
    }

    void FVulkanResourceRegistry::
    UnregisterBuffer(FVulkanBufferHandle I_Handle)
    {
        if (IsValidBufferHandle(I_Handle))
        {
            Buffers[I_Handle] = nullptr;
        }
    }

    FVulkanImageViewHandle FVulkanResourceRegistry::
    RegisterImageView(TSharedPtr<FVulkanImageView> I_ImageView)
    {
        VISERA_ASSERT(I_ImageView != nullptr);
        const auto Handle = NextImageViewHandle++;
        if (Handle >= ImageViews.size())
        {
            ImageViews.resize(Handle + 1);
        }
        ImageViews[Handle] = I_ImageView;
        return Handle;
    }

    TSharedPtr<FVulkanImageView> FVulkanResourceRegistry::
    GetImageView(FVulkanImageViewHandle I_Handle) const
    {
        if (!IsValidImageViewHandle(I_Handle))
        {
            LOG_ERROR("Invalid image view handle: {}", I_Handle);
            return nullptr;
        }
        return ImageViews[I_Handle];
    }

    void FVulkanResourceRegistry::
    UnregisterImageView(FVulkanImageViewHandle I_Handle)
    {
        if (IsValidImageViewHandle(I_Handle))
        {
            ImageViews[I_Handle] = nullptr;
        }
    }

    FVulkanSamplerHandle FVulkanResourceRegistry::
    RegisterSampler(TSharedPtr<FVulkanSampler> I_Sampler)
    {
        VISERA_ASSERT(I_Sampler != nullptr);
        const auto Handle = NextSamplerHandle++;
        if (Handle >= Samplers.size())
        {
            Samplers.resize(Handle + 1);
        }
        Samplers[Handle] = I_Sampler;
        return Handle;
    }

    TSharedPtr<FVulkanSampler> FVulkanResourceRegistry::
    GetSampler(FVulkanSamplerHandle I_Handle) const
    {
        if (!IsValidSamplerHandle(I_Handle))
        {
            LOG_ERROR("Invalid sampler handle: {}", I_Handle);
            return nullptr;
        }
        return Samplers[I_Handle];
    }

    void FVulkanResourceRegistry::
    UnregisterSampler(FVulkanSamplerHandle I_Handle)
    {
        if (IsValidSamplerHandle(I_Handle))
        {
            Samplers[I_Handle] = nullptr;
        }
    }

    Bool FVulkanResourceRegistry::
    IsValidImageHandle(FVulkanImageHandle I_Handle) const
    {
        return IsValidHandle(I_Handle) && I_Handle < Images.size() && Images[I_Handle] != nullptr;
    }

    Bool FVulkanResourceRegistry::
    IsValidBufferHandle(FVulkanBufferHandle I_Handle) const
    {
        return IsValidHandle(I_Handle) && I_Handle < Buffers.size() && Buffers[I_Handle] != nullptr;
    }

    Bool FVulkanResourceRegistry::
    IsValidImageViewHandle(FVulkanImageViewHandle I_Handle) const
    {
        return IsValidHandle(I_Handle) && I_Handle < ImageViews.size() && ImageViews[I_Handle] != nullptr;
    }

    Bool FVulkanResourceRegistry::
    IsValidSamplerHandle(FVulkanSamplerHandle I_Handle) const
    {
        return IsValidHandle(I_Handle) && I_Handle < Samplers.size() && Samplers[I_Handle] != nullptr;
    }

    void FVulkanResourceRegistry::
    Clear()
    {
        LOG_TRACE("Clearing resource registry - destroying all registered resources.");
        
        // Clear all arrays - this will release all shared pointers,
        // allowing resources to be destroyed and VMA allocations to be freed
        Images.clear();
        Buffers.clear();
        ImageViews.clear();
        Samplers.clear();
        
        // Reset handle counters
        NextImageHandle      = 1;
        NextBufferHandle     = 1;
        NextImageViewHandle  = 1;
        NextSamplerHandle    = 1;
    }
}
