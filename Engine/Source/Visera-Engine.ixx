module;
#include <Visera-Engine.hpp>
export module Visera.Engine;
#define VISERA_MODULE_NAME "Engine"
export import Visera.Engine.AssetHub;
export import Visera.Engine.Audio;
export import Visera.Engine.Render;
export import Visera.Engine.Scripting;
export import Visera.Engine.UI;
       import Visera.Core.Types.Name;
       import Visera.Core.Log;
       import Visera.Runtime;

namespace Visera
{
    export class VISERA_ENGINE_API FEngine : public IGlobalSingleton<FEngine>
    {
    public:
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Engine.");
            GRuntime    ->Bootstrap();
            
            GScripting  ->Bootstrap();
            GAssetHub   ->Bootstrap();
            GRender     ->Bootstrap();
            GAudio      ->Bootstrap();

            Status = EStatus::Bootstrapped;
        }

        void Run()
        {
            VISERA_ASSERT(IsBootstrapped());
            
            if (!GWindow->IsBootstrapped())
            {
                LOG_INFO("Visera Off-Screen Mode.");
                GRHI->BeginFrame();
                {
            
                }
                GRHI->EndFrame();
                return;
            }
            
             auto& Driver = GRHI->GetDriver();

            struct alignas(16) FTestPushConstantRange
            {
                Float Time{0};
                Float CursorX{0}, CursorY{0};
                Float OffsetX{0}, OffsetY{0};
            };
            static_assert(sizeof(FTestPushConstantRange) <= 128);
            FTestPushConstantRange MouseContext{};

            auto PipelineLayout = Driver->CreatePipelineLayout({},
{{ RHI::EShaderStage::eFragment, 0U, UInt32(sizeof MouseContext) }});
            auto RenderPass = GRHI->CreateRenderPipeline(
             "TestPass", PipelineLayout,
             GAssetHub->LoadShader(PATH("Skybox.slang"), "VertexMain")->GetSPIRVCode(),
             GAssetHub->LoadShader(PATH("Skybox.slang"), "FragmentMain")->GetSPIRVCode()
            );
             RenderPass->SetRenderArea({
                 {0,0},
                 {1920, 1080},
             });

             while (!GWindow->ShouldClose())
             {
                 GWindow->PollEvents();
                 GAudio->Tick();
            
                 static Float LastSec {0};
                 LastSec = MouseContext.Time;
                 MouseContext.Time = GRuntime->GetTimer().Elapsed().Milliseconds() / 1000.0;
                 MouseContext.CursorX = GWindow->GetMouse().Cursor.Position.X / GWindow->GetWidth();
                 MouseContext.CursorY = 1.0 - (GWindow->GetMouse().Cursor.Position.Y / GWindow->GetHeight());
                 GWindow->GetMouse().Cursor.Offset.X -= std::min(1.0f, MouseContext.Time - LastSec);
                 if (GWindow->GetMouse().Cursor.Offset.X <= 0) GWindow->GetMouse().Cursor.Offset.X = 0;
                 MouseContext.OffsetX = GWindow->GetMouse().Cursor.Offset.X; // As a Time
            
                 GRHI->BeginFrame();
                 {
                     auto& Cmd = GRHI->GetDrawCommands();
            
                     Cmd->EnterRenderPass(RenderPass);
                      Cmd->PushConstants(RHI::EShaderStage::eFragment, &MouseContext, 0, sizeof MouseContext);
                      Cmd->Draw(3,1,0,0);
                      Cmd->LeaveRenderPass();

                     // GRender->Tick(0);
            
                     GRuntime->StudioBeginFrame();
                     GRuntime->StudioEndFrame();
                 }
                 GRHI->EndFrame();
                 GRHI->Present();
             }
        }

        void Terminate() override
        {
            LOG_TRACE("Terminating Engine.");

            GAudio      ->Terminate();
            GRender     ->Terminate();
            GAssetHub   ->Terminate();
            GScripting  ->Terminate();

            GRuntime    ->Terminate();

            Status = EStatus::Terminated;
        }

        FEngine() : IGlobalSingleton("Engine") {};
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEngine>
    GEngine = MakeUnique<FEngine>();
}
