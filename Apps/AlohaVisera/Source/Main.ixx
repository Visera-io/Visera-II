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
        Entity.Add<CTransform2D>();
        if (auto Transform = Entity.TryGet<CTransform2D>())
        {
            LOG_INFO("GET!!!");
        }
        LOG_INFO("{}", Entity.Has<CTransform2D>());

        GAudio->Register(BankInit);
        auto ID = GAudio->Register(MainBGM);
        GAudio->PostEvent("Play_MainBGM", ID);

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