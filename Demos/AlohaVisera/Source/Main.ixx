module;
#include <Visera.hpp>
export module AlohaVisera;
#define VISERA_MODULE_NAME "AlohaVisera"
import Visera.Core;
//import Visera.Game;
import Visera.RHI;
import Visera.Global;
import Visera.Platform;
import Visera.Assets.Image;
using namespace Visera;

struct FEngine
{
    FPlatform* Platform;
    FInput*    Input;
    FWindow*   Window;
    FRHI*      RHI;

    Bool Run()
    {
        struct Case { const char* s; UInt64 expect; };

        Case cases[] = {
            {"", 0},
            {"hello", 5},
            {"中", 1},
            {"こんにちは", 5},
            {"😀", 1},
            {"a😀b", 3},
            {"e\u0301", 2},
            {"\u00E9", 1},
            {"\r\n", 2},
          };

        for (auto& c : cases)
        {
            FText t{c.s};
            LOG_INFO("{}", t);
            VISERA_ASSERT(t.GetCodepointCount() == c.expect);
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
        //Audio       = IGlobalService::Register<FAudio>(EName::Audio);

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