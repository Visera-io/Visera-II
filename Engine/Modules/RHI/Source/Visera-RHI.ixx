module;
#include <Visera-RHI.hpp>
export module Visera.RHI;
#define VISERA_MODULE_NAME "RHI"
export import Visera.RHI.Common;
export import Visera.RHI.Types;
       import Visera.RHI.Vulkan;
       import Visera.RHI.Registry;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;
       import Visera.Core.Delegate;
       import Visera.Runtime.Global;
       import Visera.Runtime.Log;
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
        TUnicastDelegate<void(FVulkanCommandBuffer*, FRHIImageView*)>
        DebugUIDrawCalls;

        void
        BeginFrame();
        void
        EndFrame();
        inline void
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
        TUniquePtr<FVulkanDriver> Driver;
        FRHIRegistry              Registry;

    public:
        FRHI() : IGlobalSingleton("RHI") {}
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

        // Initialize deferred deletion frame count (default: 4 frames = 3 swapchain images + 1 safety margin)
        // This can be adjusted via Registry.SetNumFramesToExpire() if needed
        Registry.SetNumFramesToExpire(4);

        Status = EStatus::Bootstrapped;
    }

    void FRHI::
    Terminate()
    {
        LOG_TRACE("Terminating RHI.");

        Registry.Clear();
        Driver.reset();

        Status = EStatus::Terminated;
    }

    void FRHI::
    BeginFrame()
    {
        // Wait for fence to ensure GPU has completed previous frame
        while (!Driver->BeginFrame())
        {
            LOG_FATAL("Failed to begin new frame!");
        }
        
        // Flush resources from frames that are now safe to delete
        // Use current frame number (before increment) as the completed frame
        Registry.FlushPendingDeletes(Registry.GetCurrentFrameNumber());
        
        // Increment frame number for this new frame
        Registry.IncrementFrameNumber();
        
        OnBeginFrame.Broadcast();
    }

    void FRHI::
    EndFrame()
    {
        OnEndFrame.Broadcast();

        // auto& CurrentFrame = Driver->GetCurrentFrame();
        // // Get ImageView from RenderTarget (needs registry for Image lookup)
        // auto* CurrentImageView = CurrentFrame.ColorRT.GetImageView();
        // DebugUIDrawCalls.Invoke(&CurrentFrame.DrawCalls, CurrentImageView);
        Driver->EndFrame();
    }
}
