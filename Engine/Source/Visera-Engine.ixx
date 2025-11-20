module;
#include <Visera-Engine.hpp>
export module Visera.Engine;
#define VISERA_MODULE_NAME "Engine"
export import Visera.Engine.AssetHub;
export import Visera.Engine.Audio;
export import Visera.Engine.Render;
export import Visera.Engine.Scripting;
export import Visera.Engine.Event;
export import Visera.Engine.UI;
       import Visera.Core.Types.Name;
       import Visera.Core.Delegate;
       import Visera.Core.OS.Time;
       import Visera.Core.Log;
       import Visera.Runtime;

namespace Visera
{
    export class VISERA_ENGINE_API FEngine : public IGlobalSingleton<FEngine>
    {
    public:
        TDelegate<Float>     AppTick;
        TMulticastDelegate<> OnBootstrap;
        TMulticastDelegate<> OnTerminate;

        void Run()
        {
            VISERA_ASSERT(IsBootstrapped());

            if (!GWindow->IsBootstrapped())
            {
                LOG_INFO("Visera Off-Screen Mode.");
                GRHI->BeginFrame();
                {
                    GEvent->OnFrameBegin.Broadcast();
                    AppTick.Invoke(0);
                    GEvent->OnFrameEnd.Broadcast();
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
                 auto Mouse = GInput->GetMouse();
                 MouseContext.Time = GRuntime->GetTimer().Elapsed().Milliseconds() / 1000.0;
                 MouseContext.CursorX = Mouse->GetPosition().X / GWindow->GetWidth();
                 MouseContext.CursorY = 1.0 - (Mouse->GetPosition().Y / GWindow->GetHeight());
                 auto DeltaX = Mouse->GetOffset().X - std::min(1.0f, MouseContext.Time - LastSec);

                 MouseContext.OffsetX = std::max(0.0f, DeltaX); // As a Time

                 Float DeltaTime = Timer.Tick().Microseconds() / 1000000.0; Timer.Reset();

                 GRHI->BeginFrame();
                 {
                     GEvent->OnFrameBegin.Broadcast();
                     auto& Cmd = GRHI->GetDrawCommands();
            
                     Cmd->EnterRenderPass(RenderPass);
                      Cmd->PushConstants(RHI::EShaderStage::eFragment, &MouseContext, 0, sizeof MouseContext);
                      Cmd->Draw(3,1,0,0);
                      Cmd->LeaveRenderPass();

                     AppTick.Invoke(DeltaTime);
                     // GRender->Tick(0);

                     GEvent->OnFrameEnd.Broadcast();
                 }
                 GRHI->EndFrame();
                 GRHI->Present();
             }
        }
    private:
        FHiResClock Timer;

    public:
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Engine.");
            GRuntime    ->Bootstrap();

            GEvent      ->Bootstrap();
            GScripting  ->Bootstrap();
            GAssetHub   ->Bootstrap();
            GRender     ->Bootstrap();
            GAudio      ->Bootstrap();

            OnBootstrap.Broadcast();

            Status = EStatus::Bootstrapped;
        }


        void Terminate() override
        {
            LOG_TRACE("Terminating Engine.");
            OnTerminate.Broadcast();

            GAudio      ->Terminate();
            GRender     ->Terminate();
            GAssetHub   ->Terminate();
            GScripting  ->Terminate();
            GEvent      ->Terminate();

            GRuntime    ->Terminate();

            Status = EStatus::Terminated;
        }

        FEngine() : IGlobalSingleton("Engine") {};
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEngine>
    GEngine = MakeUnique<FEngine>();
}
