module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting;
#define VISERA_MODULE_NAME "Runtime.Scripting"
import Visera.Runtime.Scripting.DotNET;

namespace Visera
{

    class VISERA_RUNTIME_API FScripting : IGlobalSingleton
    {
    public:

    private:
        FDotNET DotNET;

    public:
        FScripting() : IGlobalSingleton{"Scripting"} {}
        void inline
        Bootstrap() override {};
        void inline
        Terminate() override {};

    };

    export inline VISERA_RUNTIME_API TUniquePtr<FScripting>
    GScripting = MakeUnique<FScripting>();

}