module;
#include <Visera-RHI.hpp>
export module Visera.RHI;
#define VISERA_MODULE_NAME "RHI"
export import Visera.RHI.Common;
export import Visera.RHI.Types;
export import Visera.RHI.Resource;
       import Visera.RHI.Vulkan;
       import Visera.RHI.Registry;
       import Visera.Core.Types.Array;
       import Visera.Core.Delegate;
       import Visera.Global.Service;
       import Visera.Global.Log;
       import vulkan_hpp;

export namespace Visera
{
    using FRHIImageHandle = FRHIResourceHandle;
    using FRHIDrawCalls   = FVulkanCommandBuffer<EVulkanQueueFamily::Graphics>;

    class VISERA_RHI_API FRHI : public IGlobalService
    {
    public:
        TMulticastDelegate<>
        OnBeginFrame;
        TMulticastDelegate<>
        OnEndFrame;
        TUnicastDelegate<void(FRHIDrawCalls*, FRHIImageView*)>
        DebugUIDrawCalls;

        [[nodiscard]] FRHIImageHandle
        CreateTexture(FRHITextureCreateDesc&& I_ImageDesc);
        void
        DestroyTexture(FRHIImageHandle I_ImageHandle);

        [[nodiscard]] Bool
        BeginFrame();
        void
        EndFrame();
        void
        Present();

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
            FVulkanFence     Fence;
            FVulkanSemaphore SwapChainReadySemaphore;
            FVulkanSemaphore RenderFinishedSemaphore;
            FRHIDrawCalls    DrawCalls;
        };
        TArray<FFrame> InFlightFrames;
        UInt8          FrameIndex = 0;

    public:
        FRHI() : IGlobalService(EName::RHI)
        {
            Dependencies =
            {
                EName::Platform,
#if !defined(VISERA_OFFSCREEN_MODE)
                EName::Window,
#endif
            };

            if (!OnBootstrap.TryBind([this]
            {
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

                InFlightFrames.Resize(Driver->GetSwapChain().Images.GetSize());
                for (auto& Frame : InFlightFrames)
                {
                    Frame.Fence = Driver->CreateFence(True);
                    Frame.SwapChainReadySemaphore = Driver->CreateSemaphore();
                    Frame.RenderFinishedSemaphore = Driver->CreateSemaphore();
                    Frame.DrawCalls = GraphicsCommandPool.CreateCommandBuffer(True);
                }
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                Driver->WaitIdle();
                InFlightFrames.Clear();
                GraphicsCommandPool = {};
                Registry.reset();
                Driver.reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    Bool FRHI::
    BeginFrame()
    {
        FrameIndex = (FrameIndex + 1) % InFlightFrames.GetSize();
        auto& CurrentFrame = InFlightFrames[FrameIndex];

        if (!CurrentFrame.Fence.Wait()) { return False; }

        Registry->CollectGarbage(FrameIndex);

        if (Driver->WaitNextFrame(&CurrentFrame.SwapChainReadySemaphore))
        {
            if (!CurrentFrame.Fence.Reset())
            {
                LOG_ERROR("Failed to reset the Fence!");
                return False;
            }

            CurrentFrame.DrawCalls.Reset();
            CurrentFrame.DrawCalls.Begin();
            OnBeginFrame.Broadcast();

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
        auto& CurrentFrame = InFlightFrames[FrameIndex];
        OnEndFrame.Broadcast();

        CurrentFrame.DrawCalls.End();
        Driver->Submit(&CurrentFrame.DrawCalls,
        &CurrentFrame.SwapChainReadySemaphore,
        &CurrentFrame.RenderFinishedSemaphore,
        &CurrentFrame.Fence);

        Registry->ClearGarbage();
    }


    void FRHI::
    Present()
    {
        auto& CurrentFrame = InFlightFrames[FrameIndex];
        if (!Driver->Present(&CurrentFrame.RenderFinishedSemaphore))
        {
            LOG_ERROR("Failed to present!");
        }
    }


    FRHIImageHandle FRHI::
    CreateTexture(FRHITextureCreateDesc&& I_TextureDesc)
    {
        auto Handle = Registry->Register(std::move(I_TextureDesc));
        return Handle;
    }

    void FRHI::
    DestroyTexture(FRHIImageHandle I_ImageHandle)
    {
        Registry->Unregister(I_ImageHandle, FrameIndex);
    }
}
