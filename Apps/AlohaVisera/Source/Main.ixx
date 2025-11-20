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

        auto Handle = GInput->GetKeyboard()->OnPressed.Subscribe(
        [](FKeyboard::EKey I_Key)
        {
            LOG_INFO("Pressed {}", static_cast<Int32>(I_Key));
        });
        GEvent->OnFrameBegin.Subscribe([]()
        {
           if (GInput->GetKeyboard()->GetKey(FKeyboard::EKey::A)
               .IsPressed())
           {
               LOG_INFO("A");
           }
        });

        auto BankInit = GAssetHub->LoadSound(PATH("Init.bnk"));
        auto MainBGM = GAssetHub->LoadSound(PATH("Test.bnk"));

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