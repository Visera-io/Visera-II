module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Debug;
#define VISERA_MODULE_NAME "Runtime.Debug"
import Visera.Runtime.Debug.UI;
import Visera.Runtime.Window;
import Visera.Core.Global;

namespace Visera
{
    export class VISERA_RUNTIME_API FDebug : public IGlobalSingleton<FDebug>
    {
    public:
        TUniqueRef<FDebugUI> UI  = DebugUI;

    private:
        TUniquePtr<FDebugUI> DebugUI;

    public:
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Debug.");

            //DebugUI  = MakeUnique<FDebugUI>(GWindow);

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_TRACE("Terminating Debug.");

            //DebugUI.reset();

            Status = EStatus::Terminated;
        }

        FDebug() : IGlobalSingleton("Debug") {}
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FDebug>
    GDebug = MakeUnique<FDebug>();
}