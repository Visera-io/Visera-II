module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Registry;
#define VISERA_MODULE_NAME "RHI.Registry"
import Visera.RHI.Common;
import Visera.RHI.Vulkan;
import Visera.Core.Types.SlotMap;
import Visera.Core.Types.Map;
import Visera.Core.Types.Array;
import Visera.Runtime.Log;

namespace Visera
{
    enum class ERHIResourceType : UInt8
    {
        Image,
        ImageView,
        Buffer,
        Sampler,
    };

    struct FDeferredDeleteEntry
    {
        UInt64 RetireFrame;           // Frame number when resource becomes safe to delete
        ERHIResourceType Type;        // Which SlotMap to erase from
        FRHIResourceHandle Handle;     // Handle to erase
    };

    export class VISERA_RHI_API FRHIRegistry
    {
    public:
        [[nodiscard]] FVulkanImage*
        Get(FRHIImageHandle I_Handle);
        [[nodiscard]] FVulkanImageView*
        Get(FRHIImageViewHandle I_Handle);
        [[nodiscard]] FVulkanBuffer*
        Get(FRHIBufferHandle I_Handle);
        [[nodiscard]] FVulkanSampler*
        Get(FRHISamplerHandle I_Handle);

        [[nodiscard]] Bool
        Contains(FRHIImageHandle I_Handle) const;
        [[nodiscard]] Bool
        Contains(FRHIImageViewHandle I_Handle) const;
        [[nodiscard]] Bool
        Contains(FRHIBufferHandle I_Handle) const;
        [[nodiscard]] Bool
        Contains(FRHISamplerHandle I_Handle) const;

        [[nodiscard]] FRHIImageHandle
        Register(FVulkanImage&& I_Image)       { return Images.Insert(std::move(I_Image)); }
        [[nodiscard]] FRHIImageViewHandle
        Register(FVulkanImageView&& I_ImageView, FRHIImageHandle I_ImageHandle);
        [[nodiscard]] FRHIBufferHandle
        Register(FVulkanBuffer&& I_Buffer)     { return Buffers.Insert(std::move(I_Buffer)); }
        [[nodiscard]] FRHISamplerHandle
        Register(FVulkanSampler&& I_Sampler)   { return Samplers.Insert(std::move(I_Sampler)); }

        void
        RequestDestroy(FRHIImageHandle I_Handle);
        void
        RequestDestroy(FRHIImageViewHandle I_Handle);
        void
        RequestDestroy(FRHIBufferHandle I_Handle);
        void
        RequestDestroy(FRHISamplerHandle I_Handle);

        void
        FlushPendingDeletes(UInt64 I_CompletedFrameNumber);

        void
        IncrementFrameNumber() { CurrentFrameNumber++; }

        [[nodiscard]] UInt64
        GetCurrentFrameNumber() const { return CurrentFrameNumber; }

        void
        SetNumFramesToExpire(UInt8 I_NumFrames) { NumFramesToExpire = I_NumFrames; }

        void Clear();

    private:
        TSlotMap<FVulkanImage,      FRHIImageHandle>        Images;
        TSlotMap<FVulkanBuffer,     FRHIBufferHandle>       Buffers;
        TSlotMap<FVulkanImageView,  FRHIImageViewHandle>    ImageViews;
        TSlotMap<FVulkanSampler,    FRHISamplerHandle>      Samplers;

        // Reference counting for Image dependencies (ImageView -> Image)
        TMap<FRHIImageHandle, UInt32>                 ImageReferenceCounts;
        TMap<FRHIImageViewHandle, FRHIImageHandle>    ImageViewToImage;

        // Deferred deletion system
        TArray<FDeferredDeleteEntry>                 DeferredDeleteQueue;
        TMap<FRHIImageHandle, Bool>                   PendingDestroyImages;
        TMap<FRHIImageViewHandle, Bool>               PendingDestroyImageViews;
        TMap<FRHIBufferHandle, Bool>                  PendingDestroyBuffers;
        TMap<FRHISamplerHandle, Bool>                 PendingDestroySamplers;

        UInt64                                        CurrentFrameNumber = 0;
        UInt8                                         NumFramesToExpire = 4; // Default: 3 swapchain images + 1 safety margin

        void
        UnregisterImmediate(ERHIResourceType I_Type, FRHIResourceHandle I_Handle);

