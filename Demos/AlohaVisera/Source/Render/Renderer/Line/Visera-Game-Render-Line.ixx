module;
#include <Visera-Game.hpp>
export module Visera.Game.Render.Line;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Types.Array;
import Visera.Runtime.Log;
import Visera.Game.Render.Renderer.Interface;
import Visera.Game.AssetHub;
import Visera.RHI;
import Visera.Platform.Input;

namespace Visera
{
    export class VISERA_ENGINE_API FLineRenderer : public IRenderer
    {
    public:
        void
        Render(Float I_DeltaTime) override
        {
            auto CursorPosition = GInput->GetMouse()->GetPosition();

            static Bool bStartSampling = false;
            if (!bStartSampling &&
                GInput->GetKeyboard()->IsPressed(FKeyboard::EKey::Space))
            {
                LOG_INFO("Start sampling points!");
                bStartSampling = true;
            }
            if (bStartSampling &&
                GInput->GetKeyboard()->IsPressed(FKeyboard::EKey::S))
            {
                LOG_INFO("Stop sampling points!");
                Points.clear();
                bStartSampling = false;
            }

            static Float SamplingTimer = 0.0;
            if (bStartSampling)
            {
                SamplingTimer += I_DeltaTime;
                if (SamplingTimer >= 0.001)
                {
                    Float CursorX = CursorPosition.X / 1920.0; // [FIXME]: using current frame buffer size
                    Float CursorY = CursorPosition.Y / 1080.0; // [FIXME]: using current frame buffer size
                    Points.emplace_back(FVector2F(CursorX, CursorY));
                    if (Points.size() > 60000)
                    {
                        LOG_FATAL("only 60000 points (about 60s) is supported in current test mode!");
                    }
                    
                    VertexBuffer->Write(Points.data(), Points.size() * sizeof(FVector2F));
                    SamplingTimer = 0.0;
                }
            }

            auto& Cmds = GRHI->GetDrawCommands();
            Cmds->EnterRenderPass(RenderPipeline);
            {
                static Float TotalTime = I_DeltaTime;
                TotalTime += I_DeltaTime;
                Cmds->PushConstants(TotalTime);
                Cmds->BindVertexBuffer(0, VertexBuffer, 0);
                Cmds->Draw(Points.size(), 1, 0, 0);
            }
            Cmds->LeaveRenderPass();
        }

    private:
        TArray<FVector2F> Points;
        TSharedPtr<FRHIVertexBuffer> VertexBuffer;

    public:
        FLineRenderer()
        {
            FRHIRenderPipelineState PSO{};
            PSO.VertexShader   = GRHI->CreateShader( ERHIShaderStages::Vertex,
                GAssetHub->LoadShader(FPath("Line.slang"), "VertexMain", EAssetSource::Engine)
                ->GetSPIRVCode());
            PSO.FragmentShader = GRHI->CreateShader( ERHIShaderStages::Fragment,
                GAssetHub->LoadShader(FPath("Line.slang"), "FragmentMain", EAssetSource::Engine)
                ->GetSPIRVCode());
            PSO.VertexAssembly.Topology = ERHIPrimitiveTopology::LineStrip;
            PSO.AddPushConstant(FRHIPushConstantRange
                {
                    .Size   = sizeof(FRHI::FDefaultPushConstant),
                    .Offset = 0,
                    .Stages = ERHIShaderStages::All,
                });
            RenderPipeline = GRHI->CreateRenderPipeline(
                "LinePass", PSO);

            VertexBuffer = GRHI->CreateMappedVertexBuffer(sizeof(FVector2F) * 60000); // 1000point/s
        }
    };
}
