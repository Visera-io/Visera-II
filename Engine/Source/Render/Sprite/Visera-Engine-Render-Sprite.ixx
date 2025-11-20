module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Render.Sprite;
#define VISERA_MODULE_NAME "Engine.Render"
import Visera.Engine.AssetHub;
import Visera.Engine.Render.Renderer;
import Visera.Runtime.RHI; //[TODO]: Remove

namespace Visera
{
    export class VISERA_ENGINE_API FSpriteRenderer : public IRenderer
    {
    public:
        FRHI::FDefaultPushConstant PushConstant;
        void
        Render() const override
        {
            auto& Cmds = GRHI->GetDrawCommands();
            Cmds->EnterRenderPass(RenderPipeline);
            Cmds->PushConstants(RHI::EShaderStage::eFragment, &PushConstant, 0, sizeof PushConstant);
            Cmds->BindDescriptorSet(RHI::EShaderStage::eFragment, GRHI->GetDefaultDecriptorSet());
            Cmds->Draw(3, 1, 0, 0);
            Cmds->LeaveRenderPass();
        }

    private:
        TSharedPtr<FTexture> Sprite;

    public:
        FSpriteRenderer()
        {
            RenderPipeline = GRHI->CreateRenderPipeline(
                "SpritePass",
                GAssetHub->LoadShader(PATH("Sprite.slang"), "VertexMain", EAssetSource::Engine)->GetSPIRVCode(),
                GAssetHub->LoadShader(PATH("Sprite.slang"), "FragmentMain", EAssetSource::Engine)->GetSPIRVCode()
            );
            RenderPipeline->SetRenderArea({{480,270}, {960, 540}});
            Sprite = GAssetHub->LoadTexture(PATH("Fairy.png"));
            Sprite->GetRHITexture()->WriteTo(GRHI->GetDefaultDecriptorSet(), 0);
        }
    };
}
