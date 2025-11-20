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

        Foo foo;
        foo.OnInit.Bind([](FString I_Mes, FString I_Name)
        {
            LOG_INFO("{} -- {}", I_Mes, I_Name);
        });
        foo.OnInit.Invoke("Hello World", "LJYC");
        TMulticastDelegate<> M;
        for (int i = 0; i < 10; ++i)
        {
            M.Subscribe([i, &M]()
            {
                LOG_INFO("Sub {}", i);
                M.Subscribe([i, &M]()
                {
                    LOG_INFO("Cas Sub {}", i);
                });
            });
        }
        M.Broadcast();

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