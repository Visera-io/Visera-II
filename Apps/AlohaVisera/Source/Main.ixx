module;
#include <Visera.hpp>
#include "Visera-Runtime.hpp"
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Demos;
import Visera.Core;
import Visera.Runtime;
import Visera.Engine;
import Visera.Runtime.Scene;
import Visera.Runtime.AssetHub.Font.FreeType;
using namespace Visera;

class Foo : public VObject
{
public:
    void Awake() override { LOG_INFO("Wake up Foo!"); }
    void Update(Float I_FrameTime) override { LOG_INFO("Update Foo {}!", I_FrameTime); }

    void Bar() {
        auto k = shared_from_this();
    }
};

export int main(int argc, char *argv[])
{

    FHiResClock Clock{};
    for (int i = 0; i < 1; ++i)
    {
        auto Instance = GScene->CreateObject<Foo>(FName(Format("Foo_{}", i)));
        LOG_DEBUG("[{}] {}", Instance->GetID(), Instance->GetName());
        Instance->Update(1.02);
    }
    LOG_DEBUG("Test Time: {}ms", Clock.Elapsed().Milliseconds());

    //Demos
    //{ Demos::Compression Demo; }

    FFreeType Ft{};

    GEngine->Bootstrap();
    {
        auto& Driver = GRHI->GetDriver();
        //auto Fence = GRHI->GetDriver()->CreateFence(True);

        auto Image = GAssetHub->LoadImage(PATH("Visera.png"));
        // if (Image->Save())
        // {
        //     LOG_INFO("Successfully saved!");
        // }

        auto VertModule = Driver->CreateShaderModule(GAssetHub->LoadShader(PATH("Skybox.slang"), "VertexMain"));
        auto FragModule = Driver->CreateShaderModule(GAssetHub->LoadShader(PATH("Skybox.slang"), "FragmentMain"));
        auto RenderPass = Driver->CreateRenderPass(VertModule, FragModule);
        RenderPass->SetRenderArea({{0,0},{GWindow->GetWidth(),GWindow->GetHeight()}});

        auto Cmd = Driver->CreateCommandBuffer(RHI::EQueue::eGraphics);
        // auto RHIImage = Driver->CreateImage(
        //     RHI::EImageType::e2D,
        //     {200, 400, 1},
        //     RHI::EFormat::eR8G8B8A8Unorm,
        //     RHI::EImageUsage::eColorAttachment
        //     );

        Cmd->Begin();
        {
            LOG_INFO("Beginning!");
            // Cmd->EnterRenderPass(RenderPass);
            // Cmd->Draw(3,1,0,0);
            // Cmd->LeaveRenderPass();
        }
        Cmd->End();
        auto Fence = Driver->CreateFence(False);
        Driver->Submit(Cmd, Fence);
        if (!Fence->Wait(10000))
        {
            LOG_ERROR("Fence->Wait");
        }

        //LOG_FATAL("Exited for Development!");

        //GEngine->Run();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}