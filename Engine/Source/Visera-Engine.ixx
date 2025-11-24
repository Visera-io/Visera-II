module;
#include <Visera-Engine.hpp>
export module Visera.Engine;
#define VISERA_MODULE_NAME "Engine"
export import Visera.Engine.AssetHub;
export import Visera.Engine.Audio;
export import Visera.Engine.Render;
export import Visera.Engine.Scripting;
export import Visera.Engine.Event;
export import Visera.Engine.World;
export import Visera.Engine.UI;
       import Visera.Core.Types.Name;
       import Visera.Core.Delegate;
       import Visera.Core.OS.Time;
       import Visera.Core.Global;
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
//
//             auto PipelineLayout = Driver->CreatePipelineLayout({},
// {{ EShaderStage::eFragment, 0U, UInt32(sizeof MouseContext) }});
//             auto RenderPass = GRHI->CreateRenderPipeline(
//              "TestPass",
//              GAssetHub->LoadShader(FPath("Skybox.slang"), "VertexMain")->GetSPIRVCode(),
//              GAssetHub->LoadShader(FPath("Skybox.slang"), "FragmentMain")->GetSPIRVCode(),
//              PipelineLayout
//             );
//              RenderPass->SetRenderArea({
//                  {0,0},
//                  {1920, 1080},
//              });

             while (!GWindow->ShouldClose())
             {
                 GWindow->PollEvents();
                 GAudio->Tick();

                 Float DeltaTime = Timer.Tick().Microseconds() / 1000000.0; Timer.Reset();

                 GRHI->BeginFrame();
                 {
                     GEvent->OnFrameBegin.Broadcast();
                     // auto& Cmd = GRHI->GetDrawCommands();
                     //
                     // Cmd->EnterRenderPass(RenderPass);
                     //  Cmd->PushConstants(EShaderStage::eFragment, &MouseContext, 0, sizeof MouseContext);
                     //  Cmd->Draw(3,1,0,0);
                     //  Cmd->LeaveRenderPass();

                     AppTick.Invoke(DeltaTime);
                     GRender->Tick(DeltaTime);

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
