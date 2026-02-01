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
        LOG_INFO("Visera Engine Run()");

        FRHICommandList Commands;

        FJSON Configuration{};
        auto File = FFileSystem::OpenFile("Assets/App/Configs/config.json", EIOMode::Read);
        if (File && File->IsOpen())
        {
            TArray<FByte> FileData = File->ReadAll();
            if (!FileData.IsEmpty())
            {
                Configuration = FJSON(FileData);
                LOG_INFO("\n{}", Configuration.Dump());
            }
            else
            {
                LOG_WARN("Failed to read config.json or file is empty");
            }
        }
        else { LOG_ERROR("Failed to open config.json"); }

        auto TexturePaths = Configuration.GetTextArrayPath("Assets.Textures");
        TSharedPtr<FImage> TestImage;
        FTextureID TexID;

        FSeedPool SeedPool;
        FPCG32 PCG{};
        PCG.SetSequence(0, SeedPool.Get());
        UInt32 Idx = Math::Round(PCG.Uniform() * TexturePaths.GetSize());
        LOG_INFO("{}", Idx);

        FEvent TextureUpload;
        Tasks->Enqueue([&]
        {
            TestImage = AssetHub->LoadImage(TexturePaths[Idx]);
            TexID = Graphics->CreateTexture2D(TestImage);
            TextureUpload.Trigger();
        });
        TextureUpload.Wait();
        Window->SetSize(TestImage->GetWidth(), TestImage->GetHeight());

        auto TestTexture = Graphics->GetTexture2D(TexID);

        auto Material = MakeShared<FMaterial>("Assets/App/Material/BasicSprite.vmaterial");
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