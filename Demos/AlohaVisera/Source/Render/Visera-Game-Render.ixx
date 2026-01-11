module;
#include <Visera-Game.hpp>
export module Visera.Game.Render;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Global;
import Visera.Graphics.RenderGraph;

namespace Visera
{
    export class VISERA_ENGINE_API FRender : public IGlobalService<FRender>
    {
    public:
        void inline
        Tick(Float I_DeltaTime)
        {
            //RenderGraph->Clear();
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
            //RenderGraph->Execute(GRHI->GetCommandBuffer());
        }

    private:
        //TUniquePtr<FRenderGraph>       RenderGraph;

    public:
        FRender() : IGlobalService(FName{"Render"}) {}
    };

    export inline VISERA_ENGINE_API TUniquePtr<FRender>
    GRender = MakeUnique<FRender>();
}
