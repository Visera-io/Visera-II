module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Runtime;
using namespace Visera;

struct FEngine
{
    TUniquePtr<FRuntime> Runtime;
    FHiResClock Timer;

    Bool Run()
    {
        Runtime->Input->GetMouse()->OnPressed.Subscribe([](FMouse::EButton I_Button)
        {
            if (I_Button == FMouse::EButton::Left)
            {
                LOG_INFO("Left");
            }
        });

        LOG_INFO("Visera Engine Run()");

        FRHICommandList Commands;
        FJSON Config = FJSON::Load(FPath{"Assets/App/Configs/config.json"}).GetValue();

        auto TexturePath = Config.GetPath("Assets.Textures[0]"_JQL);
        auto ImgAsset1 = Runtime->AssetHub->LoadImage(TexturePath);
        auto ImgAsset2 = Runtime->AssetHub->LoadImage(TexturePath);
        auto ImgAsset3 = Runtime->AssetHub->LoadImage(TexturePath);
        if (!ImgAsset1) { LOG_ERROR("Failed to load test image"); return 1; }
        const FImage& SrcImage = ImgAsset1->GetImage();

        while (!Runtime->Window->ShouldClose())
        {
            Runtime->Window->PollEvents();

            if (!Runtime->RHI->BeginFrame()) { continue; }

            Runtime->Graphics->Tick(0);

            Commands.Reset();

            // Rendering
            {

            }

            //Runtime->RHI->Submit(Commands);

            Runtime->RHI->EndFrame();
            Runtime->RHI->Present();
        }

        return EXIT_SUCCESS;
    }

    FEngine()
    {
        LOG_INFO("Visera Engine");
        
        // Load runtime config file
        TOptional<FJSON> RuntimeConfig;
        if (auto ConfigFile = FJSON::Load(FPlatform::GetResourceDirectory() / FPath{"Engine/Config.runtime.json"}); ConfigFile.HasValue())
        {
            RuntimeConfig = std::move(ConfigFile).GetValue();
            LOG_DEBUG("Runtime config loaded: {}", RuntimeConfig.GetValue().Dump());
        }
        else
        {
            LOG_WARN("Failed to load config file Engine/Config.runtime.json -- using default config!");
        }
        
        Runtime = FRuntime::Create(EMode::Full, RuntimeConfig);
        if (!Runtime)
        {
            LOG_FATAL("Failed to create FRuntime!");
        }
    }
    ~FEngine()
    {
        LOG_INFO("~Visera Engine");
        // Runtime will be destroyed automatically, which calls Terminate()
    }
};

export int main(int argc, char *argv[])
{
    return FEngine{}.Run();
}