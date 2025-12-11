module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
import Visera.Engine;
import Visera.Studio;
using namespace Visera;

export int main(int argc, char *argv[])
{
    GEngine->Bootstrap();
    {
        GStudio->Bootstrap();

        //auto BankInit = GAssetHub->LoadSound(FPath("Init.bnk"));
        //auto MainBGM = GAssetHub->LoadSound(FPath("Test.bnk"));
        auto Entity = GWorld->CreateEntity(FName{"player_0"});
        auto& Transform = Entity.Add<CTransform2D>();
        auto& Velocity  = Entity.Add<CVelocity2D>();

        auto DebugTick = [&]()
        {
            if (auto W = GStudio->UI->Window("Debug Pannel"))
            {
                GStudio->UI->Text(Format("Transform:\n T:{}\nR:{}\nS:{}",
                    Transform.Translation,
                    Transform.Rotation,
                    Transform.Scale));
                GStudio->UI->Slider("Velocity X", &Velocity.X, -1.0, 1.0);
                GStudio->UI->Slider("Velocity Y", &Velocity.Y, -1.0, 1.0);
            }
        };

        //GAudio->Register(BankInit);
        //auto ID = GAudio->Register(MainBGM);
        //GAudio->PostEvent("Play_MainBGM", ID);

        GEngine->Run();

        GStudio->Terminate();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}