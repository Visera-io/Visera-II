module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Render.Sprite;
#define VISERA_MODULE_NAME "Engine.Render"
import Visera.Engine.AssetHub;
import Visera.Engine.Render.Renderer;

namespace Visera
{
    export class VISERA_ENGINE_API FSpriteRenderer : public IRenderer
    {
    public:
        void
        Render() const override
        {
            auto& Cmds = GRHI->GetDrawCommands();
            Cmds->EnterRenderPass(RenderPipeline);
            Cmds->Draw(3, 1, 0, 0);
            Cmds->LeaveRenderPass();
        }

    private:

    public:
        FSpriteRenderer()
        {
            // RenderPipeline = GRHI->CreateRenderPipeline(
            //     FName{"SpritePass"},
            //     GAssetHub->LoadShader(PATH("Sprite.slang"), "VertexMain")->GetSPIRVCode(),
            //     GAssetHub->LoadShader(PATH("Sprite.slang"), "FragmentMain")->GetSPIRVCode()
            // );
            // RenderPipeline->SetRenderArea({
            //      {0,0},
            //      {1920, 1080},
            //  });
        }
    };
}
