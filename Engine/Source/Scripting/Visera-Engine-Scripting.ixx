module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Scripting;
#define VISERA_MODULE_NAME "Engine.Scripting"
import Visera.Engine.Scripting.DotNET;

namespace Visera
{

    class VISERA_ENGINE_API FScripting : IGlobalSingleton
    {
    public:

    private:
        TUniquePtr<FDotNET> DotNET;

    public:
        FScripting() : IGlobalSingleton{"Scripting"} {}
        void inline
        Bootstrap() override { DotNET = MakeUnique<FDotNET>(); };
        void inline
        Terminate() override {};

    };

    export inline VISERA_ENGINE_API TUniquePtr<FScripting>
    GScripting = MakeUnique<FScripting>();

}