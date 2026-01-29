module;
#include <Visera.hpp>

#include "Visera-Core.hpp"
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.RHI;
import Visera.Tasks;
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
                Configuration = FJSON(reinterpret_cast<const char*>(FileData.Data()));
                LOG_INFO("Config.json dump:\n{}", Configuration.Dump());
            }
            else
            {
                LOG_WARN("Failed to read config.json or file is empty");
            }
        }
        else { LOG_ERROR("Failed to open config.json"); }

        auto Textures = Configuration.GetTextArrayPath("Assets.Textures");
        auto TestImage = AssetHub->LoadImage(Textures[0]);
        Window->SetSize(TestImage->GetWidth(), TestImage->GetHeight());

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
        Tasks       = IGlobalService::Register<FTasks>(EName::Tasks);
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