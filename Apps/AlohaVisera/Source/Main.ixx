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
    //
    // FHiResClock Clock{};
    // for (int i = 0; i < 1; ++i)
    // {
    //     auto Instance = GScene->CreateObject<Foo>(FName(Format("Foo_{}", i)));
    //     LOG_DEBUG("[{}] {}", Instance->GetID(), Instance->GetName());
    //     Instance->Update(1.02);
    // }
    // LOG_DEBUG("Test Time: {}ms", Clock.Elapsed().Milliseconds());
    //
    // //Demos
    // //{ Demos::Compression Demo; }
    //
    GLog->Bootstrap();
    GEngine->Bootstrap();
    {
        GStudio->Bootstrap();

        auto BankInit = GAssetHub->LoadSound(PATH("Init.bnk"));
        auto MainBGM = GAssetHub->LoadSound(PATH("Test.bnk"));

        GAudio->Register(BankInit);
        auto ID = GAudio->Register(MainBGM);
        GAudio->PostEvent("Play_Galaxy", ID);

        FDotNETComponent AppLib{};
        auto HelloWorld = AppLib.GetFunction(PLATFORM_STRING("HelloWorld"));
        if (!HelloWorld)
        { LOG_FATAL("Failed to load the \"HelloWorld\"!"); }

        FWideString Author = PLATFORM_STRING("LJYC");
        for (UInt32 Idx = 0; Idx < 1024; ++Idx)
        {
            FWideString Name = FWideString(L"LJYC ") + std::to_wstring(Idx+1);
            HelloWorld(Name.data(), Name.size());
        }

//         auto App = GScripting->CreateCommandLineApp(
// {
//             PATH("Test/Foo/bin/Debug/net10.0/Foo.dll"),
//             PATH("LJYC"),
//         }
//         );
//         App->Run();

        GEngine->Run();

        GStudio->Terminate();
    }
    GEngine->Terminate();
    return EXIT_SUCCESS;
}