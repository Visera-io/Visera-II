module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Demos;
import Visera.Core;
import Visera.Runtime;
import Visera.Engine;
import Visera.Engine.Scripting.DotNET;
import Visera.Studio;
using namespace Visera;

export int main(int argc, char *argv[])
{
    GLog->Bootstrap();

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

        auto BankInit = GAssetHub->LoadSound(PATH("Init.bnk"));
        auto MainBGM = GAssetHub->LoadSound(PATH("Test.bnk"));

        GAudio->Register(BankInit);
        auto ID = GAudio->Register(MainBGM);
        GAudio->PostEvent("Play_Galaxy", ID);

        if (auto AppMain = GScripting->GetFunction(PLATFORM_STRING("Main")))
        {
            AppMain(nullptr, 0);
        }
        else LOG_FATAL("Failed to load the \"Main\"!");

        GEngine->Run();
        GStudio->Terminate();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}