        [[nodiscard]] Bool
        IsPendingDestroy(FRHIImageHandle I_Handle) const;
        [[nodiscard]] Bool
        IsPendingDestroy(FRHIImageViewHandle I_Handle) const;
        [[nodiscard]] Bool
        IsPendingDestroy(FRHIBufferHandle I_Handle) const;
        [[nodiscard]] Bool
        IsPendingDestroy(FRHISamplerHandle I_Handle) const;

    public:
        FRHIRegistry() = default;
        FRHIRegistry(const FRHIRegistry&) = delete;
        FRHIRegistry& operator=(const FRHIRegistry&) = delete;
    };
    
    inline FRHIImageViewHandle FRHIRegistry::
    Register(FVulkanImageView&& I_ImageView, FRHIImageHandle I_ImageHandle)
    {
        // Validate ImageHandle BEFORE inserting ImageView to avoid orphaned resources
        if (!Images.Contains(I_ImageHandle))
        {
            LOG_ERROR("Cannot register ImageView: ImageHandle {} is invalid or does not exist", I_ImageHandle);
            return FRHIImageViewHandle::Null; // Return null handle on validation failure
        }
        
        // ImageHandle is valid, proceed with insertion
        const auto Handle = ImageViews.Insert(std::move(I_ImageView));
        
        // Update reference counting and store ImageView->Image mapping
        ImageReferenceCounts[I_ImageHandle]++;
        ImageViewToImage[Handle] = I_ImageHandle;
        
        return Handle;
    }

    inline FVulkanImage* FRHIRegistry::
    Get(FRHIImageHandle I_Handle)
    {
        if (IsPendingDestroy(I_Handle))
        {
            VISERA_ASSERT(False && "Attempted to Get() a resource that is pending destruction");
            return nullptr;
        }
        return Images.Get(I_Handle);
    }

    inline FVulkanImageView* FRHIRegistry::
    Get(FRHIImageViewHandle I_Handle)
    {
        if (IsPendingDestroy(I_Handle))
        {
            VISERA_ASSERT(False && "Attempted to Get() a resource that is pending destruction");
            return nullptr;
        }
        return ImageViews.Get(I_Handle);
    }

    inline FVulkanBuffer* FRHIRegistry::
    Get(FRHIBufferHandle I_Handle)
    {
        if (IsPendingDestroy(I_Handle))
        {
            VISERA_ASSERT(False && "Attempted to Get() a resource that is pending destruction");
            return nullptr;
        }
        return Buffers.Get(I_Handle);
    }

    inline FVulkanSampler* FRHIRegistry::
    Get(FRHISamplerHandle I_Handle)
    {
        if (IsPendingDestroy(I_Handle))
        {
            VISERA_ASSERT(False && "Attempted to Get() a resource that is pending destruction");
            return nullptr;
        }
        return Samplers.Get(I_Handle);
    }

    inline Bool FRHIRegistry::
    Contains(FRHIImageHandle I_Handle) const
    {
        if (IsPendingDestroy(I_Handle))
        {
            return False;
        }
        return Images.Contains(I_Handle);
    }

    inline Bool FRHIRegistry::
    Contains(FRHIImageViewHandle I_Handle) const
    {
        if (IsPendingDestroy(I_Handle))
        {
            return False;
        }
        return ImageViews.Contains(I_Handle);
    }

    inline Bool FRHIRegistry::
    Contains(FRHIBufferHandle I_Handle) const
    {
        if (IsPendingDestroy(I_Handle))
        {
            return False;
        }
        return Buffers.Contains(I_Handle);
    }

    inline Bool FRHIRegistry::
    Contains(FRHISamplerHandle I_Handle) const
    {
        if (IsPendingDestroy(I_Handle))
        {
            return False;
        }
        return Samplers.Contains(I_Handle);
    }

    inline Bool FRHIRegistry::
    IsPendingDestroy(FRHIImageHandle I_Handle) const
    {
        auto It = PendingDestroyImages.find(I_Handle);
        return It != PendingDestroyImages.end() && It->second;
    }

    inline Bool FRHIRegistry::
    IsPendingDestroy(FRHIImageViewHandle I_Handle) const
    {
        auto It = PendingDestroyImageViews.find(I_Handle);
        return It != PendingDestroyImageViews.end() && It->second;
    }

    inline Bool FRHIRegistry::
    IsPendingDestroy(FRHIBufferHandle I_Handle) const
    {
        auto It = PendingDestroyBuffers.find(I_Handle);
        return It != PendingDestroyBuffers.end() && It->second;
    }

