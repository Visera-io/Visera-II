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
        ;
        Window->SetSize(512, 512);

        LOG_INFO("{}", FText{Math::PI});

        FRHICommandList Commands;

        auto ViseraImage = AssetHub->LoadImage("Assets/App/Texture/Visera.png");

        LOG_WARN("Timer Begin");
        for (FPixel& Pixel : ViseraImage->View())
        {
            FColor Color = Pixel.Get();
            FLinearColor LinearColor { Color.R, Color.G,Color.B,Color.A };
            LinearColor = FLinearColor
            {
             LinearColor.R * LinearColor.A,
             LinearColor.G * LinearColor.A,
             LinearColor.B * LinearColor.A,
             LinearColor.A
            };
            Pixel.Set(LinearColor);
        }
        LOG_WARN("Timer End");

        while (!Window->ShouldClose())
        {
            Window->PollEvents();

            if (!RHI->BeginFrame()) { continue; }
            Commands.Reset();

            //ViseraImage->Resize(1024, 1024);
            if(!ViseraImage->IsRGBA())
            { LOG_FATAL("Not RGBA!"); }
            auto Texture = RHI->CreateTexture({
                .Width      = ViseraImage->GetWidth(),
                .Height     = ViseraImage->GetHeight(),
                .Depth      = 1,
                .Format     =  ERHIFormat::R8G8B8A8_sRGB,
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
                    .Size   = ViseraImage->GetSizeInBytes(),
                    .Usages = ERHIBufferUsage::TransferSrc
                }, ViseraImage->GetData(), ViseraImage->GetSizeInBytes());

                Commands.CopyBufferToImage(Buffer, Texture);
                InitedTextures.Insert(Texture);
                RHI->DestroyBuffer(Buffer);
            }
            // Rendering
            auto Color = FRHIClearColor::Red();
            {
                auto Time = Timer.Elapsed().Milliseconds() / 1000.0;
                auto Pulse = Math::Pow(0.5f + 0.5f * Math::Sin(FRadian(Time)), 2.2f);
                Color.R *= Pulse;
                Color.G *= Pulse;
                Color.B *= Pulse;
            }
            Commands.ConvertImageLayout (Texture, ERHIImageLayout::TransferDst);


            //Commands.ClearColorImage    (Texture, Color);

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