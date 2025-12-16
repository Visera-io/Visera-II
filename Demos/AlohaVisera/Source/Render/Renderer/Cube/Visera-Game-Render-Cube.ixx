module;
#include <Visera-Game.hpp>
export module Visera.Game.Render.Cube;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.Math.Algebra.Matrix;
import Visera.Runtime.Log;
import Visera.Game.Render.Renderer.Interface;
import Visera.Game.AssetHub;
import Visera.Game.Audio;
import Visera.Runtime.Input;

namespace Visera
{
    struct FDebugCubePushConstants
    {
        FVector2F CubeCenter;
        FVector2F CubeHalfSize;
        Float     CubeAngle;
        FVector2F MousePos;
        Float     Time;
    };



    export class VISERA_ENGINE_API FCubeRenderer : public IRenderer
    {
    public:
        void Render(Float InDeltaTime) override
    {
        static Float    TotalTime   = 0.0f;
        static FVector2F ScreenSize = { 1920.0f, 1080.0f };

        static FVector2F Center     = { 960.0f, 540.0f };
        static FVector2F HalfSize   = { 180.0f, 180.0f };
        static FVector2F Velocity   = { 260.0f, 190.0f };
        static Float     Angle      = 0.7f;

        TotalTime += InDeltaTime;
        Angle     += InDeltaTime;

        Center += Velocity * InDeltaTime;

        bool bHitX = false;
        bool bHitY = false;

        if (Center.X + HalfSize.X > ScreenSize.X)
        {
            Center.X = ScreenSize.X - HalfSize.X;
            Velocity.X = -Velocity.X;
            bHitX = true;
        }
        else if (Center.X - HalfSize.X < 0.0f)
        {
            Center.X = HalfSize.X;
            Velocity.X = -Velocity.X;
            bHitX = true;
        }

        if (Center.Y + HalfSize.Y > ScreenSize.Y)
        {
            Center.Y = ScreenSize.Y - HalfSize.Y;
            Velocity.Y = -Velocity.Y;
            bHitY = true;
        }
        else if (Center.Y - HalfSize.Y < 0.0f)
        {
            Center.Y = HalfSize.Y;
            Velocity.Y = -Velocity.Y;
            bHitY = true;
        }

        if (bHitX || bHitY)
        {
            LOG_INFO("[CubeRenderer] Cube hit border. Pos=({}, {}) Vel=({}, {})",
                     Center.X, Center.Y, Velocity.X, Velocity.Y);
            GAudio->PostEvent("Play_SFX_Collision", 1);
        }

        Int32 mouseX = GInput->GetMouse()->GetPosition().X
            , mouseY = GInput->GetMouse()->GetPosition().Y;


        FDebugCubePushConstants PC{};
        PC.CubeCenter = Center;
        PC.CubeHalfSize = HalfSize;
        PC.CubeAngle = Angle;
        PC.MousePos = FVector2F{
            static_cast<Float>(mouseX),
            static_cast<Float>(mouseY)
        };
        PC.Time = TotalTime;

        auto& Cmds = GRHI->GetDrawCommands();
        Cmds->EnterRenderPass(RenderPipeline);
        {
            Cmds->PushConstants(PC);

            Cmds->Draw(3, 1, 0, 0);
        }
        Cmds->LeaveRenderPass();
    }

    private:
        TSharedPtr<FRHIVertexBuffer> VertexBuffer;
        FVector2F PositionNDC { 0.0f, 0.0f };
        FVector2F VelocityNDC { 0.4f, 0.25f };
        Float     DepthNDC    { 0.5f };
        Float     AngleRad    { 0.0f };
        Float     AngularSpeed{ 1.0f };
        Float     HalfSizeNDC { 0.5f };

        Float     TotalTime   { 0.0f };

        void TickAnimation(Float I_DeltaTime)
        {
            TotalTime += I_DeltaTime;

            PositionNDC.X += VelocityNDC.X * I_DeltaTime;
            PositionNDC.Y += VelocityNDC.Y * I_DeltaTime;

            constexpr Float MinNDC = -1.0f;
            constexpr Float MaxNDC =  1.0f;

            const Float MinX = MinNDC + HalfSizeNDC;
            const Float MaxX = MaxNDC - HalfSizeNDC;
            const Float MinY = MinNDC + HalfSizeNDC;
            const Float MaxY = MaxNDC - HalfSizeNDC;

            Bool bHit = False;

            if (PositionNDC.X < MinX)
            {
                PositionNDC.X = MinX;
                VelocityNDC.X = -VelocityNDC.X;
                bHit = True;
            }
            else if (PositionNDC.X > MaxX)
            {
                PositionNDC.X = MaxX;
                VelocityNDC.X = -VelocityNDC.X;
                bHit = True;
            }

            if (PositionNDC.Y < MinY)
            {
                PositionNDC.Y = MinY;
                VelocityNDC.Y = -VelocityNDC.Y;
                bHit = True;
            }
            else if (PositionNDC.Y > MaxY)
            {
                PositionNDC.Y = MaxY;
                VelocityNDC.Y = -VelocityNDC.Y;
                bHit = True;
            }

            if (bHit)
            {
                LOG_DEBUG("Cube bounced at NDC = ({}, {})", PositionNDC.X, PositionNDC.Y);
                GAudio->PostEvent("Play_SFX_Collision", 1);
            }

            AngleRad += AngularSpeed * I_DeltaTime;
        }

    public:
        FCubeRenderer()
        {
            FRHIRenderPipelineState PSO{};

            PSO.VertexShader = GRHI->CreateShader(
                ERHIShaderStages::Vertex,
                GAssetHub
                    ->LoadShader(FPath("Cube.slang"),
                                 "VertexMain",
                                 EAssetSource::Engine)
                    ->GetSPIRVCode());

            PSO.FragmentShader = GRHI->CreateShader(
                ERHIShaderStages::Fragment,
                GAssetHub
                    ->LoadShader(FPath("Cube.slang"),
                                 "FragmentMain",
                                 EAssetSource::Engine)
                    ->GetSPIRVCode());

            PSO.VertexAssembly.Topology = ERHIPrimitiveTopology::TriangleList;

            PSO.AddPushConstant(FRHIPushConstantRange{
                .Size   = sizeof(FDebugCubePushConstants),
                .Offset = 0,
                .Stages = ERHIShaderStages::All,
            });

            RenderPipeline = GRHI->CreateRenderPipeline("CubePass", PSO);
        }
    };
}