    inline Bool FRHIRegistry::
    IsPendingDestroy(FRHISamplerHandle I_Handle) const
    {
        auto It = PendingDestroySamplers.find(I_Handle);
        return It != PendingDestroySamplers.end() && It->second;
    }

    inline void FRHIRegistry::
    RequestDestroy(FRHIImageHandle I_Handle)
    {
        // Check if already pending or doesn't exist
        if (IsPendingDestroy(I_Handle) || !Images.Contains(I_Handle))
        {
            return;
        }

        // Check reference count - Image cannot be destroyed if ImageViews still reference it
        auto RefCountIt = ImageReferenceCounts.find(I_Handle);
        if (RefCountIt != ImageReferenceCounts.end() && RefCountIt->second > 0)
        {
            LOG_ERROR("Cannot request destroy Image (handle:{}): {} ImageView(s) still reference it",
                      I_Handle, RefCountIt->second);
            return;
        }

        // Double-check: verify no non-pending ImageViews reference this Image
        // This is a safety check in case refcount tracking is inconsistent
        for (const auto& [ImageViewHandle, ImageHandle] : ImageViewToImage)
        {
            if (ImageHandle == I_Handle && !IsPendingDestroy(ImageViewHandle))
            {
                LOG_ERROR("Cannot request destroy Image (handle:{}): ImageView (handle:{}) still references it and is not pending destruction",
                          I_Handle, ImageViewHandle);
                return;
            }
        }

        // Mark as pending destroy
        PendingDestroyImages[I_Handle] = True;

        // Calculate retire frame
        const UInt64 RetireFrame = CurrentFrameNumber + NumFramesToExpire;

        // Enqueue for deferred deletion
        DeferredDeleteQueue.push_back(FDeferredDeleteEntry{
            .RetireFrame = RetireFrame,
            .Type = ERHIResourceType::Image,
            .Handle = I_Handle
        });
    }

    inline void FRHIRegistry::
    RequestDestroy(FRHIImageViewHandle I_Handle)
    {
        // Check if already pending or doesn't exist
        if (IsPendingDestroy(I_Handle) || !ImageViews.Contains(I_Handle))
        {
            return;
        }

        // Mark as pending destroy
        PendingDestroyImageViews[I_Handle] = True;

        // Calculate retire frame
        const UInt64 RetireFrame = CurrentFrameNumber + NumFramesToExpire;

        // Enqueue for deferred deletion
        DeferredDeleteQueue.push_back(FDeferredDeleteEntry{
            .RetireFrame = RetireFrame,
            .Type = ERHIResourceType::ImageView,
            .Handle = I_Handle
        });

        // Decrement Image reference count
        auto ImageIt = ImageViewToImage.find(I_Handle);
        if (ImageIt != ImageViewToImage.end())
        {
            const auto ImageHandle = ImageIt->second;
            auto RefCountIt = ImageReferenceCounts.find(ImageHandle);
            if (RefCountIt != ImageReferenceCounts.end() && RefCountIt->second > 0)
            {
                RefCountIt->second--;
                if (RefCountIt->second == 0)
                {
                    // Reference count reached 0, can now request destroy the Image
                    ImageReferenceCounts.erase(RefCountIt);
                    // Request destroy the Image if it's not already pending
                    if (!IsPendingDestroy(ImageHandle))
                    {
                        RequestDestroy(ImageHandle);
                    }
                }
            }
        }
    }

    inline void FRHIRegistry::
    RequestDestroy(FRHIBufferHandle I_Handle)
    {
        // Check if already pending or doesn't exist
        if (IsPendingDestroy(I_Handle) || !Buffers.Contains(I_Handle))
        {
            return;
        }

        // Mark as pending destroy
        PendingDestroyBuffers[I_Handle] = True;

        // Calculate retire frame
        const UInt64 RetireFrame = CurrentFrameNumber + NumFramesToExpire;

        // Enqueue for deferred deletion
        DeferredDeleteQueue.push_back(FDeferredDeleteEntry{
            .RetireFrame = RetireFrame,
            .Type = ERHIResourceType::Buffer,
            .Handle = I_Handle
        });
    }

