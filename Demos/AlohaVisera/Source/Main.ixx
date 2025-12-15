module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Game;
using namespace Visera;

export int main(int argc, char *argv[])
{
    GEngine->Bootstrap();
    {
        auto BankInit = GAssetHub->LoadSound(FPath("Init.bnk"));
        auto MainBGM = GAssetHub->LoadSound(FPath("Test.bnk"));

        GAudio->Register(BankInit);
        auto ID = GAudio->Register(MainBGM);
        GAudio->PostEvent("Play_Advanture", ID);

        auto Entity = GWorld->CreateEntity(FName{"player_0"});
        auto& Transform = Entity.Add<CTransform2D>();
        auto& Velocity  = Entity.Add<CVelocity2D>();

        GEngine->Run();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}