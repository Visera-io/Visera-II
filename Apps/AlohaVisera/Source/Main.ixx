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

class Foo : public VObject
{
public:
    void Awake() override { LOG_INFO("Wake up Foo!"); }
    void Update(Float I_FrameTime) override { LOG_INFO("Update Foo {}!", I_FrameTime); }

    void Bar() {
        auto k = shared_from_this();
    }
};

export int main(int argc, char *argv[])
{
    GLog->Bootstrap();

    LOG_WARN("Practicing C#.");
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
    }

    GEngine->Bootstrap();
    {
        GStudio->Bootstrap();

        auto BankInit = GAssetHub->LoadSound(PATH("Init.bnk"));
        auto MainBGM = GAssetHub->LoadSound(PATH("Test.bnk"));

        GAudio->Register(BankInit);
        auto ID = GAudio->Register(MainBGM);
        GAudio->PostEvent("Play_Galaxy", ID);

        auto HelloWorld = GScripting->GetFunction(PLATFORM_STRING("HelloWorld"));
        if (HelloWorld)
        {
            FPlatformString Author = PLATFORM_STRING("LJYC");
            for (UInt32 Idx = 0; Idx < 3; ++Idx)
            {
                FPlatformString Name = PLATFORM_STRING("LJYC");
                HelloWorld(Name.data(), Name.size());
            }
        }
        else {LOG_WARN("Failed to load the \"HelloWorld\"!");}

        GEngine->Run();
        GStudio->Terminate();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}