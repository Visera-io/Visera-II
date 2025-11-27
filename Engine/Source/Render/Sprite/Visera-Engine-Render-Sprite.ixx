module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Render.Sprite;
#define VISERA_MODULE_NAME "Engine.Render"
import Visera.Engine.AssetHub;
import Visera.Engine.Render.Renderer;
import Visera.Runtime.RHI;
import Visera.Runtime.Input; //[TODO]: Remove

namespace Visera
{
    export class VISERA_ENGINE_API FSpriteRenderer : public IRenderer
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
            auto K = GInput->GetKeyboard();
            static Float Scale{0.7};
            static Float TranslateX{0}, TranslateY{0};
            if (K->IsPressed(FKeyboard::EKey::A))
            {
                TranslateX -= 0.5;
            }
            if (K->IsPressed(FKeyboard::EKey::D))
            {
                TranslateX += 0.5;
            }
            if (K->IsPressed(FKeyboard::EKey::W))
            {
                TranslateY -= 0.5;
            }
            if (K->IsPressed(FKeyboard::EKey::S))
            {
                TranslateY += 0.5;
            }
            if (K->IsPressed(FKeyboard::EKey::Q))
            {
                Scale -= 0.001;
                if (Scale < 0.1) { Scale = 0.1; }
            }
            if (K->IsPressed(FKeyboard::EKey::E))
            {
                Scale += 0.001;
            }
            if (K->IsPressed(FKeyboard::EKey::Space))
            {
                PushConstant.Scale =  0.5;
            }
            else PushConstant.Scale =  1.0;

            PushConstant.DeltaTime = I_DeltaTime;
            static FVertex NewCorners[6];
            for (int i = 0; i < 6; i++)
            {
                NewCorners[i].X = (Corners[i].X * Scale + TranslateX) / 1920.0;
                NewCorners[i].Y = (Corners[i].Y * Scale + TranslateY) / 1080.0;
                NewCorners[i].Z = Corners[i].Z;
                NewCorners[i].W = Corners[i].W;
            }

            VertexBuffer2->Write(BGCorners);
            Cmds->EnterRenderPass(RenderPipeline);
            Cmds->BindVertexBuffer(0, VertexBuffer2, 0);
            Cmds->PushConstants(PushConstant);
            Cmds->BindDescriptorSet(0, GRHI->GetDefaultDecriptorSet2());
            Cmds->Draw(6, 1, 0, 0);

            VertexBuffer->Write(NewCorners);
            Cmds->BindVertexBuffer(0, VertexBuffer, 0);
            Cmds->PushConstants(PushConstant);
            Cmds->BindDescriptorSet(0, GRHI->GetDefaultDecriptorSet());
            Cmds->Draw(6, 1, 0, 0);
            Cmds->LeaveRenderPass();
        }

    private:
        FVertex Corners[6];
        FVertex BGCorners[6];
        TSharedPtr<FTexture> Background;
        TSharedPtr<FTexture> Sprite;
        TSharedPtr<FRHIVertexBuffer> VertexBuffer;
        TSharedPtr<FRHIVertexBuffer> VertexBuffer2;

    public:
        FSpriteRenderer()
        {
            // RenderPipeline = GRHI->CreateRenderPipeline(
            //     "SpritePass",
            //     GAssetHub->LoadShader(FPath("Sprite.slang"), "VertexMain", EAssetSource::Engine)->GetSPIRVCode(),
            //     GAssetHub->LoadShader(FPath("Sprite.slang"), "FragmentMain", EAssetSource::Engine)->GetSPIRVCode()
            // );
            FRHIRenderPipelineState PSO{};
            PSO.VertexShader   = GRHI->CreateShader( ERHIShaderStages::Vertex,
                GAssetHub->LoadShader(FPath("Sprite.slang"), "VertexMain", EAssetSource::Engine)
                ->GetSPIRVCode());
            PSO.FragmentShader = GRHI->CreateShader( ERHIShaderStages::Vertex,
                GAssetHub->LoadShader(FPath("Sprite.slang"), "FragmentMain", EAssetSource::Engine)
                ->GetSPIRVCode());

            PSO.AddDescriptorSet(0, FRHIDescriptorSetLayout{}
            .AddBinding(FRHIDescriptorSetBinding::CombinedImageSampler(0, ERHIShaderStages::Fragment))
            .AddBinding(FRHIDescriptorSetBinding::CombinedImageSampler(1, ERHIShaderStages::Fragment))
            );
            PSO.AddPushConstant(FRHIPushConstantRange
                {
                    .Size   = sizeof(FRHI::FDefaultPushConstant),
                    .Offset = 0,
                    .Stages = ERHIShaderStages::All,
                });
            RenderPipeline = GRHI->CreateRenderPipeline(
                "SpritePass", PSO);
            //RenderPipeline->SetRenderArea({{480,270}, {960, 540}});

            Sprite = GAssetHub->LoadTexture(FPath("Fairy.png"));
            Sprite->GetRHITexture()->WriteTo(GRHI->GetDefaultDecriptorSet(), 0);

            VertexBuffer = GRHI->CreateVertexBuffer(sizeof(FVertex) * 6);
            Corners[0].X = -256; Corners[0].Y = -256; // Pos
            Corners[0].Z =  0.0; Corners[0].W =  0.0; // UV
            Corners[1].X =  256; Corners[1].Y = -256; // Pos
            Corners[1].Z =  1.0; Corners[1].W =  0.0; // UV
            Corners[2].X =  256; Corners[2].Y =  256; // Pos
            Corners[2].Z =  1.0; Corners[2].W =  1.0; // UV

            Corners[3].X = Corners[2].X; Corners[3].Y = Corners[2].Y; // Pos
            Corners[3].Z = Corners[2].Z; Corners[3].W = Corners[2].W; // UV

            Corners[4].X = -256; Corners[4].Y =  256; // Pos
            Corners[4].Z =  0.0; Corners[4].W =  1.0; // UV
            Corners[5].X = Corners[0].X; Corners[5].Y = Corners[0].Y; // Pos
            Corners[5].Z = Corners[0].Z; Corners[5].W = Corners[0].W; // UV

            Background = GAssetHub->LoadTexture(FPath("Background.png"));
            Background->GetRHITexture()->WriteTo(GRHI->GetDefaultDecriptorSet2(), 0);

            VertexBuffer2 = GRHI->CreateVertexBuffer(sizeof(FVertex) * 6);
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