    inline void FRHIRegistry::
    RequestDestroy(FRHISamplerHandle I_Handle)
    {
        // Check if already pending or doesn't exist
        if (IsPendingDestroy(I_Handle) || !Samplers.Contains(I_Handle))
        {
            return;
        }

        // Mark as pending destroy
        PendingDestroySamplers[I_Handle] = True;

        // Calculate retire frame
        const UInt64 RetireFrame = CurrentFrameNumber + NumFramesToExpire;

        // Enqueue for deferred deletion
        DeferredDeleteQueue.push_back(FDeferredDeleteEntry{
            .RetireFrame = RetireFrame,
            .Type = ERHIResourceType::Sampler,
            .Handle = I_Handle
        });
    }

    inline void FRHIRegistry::
    FlushPendingDeletes(UInt64 I_CompletedFrameNumber)
    {
        // Process expired entries (swap-pop for efficiency)
        // Iterate backwards to safely remove elements while iterating
        for (Int64 I = static_cast<Int64>(DeferredDeleteQueue.size()) - 1; I >= 0; --I)
        {
            const size_t Index = static_cast<size_t>(I);
            auto& Entry = DeferredDeleteQueue[Index];
            if (Entry.RetireFrame <= I_CompletedFrameNumber)
            {
                // This entry is safe to delete now
                UnregisterImmediate(Entry.Type, Entry.Handle);
                
                // Remove from queue (swap with last, then pop)
                if (Index < DeferredDeleteQueue.size() - 1)
                {
                    Entry = DeferredDeleteQueue.back();
                }
                DeferredDeleteQueue.pop_back();
            }
        }
    }

    inline void FRHIRegistry::
    UnregisterImmediate(ERHIResourceType I_Type, FRHIResourceHandle I_Handle)
    {
        switch (I_Type)
        {
        case ERHIResourceType::Image:
        {
            const auto Handle = FRHIImageHandle(I_Handle);
            if (!Images.Erase(Handle))
            {
                LOG_ERROR("Failed to erase Image (handle:{}).", Handle);
            }
            PendingDestroyImages.erase(Handle);
            ImageReferenceCounts.erase(Handle);
            break;
        }
        case ERHIResourceType::ImageView:
        {
            const auto Handle = FRHIImageViewHandle(I_Handle);
            // Decrement Image reference count (should already be handled in RequestDestroy, but double-check)
            auto ImageIt = ImageViewToImage.find(Handle);
            if (ImageIt != ImageViewToImage.end())
            {
                ImageViewToImage.erase(ImageIt);
            }
            if (!ImageViews.Erase(Handle))
            {
                LOG_ERROR("Failed to erase ImageView (handle:{}).", Handle);
            }
            PendingDestroyImageViews.erase(Handle);
            break;
        }
        case ERHIResourceType::Buffer:
        {
            const auto Handle = FRHIBufferHandle(I_Handle);
            if (!Buffers.Erase(Handle))
            {
                LOG_ERROR("Failed to erase Buffer (handle:{}).", Handle);
            }
            PendingDestroyBuffers.erase(Handle);
            break;
        }
        case ERHIResourceType::Sampler:
        {
            const auto Handle = FRHISamplerHandle(I_Handle);
            if (!Samplers.Erase(Handle))
            {
                LOG_ERROR("Failed to erase Sampler (handle:{}).", Handle);
            }
            PendingDestroySamplers.erase(Handle);
            break;
        }
        default:
            LOG_ERROR("Unknown resource type in UnregisterImmediate");
            break;
        }
    }

    inline void FRHIRegistry::
    Clear()
    {
        LOG_DEBUG("Clearing RHI resource registry...");
        
        // Force flush all pending deletions by setting all retire frames to 0
        for (auto& Entry : DeferredDeleteQueue)
        {
            Entry.RetireFrame = 0;
        }
        FlushPendingDeletes(0);
        
        // Clear reference counting maps
        ImageReferenceCounts.clear();
        ImageViewToImage.clear();
        
        // Clear pending destroy maps
        PendingDestroyImages.clear();
        PendingDestroyImageViews.clear();
        PendingDestroyBuffers.clear();
        PendingDestroySamplers.clear();
        
        // Clear deferred delete queue
        DeferredDeleteQueue.clear();
        
        // Clear all SlotMaps - this will destroy all resources and free VMA allocations
        Images.Clear();
        Buffers.Clear();
        ImageViews.Clear();
        Samplers.Clear();
        
        // Reset frame number
        CurrentFrameNumber = 0;
    }
}
