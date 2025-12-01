module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Demos;
import Visera.Core;
import Visera.Runtime;
import Visera.Engine;
import Visera.Studio;
using namespace Visera;

struct Foo
{
    using FOnInit = TDelegate<FString, FString>;
    FOnInit OnInit;
};

export int main(int argc, char *argv[])
{
    FVector3F Vec {1,1,1};

    if (Vec.Normalize())
    {
        LOG_WARN("Vector = {} | L2 Norm: {}", Vec, Vec.L2Norm());
    }
    LOG_WARN("{}", FMatrix4x4F::Identity());
    /*LOG_WARN("Practicing C#.");
    {
        GPlatform->Bootstrap();
        GScripting->Bootstrap();
        GAssetHub->Bootstrap();

        if (auto AppMain = GScripting->GetFunction(PLATFORM_STRING("Main")))
        {
            AppMain(nullptr, 0);
        }
        else LOG_FATAL("Failed to load the \"Main\"!");

        GAssetHub->Terminate();
        GScripting->Terminate();
        GPlatform->Terminate();
        GLog->Terminate();
        return 0;
    }*/

    GEngine->Bootstrap();
    {
        GStudio->Bootstrap();

        //auto BankInit = GAssetHub->LoadSound(FPath("Init.bnk"));
        //auto MainBGM = GAssetHub->LoadSound(FPath("Test.bnk"));

        //GAudio->Register(BankInit);
        //auto ID = GAudio->Register(MainBGM);
        //GAudio->PostEvent("Play_MainBGM", ID);

        if (auto AppTick = GScripting->GetFunction(PLATFORM_STRING("Tick")))
        {
            if (!GEngine->AppTick.TryBind([AppTick](Float I_DeltaTime)
            {
                AppTick(&I_DeltaTime, sizeof(Float));
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