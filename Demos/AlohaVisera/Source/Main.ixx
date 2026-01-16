module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
//import Visera.Game;
import Visera.RHI;
import Visera.Global;
import Visera.Platform;
import Visera.Audio;
import Visera.Assets.Image;
using namespace Visera;

struct FEngine
{
    FPlatform* Platform;
    FInput*    Input;
    FWindow*   Window;
    FRHI*      RHI;
    FAudio*    Audio;

    Bool Run()
    {
        FText Text{"123"};
        for (int i = 0; i < 100; i++)
        {
            Text += i;
        }
        LOG_INFO("({}): {}",Text.GetLength(), Text);
        auto Result = Text.FindAll("1");
        for (auto& R : Result)
        {

            LOG_INFO("({}): {}", Result.GetSize(), R);
        }

        while (!Window->ShouldClose())
        {
            Window->PollEvents();
        }

        return EXIT_SUCCESS;
    }

    FEngine()
    {
        Platform    = IGlobalService::Register<FPlatform>(EName::Platform);
        Input       = IGlobalService::Register<FInput>(EName::Input);
        Window      = IGlobalService::Register<FWindow>(EName::Window);
        RHI         = IGlobalService::Register<FRHI>(EName::RHI);
        Audio       = IGlobalService::Register<FAudio>(EName::Audio);

        IGlobalService::Bootstrap();
    }
    ~FEngine()
    {
        IGlobalService::Terminate();
    }
};

export int main(int argc, char *argv[])
{
    return FEngine{}.Run();
}