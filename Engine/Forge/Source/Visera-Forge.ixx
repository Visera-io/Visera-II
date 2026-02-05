module;
#include <Visera-Forge.hpp>
export module Visera.Forge;
#define VISERA_MODULE_NAME "Forge"
import Visera.Global;
import Visera.Tasks;
import Visera.AssetHub;
import Visera.Core.Types.String;

export namespace Visera::Forge
{
    int Execute(int I_Argc, char* I_Argv[])
    {
        LOG_INFO("Hello World!");

        if (I_Argc > 1 && FStringView(I_Argv[1]) == "Font")
        {
            LOG_INFO("Baking Font (WIP)");
            (void)IGlobalService::Register<FTasks>(EName::Tasks);
            (void)IGlobalService::Register<FAssetHub>(EName::AssetHub);
        }

        return 0;
    }
}

export int main(int I_Argc, char* I_Argv[])
{
    return Visera::Forge::Execute(I_Argc, I_Argv);
}