module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Game;
import Visera.RHI;
import Visera.Platform;
import Visera.Assets.Image;
using namespace Visera;

export int main(int argc, char *argv[])
{
    //GEngine->Bootstrap();
    {
        if (GWindow->IsBootstrapped())
        {
            FImage AppIcon { GPlatform->GetResourceDirectory() / FPath{"Assets/Engine/Image/Visera.png"} };
            GWindow->SetIcon(AppIcon.Access(), AppIcon.GetWidth(), AppIcon.GetHeight());
        }
        //
        // auto BankInit = GAssetHub->LoadSound(FPath("Init.bnk"));
        // auto MainBGM = GAssetHub->LoadSound(FPath("Test.bnk"));
        //
        // GAudio->Register(BankInit);
        // auto ID = GAudio->Register(MainBGM);
        // GAudio->PostEvent("Play_Advanture", ID);
        //
        // auto Entity = GWorld->CreateEntity(FName{"player_0"});
        // auto& Transform = Entity.Add<CTransform2D>();
        // auto& Velocity  = Entity.Add<CVelocity2D>();

        // (void)GRHI->CreateTexture({
        //     .Width  = 100,
        //     .Height = 100,
        //     .Depth  = 1,
        //     .Format = ERHIFormat::R16G16B16A16_Float,
        //     .Type   = ERHIImageType::Image2D,
        //     .Usages = ERHIImageUsage::TransferSrc | ERHIImageUsage::TransferDst
        // });

        GEngine->Run();
    }
    //GEngine->Terminate();
    return EXIT_SUCCESS;
}