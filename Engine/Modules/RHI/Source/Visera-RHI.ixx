module;
#include <Visera-RHI.hpp>
export module Visera.RHI;
#define VISERA_MODULE_NAME "RHI"
export import Visera.RHI.Common;
export import Visera.RHI.Types;
       import Visera.RHI.Vulkan;
       import Visera.Assets.Image;
       import Visera.Runtime.Log;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Delegate;
       import Visera.Runtime.Global;
       import vulkan_hpp;

namespace Visera
{
    export class VISERA_RHI_API FRHI : public IGlobalSingleton<FRHI>
    {
    public:
        TMulticastDelegate<>
        OnBeginFrame;
        TMulticastDelegate<>
        OnEndFrame;
        TUnicastDelegate<void(TSharedRef<FRHICommandBuffer>, TSharedRef<FRHIImageView>)>
        DebugUIDrawCalls;

        void
        BeginFrame();
        void
        EndFrame();
        inline void
        Present() { if (!Driver->Present()) { LOG_ERROR("Failed to present!"); } }

        [[nodiscard]] inline TSharedRef<FRHICommandBuffer>
        GetCommandBuffer() { return Driver->GetCurrentFrame().DrawCalls; }

        [[nodiscard]] FRHIImageHandle
        CreateImage(ERHIImageType   I_ImageType,
                    UInt32          I_Width,
                    UInt32          I_Height,
                    UInt32          I_Depth,
                    ERHIFormat      I_Format,
                    ERHIImageUsage  I_Usages);
        [[nodiscard]] FRHIImageViewHandle
        CreateImageView(FRHIImageHandle I_ImageHandle);
        [[nodiscard]] FRHIBufferHandle
        CreateBuffer(UInt64          I_Size,
                     ERHIBufferUsage I_Usages);
        [[nodiscard]] FRHISamplerHandle
        CreateSampler(ERHISamplerType I_SamplerType);
        
        // Resource lookup by handle
        [[nodiscard]] TSharedPtr<FRHIImage>
        GetImage(FRHIImageHandle I_Handle) const;
        [[nodiscard]] TSharedPtr<FRHIImageView>
        GetImageView(FRHIImageViewHandle I_Handle) const;
        [[nodiscard]] TSharedPtr<FRHIVertexBuffer>
        GetBuffer(FRHIBufferHandle I_Handle) const;
        [[nodiscard]] TSharedPtr<FRHISampler>
        GetSampler(FRHISamplerHandle I_Handle) const;
        
        // Resource registry access (for bindless descriptor writes)
        [[nodiscard]] inline const FVulkanResourceRegistry&
        GetResourceRegistry() const { return Driver->GetResourceRegistry(); }

        // Low-level API
        [[nodiscard]] inline const TUniquePtr<FVulkanDriver>&
        GetDriver(DEBUG_ONLY_FIELD(const std::source_location& I_Location = std::source_location::current()))  const
        {
            DEBUG_ONLY_FIELD(LOG_WARN("\"{}\" line:{} \"{}\" accessed the RHI driver.",
                             I_Location.file_name(),
                             I_Location.line(),
                             I_Location.function_name()));
            return Driver;
        };

    private:
        TUniquePtr<FVulkanDriver> Driver;

        FVulkanSamplerHandle DefaultLinearSampler;
        FVulkanSamplerHandle DefaultNearestSampler;
        
        // Handle mapping: RHI handles map directly to Vulkan handles (same UInt32 space)
        // For future multi-backend support, we may need a mapping table

    public:
        FRHI() : IGlobalSingleton("RHI.Vulkan") {}
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;
    };

    export inline VISERA_RHI_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    void FRHI::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping RHI.");
        Driver = MakeUnique<FVulkanDriver>();

        DefaultLinearSampler = Driver->CreateImageSampler(
            TypeCast(ERHIFilter::Linear),
            TypeCast(ERHISamplerAddressMode::Repeat));
        DefaultNearestSampler = Driver->CreateImageSampler(
            TypeCast(ERHIFilter::Nearest),
            TypeCast(ERHISamplerAddressMode::Repeat));

        Status = EStatus::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");

        Driver.reset();

