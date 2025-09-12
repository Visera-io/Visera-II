module;
#include <Visera-Runtime.hpp>
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <Windows.h>
#endif
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Platform;
export import Visera.Runtime.AssetHub;
export import Visera.Runtime.Window;
export import Visera.Runtime.Events;
export import Visera.Runtime.RHI;
       import Visera.Core.Log;
       import Visera.Core.OS.Time;

namespace Visera
{
    class VISERA_RUNTIME_API FRuntime
    {
        static FRuntime Runtime;
    public:
        FRuntime()
        {
            LOG_INFO("Initializing Runtime.");

#if defined(VISERA_ON_WINDOWS_SYSTEM)
            SetConsoleOutputCP(65001); // Set console output code page to UTF-8
            SetConsoleCP(65001);       // Also set input code page to UTF-8 for consistency
#endif

            GLog->Bootstrap();
        }

        ~FRuntime()
        {
            GLog->Terminate();
            LOG_INFO("Finalizing Runtime (running time: {}s).", Timer.Elapsed().Seconds());
        }

    private:
        FHiResClock Timer{};
    };

    FRuntime FRuntime::Runtime{};

}