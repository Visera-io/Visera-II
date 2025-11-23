module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Render.Background;
#define VISERA_MODULE_NAME "Engine.Render"
import Visera.Engine.AssetHub;
import Visera.Engine.Render.Renderer;
import Visera.Runtime.RHI; //[TODO]: Remove

namespace Visera
{
    export class VISERA_ENGINE_API FBackgroundRenderer : public IRenderer
    {
    public:
        struct FVertex
        {
            Float X{0}, Y{0}, Z{0}, W{0};
        };
        FRHI::FDefaultPushConstant PushConstant;

        void
        Render(Float I_DeltaTime) override
        {
            auto& Cmds = GRHI->GetDrawCommands();

            VertexBuffer->Write(BGCorners, sizeof(FVertex) * 6);
            Cmds->EnterRenderPass(RenderPipeline);
            Cmds->BindVertexBuffer(0, VertexBuffer, 0);
            Cmds->PushConstants(PushConstant);
            Cmds->BindDescriptorSet(0, GRHI->GetDefaultDecriptorSet());
            Cmds->Draw(6, 1, 0, 0);
            Cmds->LeaveRenderPass();
        }

    private:
        FVertex BGCorners[6];
        TSharedPtr<FTexture> Background;
        TSharedPtr<FBuffer> VertexBuffer;

    public:
        FBackgroundRenderer()
        {
            RenderPipeline = GRHI->CreateRenderPipeline(
                "SpritePass",
                GAssetHub->LoadShader(PATH("Background.slang"), "VertexMain", EAssetSource::Engine)->GetSPIRVCode(),
                GAssetHub->LoadShader(PATH("Background.slang"), "FragmentMain", EAssetSource::Engine)->GetSPIRVCode()
            );
            //RenderPipeline->SetRenderArea({{480,270}, {960, 540}});

            Background = GAssetHub->LoadTexture(PATH("Background.png"));
            Background->GetRHITexture()->WriteTo(GRHI->GetDefaultDecriptorSet(), 0);

            VertexBuffer = GRHI->CreateVertexBuffer(sizeof(FVertex) * 6);
            BGCorners[0].X = -1.00; BGCorners[0].Y = -1.00; // Pos
            BGCorners[0].Z =  0.0; BGCorners[0].W =  0.0; // UV
            BGCorners[1].X =  1.00; BGCorners[1].Y = -1.00; // Pos
            BGCorners[1].Z =  1.0; BGCorners[1].W =  0.0; // UV
            BGCorners[2].X =  1.00; BGCorners[2].Y =  1.00; // Pos
            BGCorners[2].Z =  1.0; BGCorners[2].W =  1.0; // UV

            BGCorners[3].X = BGCorners[2].X; BGCorners[3].Y = BGCorners[2].Y; // Pos
            BGCorners[3].Z = BGCorners[2].Z; BGCorners[3].W = BGCorners[2].W; // UV

            BGCorners[4].X = -1.00; BGCorners[4].Y =  1.00; // Pos
            BGCorners[4].Z =  0.0; BGCorners[4].W =  1.0; // UV
            BGCorners[5].X = BGCorners[0].X; BGCorners[5].Y = BGCorners[0].Y; // Pos
            BGCorners[5].Z = BGCorners[0].Z; BGCorners[5].W = BGCorners[0].W; // UV
        }
    };
}
