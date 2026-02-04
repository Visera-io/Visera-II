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
        struct Foo
        {
            ~Foo() { LOG_INFO("De Foo"); }
        };

        FJSON Config = FJSON::Load("Assets/App/Configs/config.json").GetValue();

        TSharedPtr<FImage> TestImage = AssetHub->LoadImage(FPath(Config.GetString(FJSONPath("Assets.Textures[0]"))));
        
        // Test LUT: Color inversion using sRGB to Linear conversion
        LOG_INFO("Testing LUT: Inverting colors in image ({}x{})", TestImage->GetWidth(), TestImage->GetHeight());
        {
            UInt32 ProcessedPixels = 0;
            for (auto& Pixel : TestImage->View())
            {
                // Get pixel color (RAW read - for sRGB images, Get() returns sRGB values as 0-1 floats)
                FLinearColor PixelColor = Pixel.Get();
                
                // Convert to sRGB UInt8 values if image is sRGB format
                if (TestImage->GetPixelFormat() == EPixelFormat::RGBA8_UNorm)
                {
                    // Extract sRGB UInt8 values
                    UInt8 SRGB_R = static_cast<UInt8>(Math::Clamp(PixelColor.R * 255.0f, 0.0f, 255.0f));
                    UInt8 SRGB_G = static_cast<UInt8>(Math::Clamp(PixelColor.G * 255.0f, 0.0f, 255.0f));
                    UInt8 SRGB_B = static_cast<UInt8>(Math::Clamp(PixelColor.B * 255.0f, 0.0f, 255.0f));
                    UInt8 SRGB_A = static_cast<UInt8>(Math::Clamp(PixelColor.A * 255.0f, 0.0f, 255.0f));
                    
                    // Convert sRGB to Linear using LUT (via FLinearColor constructor)
                    FLinearColor LinearColor(SRGB_R, SRGB_G, SRGB_B, SRGB_A);
                    
                    // Invert color in Linear space: (1.0 - color)
                    LinearColor.R = 1.0f - LinearColor.R;
                    LinearColor.G = 1.0f - LinearColor.G;
                    LinearColor.B = 1.0f - LinearColor.B;
                    // Keep alpha unchanged
                    
                    // Convert back to sRGB
                    FColor InvertedSRGB = FColor::SRGB8ColorFromLinear(LinearColor);
                    
                    // Set pixel (RAW write - expects sRGB values as 0-1 floats for sRGB images)
                    FLinearColor SRGBAsLinear;
                    SRGBAsLinear.R = InvertedSRGB.R / 255.0f;
                    SRGBAsLinear.G = InvertedSRGB.G / 255.0f;
                    SRGBAsLinear.B = InvertedSRGB.B / 255.0f;
                    SRGBAsLinear.A = InvertedSRGB.A / 255.0f;
                    Pixel.Set(SRGBAsLinear);
                }
                else
                {
                    // For Linear formats, directly invert in Linear space
                    PixelColor.R = 1.0f - PixelColor.R;
                    PixelColor.G = 1.0f - PixelColor.G;
                    PixelColor.B = 1.0f - PixelColor.B;
                    Pixel.Set(PixelColor);
                }
                
                ProcessedPixels++;
            }
            LOG_INFO("LUT test completed: Processed {} pixels", ProcessedPixels);
        }
        
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