module;
#include <Visera-Game.hpp>
export module Visera.Game.Render;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Runtime.Global;
import Visera.Runtime.Log;
import Visera.RHI;
import Visera.Graphics.RenderGraph;
import Visera.Graphics.RenderPass.Tonemap;
import Visera.Runtime.Name;

namespace Visera
{
    export class VISERA_ENGINE_API FRender : public IGlobalSingleton<FRender>
    {
    public:
        void inline
        Tick(Float I_DeltaTime)
        {
            RenderGraph->Clear();
            // RenderGraph->AddNode(FName{"Tonemap"},
            // [&](TSharedRef<FRHICommandBuffer> Cmd)
            // {
            //     TonemapPass->Execute(Cmd,
            //         HDRInput,
            //         LDROutput,
            //         Sampler,
            //         Width,
            //         Height);
            // });
            RenderGraph->Execute(GRHI->GetDrawCommands());
        }

    private:
        TUniquePtr<FRenderGraph>       RenderGraph;
        TUniquePtr<FTonemapRenderPass> TonemapPass;

    public:
        void
        Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Render.");
            
            RenderGraph = MakeUnique<FRenderGraph>();
            TonemapPass = MakeUnique<FTonemapRenderPass>();
        }
        void
        Terminate() override
        {
            LOG_TRACE("Terminating Render.");
            
            TonemapPass.reset();
            RenderGraph.reset();
        }

        FRender() : IGlobalSingleton("Render") {}
    };

    export inline VISERA_ENGINE_API TUniquePtr<FRender>
    GRender = MakeUnique<FRender>();
}
