module;
#include <Visera-Engine.hpp>
export module Visera.Engine;
#define VISERA_MODULE_NAME "Engine"
import Visera.Engine.Render;
import Visera.Engine.UI;
import Visera.Runtime;
import Visera.Core.Log;

namespace Visera
{
    class VISERA_ENGINE_API FEngine : public IGlobalSingleton
    {
    public:
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Engine.");

            GAssetHub->Bootstrap();
            GWindow->Bootstrap();
            GRHI->Bootstrap();
            GAudio->Bootstrap();
            GScene->Bootstrap();

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
            //auto Fence = GRHI->GetDriver()->CreateFence(True);

            auto VertModule = Driver->CreateShaderModule(GAssetHub->LoadShader(PATH("Skybox.slang"), "VertexMain"));
            auto FragModule = Driver->CreateShaderModule(GAssetHub->LoadShader(PATH("Skybox.slang"), "FragmentMain"));
            auto RenderPass = Driver->CreateRenderPass(TEXT("TestPass"), VertModule, FragModule);
            RenderPass->SetRenderArea({
                {0,0},
                {1920,1080}
            });

            // auto RHIImage = Driver->CreateImage(
            //     RHI::EImageType::e2D,
            //     {200, 400, 1},
            //     RHI::EFormat::eR8G8B8A8Unorm,
            //     RHI::EImageUsage::eColorAttachment
            //     );

            struct alignas(16) FTestPushConstantRange
            {
                Float Time{0};
                Float CursorX{0}, CursorY{0};
                Float OffsetX{0}, OffsetY{0};
            };
            static_assert(sizeof(FTestPushConstantRange) <= 128);
            FTestPushConstantRange MouseContext{};

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
                    auto& Cmd = Driver->GetCurrentFrame().DrawCalls;

                    Cmd->EnterRenderPass(RenderPass);
                    Cmd->PushConstants(RHI::EShaderStage::eFragment, &MouseContext, 0, sizeof MouseContext);
                    Cmd->Draw(3,1,0,0);
                    Cmd->LeaveRenderPass();

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

            GScene->Terminate();
            GAudio->Terminate();
            GRHI->Terminate();
            GWindow->Terminate();
            GAssetHub->Terminate();

            Status = EStatus::Terminated;
        }

        FEngine() : IGlobalSingleton("Engine") {};
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEngine>
    GEngine = MakeUnique<FEngine>();
}
