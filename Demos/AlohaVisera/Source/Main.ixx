module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.RHI;
import Visera.Tasks;
import Visera.Audio;
import Visera.Global;
import Visera.Window;
import Visera.Input;
import Visera.Graphics;
import Visera.Graphics.Renderer;
import Visera.AssetHub;
using namespace Visera;

struct FEngine
{
    FInput*    Input;
    FWindow*   Window;
    FTasks*    Tasks;
    FRHI*      RHI;
    FAudio*    Audio;
    FGraphics* Graphics;
    FAssetHub* AssetHub;

    FHiResClock Timer;

    Bool Run()
    {
        Input->GetMouse()->OnPressed.Subscribe([](FMouse::EButton I_Button)
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
        TSharedPtr<FImageAsset> ImgAsset1 = AssetHub->LoadImage(TexturePath);
        TSharedPtr<FImageAsset> ImgAsset2 = AssetHub->LoadImage(TexturePath);
        TSharedPtr<FImageAsset> ImgAsset3 = AssetHub->LoadImage(TexturePath);
        if (!ImgAsset1) { LOG_ERROR("Failed to load test image"); return 1; }
        const FImage& SrcImage = ImgAsset1->GetImage();

        while (!Window->ShouldClose())
        {
            Window->PollEvents();

            if (!RHI->BeginFrame()) { continue; }

            Graphics->Tick(0);

            Commands.Reset();

            // Rendering
            {

            }

            //RHI->Submit(Commands);

            RHI->EndFrame();
            RHI->Present();
        }

        return EXIT_SUCCESS;
    }

    FEngine()
    {
        LOG_INFO("Visera Engine");
        Input    = IGlobalService::Register<FInput>    (EName::Input);
        Window   = IGlobalService::Register<FWindow>   (EName::Window);
        Tasks    = IGlobalService::Register<FTasks>    (EName::Tasks);
        RHI      = IGlobalService::Register<FRHI>      (EName::RHI);
        Audio    = IGlobalService::Register<FAudio>    (EName::Audio);
        Graphics = IGlobalService::Register<FGraphics> (EName::Graphics);
        AssetHub = IGlobalService::Register<FAssetHub> (EName::AssetHub);

        IGlobalService::Bootstrap();
    }
    ~FEngine()
    {
        IGlobalService::Terminate();
        LOG_INFO("~Visera Engine");
    }
};

export int main(int argc, char *argv[])
{
    return FEngine{}.Run();
}