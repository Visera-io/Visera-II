module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.RHI;
import Visera.Tasks;
import Visera.Audio;
import Visera.Global;
import Visera.Platform;
import Visera.Graphics;
import Visera.Graphics.Renderer;
import Visera.AssetHub;
using namespace Visera;
import nlohmann.json;

struct FEngine
{
    FPlatform* Platform;
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
        TMPSCQueue<Int32> MPSCQueue;
        MPSCQueue.Enqueue(1);
        MPSCQueue.Enqueue(2);
        LOG_INFO("{}", MPSCQueue.Dequeue().GetValue());
        LOG_INFO("{}", MPSCQueue.Dequeue().GetValue());
        TSPSCQueue<Float> SPSCQueue;
        SPSCQueue.Enqueue(1);
        SPSCQueue.Enqueue(2);
        LOG_INFO("{}", SPSCQueue.Dequeue().GetValue());
        LOG_INFO("{}", SPSCQueue.Dequeue().GetValue());

        TOptional<Int32> Num = 32;
        LOG_ERROR("{}", Num.GetValue());

        LOG_INFO("{}", FText(True));
        LOG_INFO("{}", FText(False));
        FString MyString = "A.B.C";
        for (auto View : MyString.SplitToViews(".B"))
        {
            LOG_INFO("{}", View);
        }
        LOG_WARN("{}", MyString);

        LOG_INFO("Visera Engine Run()");

        FRHICommandList Commands;

        FJSON Config = FJSON::Load("Assets/App/Configs/config.json").GetValue();

        TSharedPtr<FImage> TestImage = AssetHub->LoadImage(FPath(Config.GetString(FJSONPath("Assets.Textures[0]"))));
        FTextureID TexID = Graphics->CreateTexture2D(TestImage);

        Window->SetSize(TestImage->GetWidth(), TestImage->GetHeight());

        auto TestTexture = Graphics->GetTexture2D(TexID);

        auto Material = MakeShared<FMaterial>(FJSON::Load("Assets/App/Material/BasicSprite.vmaterial").GetValue());
        if (Material->IsValid())
        {
            const char* SurfaceName = "Unknown";
            switch (Material->GetSurface())
            {
            case ESurfaceType::Opaque:      SurfaceName = "Opaque";      break;
            case ESurfaceType::Masked:     SurfaceName = "Masked";      break;
            case ESurfaceType::Transparent: SurfaceName = "Transparent"; break;
            }
            LOG_INFO("[Material] Version={} Shader=\"{}\" Surface={} BaseColorPath=\"{}\"",
                     Material->GetVersion(),
                     Material->GetShader(),
                     SurfaceName,
                     Material->GetBaseColorPath());
        }
        else { LOG_WARN("[Material] Load failed or invalid (check Assets/App/Material/BasicSprite.vmaterial)"); }

        Material->SetBaseColorHandle(TexID);

        FSpriteRenderer Sprite;
        Sprite.SetMaterial(Material);

        while (!Window->ShouldClose())
        {
            Window->PollEvents();

            if (!RHI->BeginFrame()) { continue; }

            Graphics->Tick(0);

            Commands.Reset();

            // Rendering
            {

            }
            Commands.ConvertImageLayout(TestTexture->TextureHandle, ERHIImageLayout::TransferSrc);
            Commands.BlitToSwapChain(TestTexture->TextureHandle, ERHIFilter::Linear);
            Commands.ConvertImageLayout(TestTexture->TextureHandle, ERHIImageLayout::ShaderReadOnly);

            RHI->Submit(Commands);

            RHI->EndFrame();
            RHI->Present();
        }

        return EXIT_SUCCESS;
    }

    FEngine()
    {
        LOG_INFO("Visera Engine");
        Platform = IGlobalService::Register<FPlatform> (EName::Platform);
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