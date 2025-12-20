module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.Debug;
#define VISERA_MODULE_NAME "Graphics.Debug"
import Visera.Graphics.Debug.UI;
import Visera.Platform.Window;
import Visera.Runtime.Log;
import Visera.Runtime.Global;

namespace Visera::Graphics
{
    export class VISERA_GRAPHICS_API FDebug : public IGlobalSingleton<FDebug>
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

    export inline VISERA_GRAPHICS_API TUniquePtr<FDebug>
    GDebug = MakeUnique<FDebug>();
}