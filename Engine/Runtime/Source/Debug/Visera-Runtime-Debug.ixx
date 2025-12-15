module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Debug;
#define VISERA_MODULE_NAME "Runtime.Debug"
export import Visera.Runtime.Debug.UI;
       import Visera.Core.Global;
       import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FDebug : public IGlobalSingleton<FDebug>
    {
    public:

    private:

    public:
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Debug.");

            GLog->Bootstrap();

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_TRACE("Terminating Debug.");

            GLog->Terminate();

            Status = EStatus::Terminated;
        }

        FDebug() : IGlobalSingleton("Debug") {}
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FDebug>
    GDebug = MakeUnique<FDebug>();
}