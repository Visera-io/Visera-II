module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.RHI;
import Visera.Audio;
import Visera.Global;
import Visera.Platform;
import Visera.Graphics;
import Visera.Assets.Image;
using namespace Visera;

struct FEngine
{
    FPlatform* Platform;
    FInput*    Input;
    FWindow*   Window;
    FRHI*      RHI;
    FAudio*    Audio;
    FGraphics* Graphics;

    Bool Run()
    {
        LOG_INFO("Visera Engine Run()");

        FRHICommandList Commands;
        while (!Window->ShouldClose())
        {
            Window->PollEvents();
            Sleep(0.5);

            if (!RHI->BeginFrame()) { continue; }
            Commands.Reset();

            auto Texture = RHI->CreateTexture({
            .Width = 1920,
            .Height = 1080,
            .Depth = 1,
            .Format = ERHIFormat::R8G8B8A8_UNorm,
            .Type =  ERHIImageType::Image2D,
            .Usages = ERHIImageUsage::RenderTarget | ERHIImageUsage::TransferSrc | ERHIImageUsage::TransferDst,
            .ViewType = ERHIImageViewType::Image2D,}
            );

            // Rendering

            RHI->DestroyTexture(Texture);
            Commands.ConvertImageLayout(Texture, ERHIImageLayout::TransferDst);
            Commands.ClearColorImage   (Texture, FRHIClearColor::Green());
            Commands.ConvertImageLayout(Texture, ERHIImageLayout::ColorAttachment);
            for (auto Command : Commands)
            {
                LOG_INFO("Visera Engine Command :{} ", Command.Type);
            }

            RHI->Submit(Commands);

            RHI->EndFrame();
            RHI->Present();
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