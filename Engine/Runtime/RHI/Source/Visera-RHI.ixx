module;
#include <Visera-RHI.hpp>
export module Visera.RHI;
#define VISERA_MODULE_NAME "RHI"
export import Visera.RHI.Common;
export import Visera.RHI.Types;
export import Visera.RHI.Resource;
       import Visera.RHI.Vulkan;
       import Visera.RHI.Registry;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Delegate;
       import Visera.Global.Service;
       import Visera.Global.Log;
       import vulkan_hpp;

export namespace Visera
{
    using FRHIImageHandle = FRHIResourceHandle;
    using FRHIDrawCalls   = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;

    class VISERA_RHI_API FRHI : public IGlobalService<FRHI>
    {
    public:
        TMulticastDelegate<>
        OnBeginFrame;
        TMulticastDelegate<>
        OnEndFrame;
        TUnicastDelegate<void(FRHIDrawCalls*, FRHIImageView*)>
        DebugUIDrawCalls;

        [[nodiscard]] FRHIImageHandle
        CreateTexture(const FRHITextureCreateDesc& I_ImageDesc);

        [[nodiscard]] Bool
        BeginFrame();
        void
        EndFrame();
        void
        Present() { if (!Driver->Present()) { LOG_ERROR("Failed to present!"); } }

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
        TUniquePtr<FVulkanDriver>   Driver;
        TUniquePtr<FRHIRegistry>    Registry;

        FVulkanCommandPool<EVulkanQueueFamily::Graphics>
        GraphicsCommandPool;

        struct FFrame
        {
            FVulkanFence Fence;
        };
        TArray<FFrame> Frames;
        UInt8 FrameIndex = 0;

    public:
        FRHI() : IGlobalService(FName{"RHI"}) {}
    };

    export inline VISERA_RHI_API TUniquePtr<FRHI>
    GRHI = MakeUnique<FRHI>();

    /*void FRHI::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping RHI.");
        Driver   = MakeUnique<FVulkanDriver>();
        Registry = MakeUnique<FRHIRegistry>(Driver);

        GraphicsCommandPool = Driver->CreateCommandPool<EVulkanQueueFamily::Graphics>
        (
            False
        );

        auto Cmd = GraphicsCommandPool.CreateCommandBuffer(True);
        Cmd.Begin();
        {
            for (auto& Image : Driver->GetSwapChain().Images)
            Cmd.ConvertImageLayout(&Image,
                vk::ImageLayout::ePresentSrcKHR,
                EVulkanGraphicsStage::TopOfPipe,
                vk::AccessFlagBits2::eNone,
                EVulkanGraphicsStage::BottomOfPipe,
                vk::AccessFlagBits2::eNone);
        }
        Cmd.End();
        auto Fence = Driver->CreateFence(False);

        Driver->Submit(&Cmd, nullptr, nullptr, &Fence);
        if (!Fence.Wait())
        {
            LOG_FATAL("Failed to init RHI SwapChain!");
        }

        Frames.resize(Driver->GetSwapChain().Images.size());
        for (auto& Frame : Frames)
        {
            Frame.Fence = Driver->CreateFence(True);
        }

        Status = EStatus::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");

        Registry.reset();
        Driver.reset();

        Status = EStatus::Terminated;
    }*/

    Bool FRHI::
    BeginFrame()
    {
        auto& Frame = Frames[FrameIndex];
        if (!Frame.Fence.Wait()) { return False; }

        if (Driver->WaitNextFrame())
        {
            OnBeginFrame.Broadcast();
            Frame.Fence.Reset();
            return True;
        }
        else
        {
            LOG_ERROR("Failed to begin new frame!");
            return False;
        }
    }

    void FRHI::
    EndFrame()
    {
        OnEndFrame.Broadcast();
        FrameIndex = (FrameIndex + 1) % Frames.size();
    }

    FRHIImageHandle FRHI::
    CreateTexture(const FRHITextureCreateDesc& I_TextureDesc)
    {
        auto Handle = Registry->Register(I_TextureDesc);
        LOG_DEBUG("Registered a new Texture (handle:{}).", Handle);
        return Handle;
    }
}
