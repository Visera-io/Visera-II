module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Platform;
import Visera.Runtime;
using namespace Visera;

struct FEngine
{
    TUniquePtr<FRuntime> Runtime;
    FHiResClock Timer;

    Bool Run()
    {
        auto Image = Runtime->AssetHub->LoadImage(FPlatform::GetResourceDirectory() / FPath{"Assets/App/Texture/Bronya.png"})->GetImage();


        LOG_INFO("Visera Engine Run()");

        //FRHICommandList Commands;
        FJSON Config = FJSON::Load(FPath{"Assets/App/Configs/config.json"}).GetValue();

        auto TexturePath = Config.GetPath("Assets.Textures[0]"_JQL);
        auto ImgAsset1 = Runtime->AssetHub->LoadImage(TexturePath);
        auto ImgAsset2 = Runtime->AssetHub->LoadImage(TexturePath);
        auto ImgAsset3 = Runtime->AssetHub->LoadImage(TexturePath);
        if (!ImgAsset1) { LOG_ERROR("Failed to load test image"); return 1; }
        const FImage& SrcImage = ImgAsset1->GetImage();

        FJSON Config2;
        Config2.Set("Window.Title"_JQL, "Runtime 2");

        auto Runtime2 = FRuntime::Create("Runtime2", EMode::Full, Config2);
        Runtime->Input->GetKeyboard()->OnPressed.Subscribe([](FKeyboard::EKey I_Key)
        {
            if (FKeyboard::EKey::Space == I_Key)
            {
                LOG_INFO("Runtime 1 Keyboard Space!");
            }
        });
        Runtime2->Input->GetKeyboard()->OnPressed.Subscribe([](FKeyboard::EKey I_Key)
        {
            if (FKeyboard::EKey::Space == I_Key)
            {
                LOG_INFO("Runtime 2 Keyboard Space!");
            }
        });

        while (!Runtime->Window->ShouldClose())
        {
            Runtime->Window->PollEvents();

            if (Runtime2 != nullptr)
            {
                if (!Runtime2->Window->ShouldClose())
                {
                    Runtime2->Window->PollEvents();
                }
                else Runtime2.Reset();
            }

            // Frame for PrimaryWindow (Runtime)
            if (!Runtime->RHI->BeginFrame()) { continue; }

            // Rendering
            {

            }

            Runtime->RHI->EndFrame();
            Runtime->RHI->Present();

            // Frame for Runtime2's window (each window needs its own BeginFrame/EndFrame/Present cycle)
            if (Runtime2 != nullptr)
            {
                if (Runtime->RHI->BeginFrame(Runtime2->Window.Get()))
                {
                    Runtime->RHI->EndFrame(Runtime2->Window.Get());
                    Runtime->RHI->Present(Runtime2->Window.Get());
                }
            }
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
        }
        
        Runtime = FRuntime::Create("MainRuntime", EMode::Full, RuntimeConfig);
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