        Status = EStatus::Terminated;
    }

    FRHIImageHandle FRHI::
    CreateImage(ERHIImageType   I_ImageType,
                UInt32          I_Width,
                UInt32          I_Height,
                UInt32          I_Depth,
                ERHIFormat      I_Format,
                ERHIImageUsage  I_Usages)
    {
        LOG_DEBUG("Creating RHI Image (type:{}, extent:[{},{},{}], format:{})",
                  I_ImageType, I_Width, I_Height, I_Depth, I_Format);
        
        auto VulkanImageType = TypeCast(I_ImageType);
        auto VulkanFormat    = TypeCast(I_Format);
        auto VulkanUsages    = TypeCast(I_Usages);
        FRHIExtent3D RHIExtent{I_Width, I_Height, I_Depth};
        auto VulkanExtent    = TypeCast(RHIExtent);
        
        // RHI handles map directly to Vulkan handles (same UInt32 space)
        auto VulkanHandle = Driver->CreateImage(VulkanImageType, VulkanExtent, VulkanFormat, VulkanUsages);
        return static_cast<FRHIImageHandle>(VulkanHandle);
    }

    FRHIImageViewHandle FRHI::
    CreateImageView(FRHIImageHandle I_ImageHandle)
    {
        LOG_DEBUG("Creating RHI Image View (image handle:{})", I_ImageHandle);
        
        // Convert RHI handle to Vulkan handle
        auto VulkanImageHandle = static_cast<FVulkanImageHandle>(I_ImageHandle);
        
        // Create image view with default parameters (can be extended later)
        auto VulkanHandle = Driver->CreateImageView(
            VulkanImageHandle,
            TypeCast(ERHIImageViewType::Image2D),
            TypeCast(ERHIImageAspect::Color)
        );
        return static_cast<FRHIImageViewHandle>(VulkanHandle);
    }

    FRHIBufferHandle FRHI::
    CreateBuffer(UInt64          I_Size,
                 ERHIBufferUsage I_Usages)
    {
        LOG_DEBUG("Creating RHI Buffer (size:{}, usages:{})", I_Size, I_Usages);
        
        // Convert RHI buffer usage flags to Vulkan buffer usage flags
        // ERHIBufferUsage uses BufferUsageFlagBits2 values, but driver expects BufferUsageFlags (based on BufferUsageFlagBits)
        // Since the underlying bit values are compatible for common flags, we can cast through the underlying integer type
        vk::BufferUsageFlags VulkanUsages = static_cast<vk::BufferUsageFlags>(static_cast<UInt32>(I_Usages));
        
        auto VulkanHandle = Driver->CreateBuffer(I_Size, VulkanUsages);
        return static_cast<FRHIBufferHandle>(VulkanHandle);
    }

    FRHISamplerHandle FRHI::
    CreateSampler(ERHISamplerType I_SamplerType)
    {
        LOG_DEBUG("Creating RHI Sampler (type:{})", I_SamplerType);
        
        ERHIFilter Filter;
        switch (I_SamplerType)
        {
        case ERHISamplerType::Linear:
            Filter = ERHIFilter::Linear;
            break;
        case ERHISamplerType::Nearest:
            Filter = ERHIFilter::Nearest;
            break;
        default:
            Filter = ERHIFilter::Linear;
            break;
        }
        
        auto VulkanHandle = Driver->CreateImageSampler(
            TypeCast(Filter),
            TypeCast(ERHISamplerAddressMode::Repeat));
        return static_cast<FRHISamplerHandle>(VulkanHandle);
    }

    TSharedPtr<FRHIImage> FRHI::
    GetImage(FRHIImageHandle I_Handle) const
    {
        auto VulkanHandle = static_cast<FVulkanImageHandle>(I_Handle);
        return Driver->GetImage(VulkanHandle);
    }

    TSharedPtr<FRHIImageView> FRHI::
    GetImageView(FRHIImageViewHandle I_Handle) const
    {
        auto VulkanHandle = static_cast<FVulkanImageViewHandle>(I_Handle);
        return Driver->GetImageView(VulkanHandle);
    }

    TSharedPtr<FRHIVertexBuffer> FRHI::
    GetBuffer(FRHIBufferHandle I_Handle) const
    {
        auto VulkanHandle = static_cast<FVulkanBufferHandle>(I_Handle);
        return Driver->GetBuffer(VulkanHandle);
    }

    TSharedPtr<FRHISampler> FRHI::
    GetSampler(FRHISamplerHandle I_Handle) const
    {
        auto VulkanHandle = static_cast<FVulkanSamplerHandle>(I_Handle);
        return Driver->GetSampler(VulkanHandle);
    }

    void FRHI::
    BeginFrame()
    {
        while (!Driver->BeginFrame())
        {
            LOG_FATAL("Failed to begin new frame!");
        }
        OnBeginFrame.Broadcast();
    }

    void FRHI::
    EndFrame()
    {
        OnEndFrame.Broadcast();

        auto& CurrentFrame = Driver->GetCurrentFrame();
        DebugUIDrawCalls.Invoke(CurrentFrame.DrawCalls, CurrentFrame.ColorRT->GetImageView());
        Driver->EndFrame();
    }
}
