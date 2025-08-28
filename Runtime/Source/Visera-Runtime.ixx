module;
#include <Visera-Runtime.hpp>
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <Windows.h>
#endif
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Graphics;
export import Visera.Runtime.RHI;
export import Visera.Runtime.Window;
       import Visera.Core.OS.Time;

namespace Visera
{
    class VISERA_RUNTIME_API FRuntime
    {
        static FRuntime Runtime;
    public:
        FRuntime()
        {
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            SetConsoleOutputCP(65001); // Set console output code page to UTF-8
            SetConsoleCP(65001);       // Also set input code page to UTF-8 for consistency
#endif
        }

        ~FRuntime()
        {
            if (GWindow->IsBootstrapped()) { GWindow->Terminate(); }
            if (GRHI->IsBootstrapped())    { GRHI->Terminate(); }
            std::cout << fmt::format("[Runtime] Finalizing Runtime (running time: {}s)\n", Timer.Elapsed().Seconds());
        }

    private:
        FHiResClock Timer{};
    };

    FRuntime FRuntime::Runtime{};

}