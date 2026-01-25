module;
#include <Visera.hpp>

#include "Visera-Core.hpp"
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.RHI;
import Visera.Audio;
import Visera.Global;
import Visera.Platform;
import Visera.Graphics;
import Visera.AssetHub;
using namespace Visera;

// ACES tone mapping implementation
[[nodiscard]] static FLinearColor
TonemapACESFitted(const FLinearColor& I_Color)
{
    // ACES Input Matrix (sRGB to ACES working space)
    static constexpr FMatrix3x3F ACESInputMat(
        0.59719f, 0.35458f, 0.04823f,
        0.07600f, 0.90834f, 0.01566f,
        0.02840f, 0.13383f, 0.83777f
    );
    
    // ACES Output Matrix (ACES to display space)
    static constexpr FMatrix3x3F ACESOutputMat(
         1.60475f, -0.53108f, -0.07367f,
        -0.10208f,  1.10813f, -0.00605f,
        -0.00327f, -0.07276f,  1.07602f
    );
    
    // RRT and ODT fit function
    auto RRTAndODTFit = [](Float V) -> Float
    {
        const Float A = V * (V + 0.0245786f) - 0.000090537f;
        const Float B = V * (0.983729f * V + 0.4329510f) + 0.238081f;
        return A / B;
    };
    
    // Extract RGB as vector
    FVector3F ColorVec(I_Color.R, I_Color.G, I_Color.B);
    
    // Apply ACES Input Matrix
    ColorVec = ACESInputMat * ColorVec;
    
    // Apply RRT and ODT fit
    ColorVec.X = RRTAndODTFit(ColorVec.X);
    ColorVec.Y = RRTAndODTFit(ColorVec.Y);
    ColorVec.Z = RRTAndODTFit(ColorVec.Z);
    
    // Apply ACES Output Matrix
    ColorVec = ACESOutputMat * ColorVec;
    
    // Clamp to valid range and preserve alpha
    return FLinearColor(
        Math::Clamp(ColorVec.X, 0.0f, 1.0f),
        Math::Clamp(ColorVec.Y, 0.0f, 1.0f),
        Math::Clamp(ColorVec.Z, 0.0f, 1.0f),
        I_Color.A
    );
}

struct FEngine
{
    FPlatform* Platform;
    FInput*    Input;
    FWindow*   Window;
    FRHI*      RHI;
    FAudio*    Audio;
    FGraphics* Graphics;
    FAssetHub* AssetHub;

    FHiResClock Timer;

    Bool Run()
    {
        LOG_INFO("Visera Engine Run()");

        LOG_INFO("{}", FText{Math::PI});

        FRHICommandList Commands;

        auto TestImage = AssetHub->LoadImage("Assets/App/Texture/Carrots.exr");
        Window->SetSize(TestImage->GetWidth(), TestImage->GetHeight());
        
        // Apply ACES tone mapping to HDR EXR image for display
        LOG_INFO("Applying ACES tone mapping to EXR image...");
        for (auto& Pixel : TestImage->View())
        {
            auto PixelColor = Pixel.Get();
            // Apply ACES tone mapping to convert HDR to displayable range
            auto TonemappedColor = TonemapACESFitted(PixelColor);
            Pixel.Set(TonemappedColor);
        }
        LOG_INFO("ACES tone mapping complete.");

        auto TestSampler = RHI->CreateSampler(FRHISamplerCreateDesc
            {
                .Type        = ERHISamplerType::Linear,
                .AddressMode = ERHISamplerAddressMode::Repeat,
            });
        RHI->DestroySampler(TestSampler);

        while (!Window->ShouldClose())
        {
            Window->PollEvents();

            if (!RHI->BeginFrame()) { continue; }
            Commands.Reset();
            // for (FPixel& Pixel : TestImage->View())
            // {
            //     FColor Color = Pixel.Get();
            //     FLinearColor LinearColor { Color.R, Color.G,Color.B,Color.A };
            //     LinearColor = FLinearColor
            //     {
            //         LinearColor.R * LinearColor.A,
            //         LinearColor.G * LinearColor.A,
            //         LinearColor.B * LinearColor.A,
            //         LinearColor.A
            //        };
            //     Pixel.Set(LinearColor);
            // }
            //TestImage->Resize(1024, 1024);
            auto RHIFormat = ERHIFormat::Undefined;
            switch (TestImage->GetPixelFormat())
            {
            case EPixelFormat::RGBA8_UNorm:
                if (TestImage->GetColorSpace() == EColorSpace::sRGB)
                {
                    RHIFormat = ERHIFormat::R8G8B8A8_sRGB;
                }
                else
                {
                    RHIFormat = ERHIFormat::R8G8B8A8_UNorm;
                }
                break;
            case EPixelFormat::RGBA16_Float:
                RHIFormat = ERHIFormat::R16G16B16A16_Float;
                break;
            default: LOG_FATAL("Unknown pixel format!");
            }


            if(!TestImage->IsRGBA())
            { LOG_FATAL("Not RGBA!"); }
            auto Texture = RHI->CreateTexture({
                .Width      = TestImage->GetWidth(),
                .Height     = TestImage->GetHeight(),
                .Depth      = 1,
                .Format     =  RHIFormat,
                .Type       =  ERHIImageType::Image2D,
                .Usages = ERHIImageUsage::ShaderResource |
                          ERHIImageUsage::TransferSrc  |
                          ERHIImageUsage::TransferDst,
                .ViewType = ERHIImageViewType::Image2D,}
            );
            static TSet<FRHITextureHandle> InitedTextures;
            if (!InitedTextures.Contains(Texture))
            {
                LOG_INFO("Copying Buffer to Image");

                auto Buffer = RHI->CreateBuffer({
                    .Size   = TestImage->GetSizeInBytes(),
                    .Usages = ERHIBufferUsage::TransferSrc
                }, TestImage->GetData(), TestImage->GetSizeInBytes());

                Commands.CopyBufferToImage(Buffer, Texture);
                InitedTextures.Insert(Texture);
                RHI->DestroyBuffer(Buffer);
            }
            // Rendering
            {

            }

            Commands.ConvertImageLayout (Texture, ERHIImageLayout::TransferDst);

            Commands.ConvertImageLayout (Texture, ERHIImageLayout::TransferSrc);
            Commands.BlitToSwapChain    (Texture, ERHIFilter::Nearest);
            Commands.ConvertImageLayout (Texture, ERHIImageLayout::ShaderReadOnly);

            // for (auto Command : Commands)
            // {
            //     LOG_INFO("Visera Engine Command :{} ", Command.Type);
            // }

            RHI->Submit(Commands);

            RHI->EndFrame();
            RHI->Present();

            RHI->DestroyTexture(Texture);
        }

        return EXIT_SUCCESS;
    }

    FEngine()
    {
        LOG_INFO("Visera Engine");
        Platform    = IGlobalService::Register<FPlatform>(EName::Platform);
        Input       = IGlobalService::Register<FInput>(EName::Input);
        Window      = IGlobalService::Register<FWindow>(EName::Window);
        RHI         = IGlobalService::Register<FRHI>(EName::RHI);
        Audio       = IGlobalService::Register<FAudio>(EName::Audio);
        Graphics    = IGlobalService::Register<FGraphics>(EName::Graphics);
        AssetHub    = IGlobalService::Register<FAssetHub>(EName::AssetHub);

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