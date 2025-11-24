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

        auto BankInit = GAssetHub->LoadSound(FPath("Init.bnk"));
        auto MainBGM = GAssetHub->LoadSound(FPath("Test.bnk"));

        GAudio->Register(BankInit);
        auto ID = GAudio->Register(MainBGM);
        GAudio->PostEvent("Play_Galaxy", ID);

        if (auto AppTick = GScripting->GetFunction(PLATFORM_STRING("Tick")))
        {
            GEngine->AppTick.Bind([AppTick](Float I_DeltaTime)
            {
                AppTick(&I_DeltaTime, sizeof(Float));
            });
        }
        else LOG_FATAL("Failed to load the \"Main\"!");

        GEngine->Run();
        GStudio->Terminate();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}