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

        auto BankInit = GAssetHub->LoadSound(FPath("Init.bnk"));
        auto MainBGM = GAssetHub->LoadSound(FPath("Test.bnk"));
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
                GStudio->UI->Slider("Velocity X", &Velocity.X, 0.0, 100.0);
                GStudio->UI->Slider("Velocity Y", &Velocity.Y, 0.0, 100.0);
            }
        };

        GAudio->Register(BankInit);
        auto ID = GAudio->Register(MainBGM);
        GAudio->PostEvent("Play_MainBGM", ID);

        if (auto AppTick = GScripting->GetFunction(PLATFORM_STRING("Tick")))
        {
            if (!GEngine->AppTick.TryBind([AppTick, DebugTick](Float I_DeltaTime)
            {
                AppTick(&I_DeltaTime, sizeof(Float));
                DebugTick();
            }))
            { LOG_FATAL("Failed to bind the AppTick()!"); }
        }
        else LOG_FATAL("Failed to load the \"AppTick()\" from .NET runtime!");

        GEngine->Run();

        GStudio->Terminate();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}