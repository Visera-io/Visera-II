module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Scripting;
#define VISERA_MODULE_NAME "Engine.Scripting"
import Visera.Engine.Scripting.DotNET;

namespace Visera
{
    export class VISERA_ENGINE_API FScripting : public IGlobalSingleton<FScripting>
    {
    public:

    private:
        TUniquePtr<FDotNET> DotNET;

    public:
        FScripting() : IGlobalSingleton{"Scripting"} {}
        void inline
        Bootstrap() override { DotNET = MakeUnique<FDotNET>(); Status = EStatus::Bootstrapped; }
        void inline
        Terminate() override { Status = EStatus::Terminated; }
    };

    export inline VISERA_ENGINE_API TUniquePtr<FScripting>
    GScripting = MakeUnique<FScripting>();
}