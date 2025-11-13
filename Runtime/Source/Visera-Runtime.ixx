module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Platform;
export import Visera.Runtime.Window;
export import Visera.Runtime.Event;
export import Visera.Runtime.RHI;
export import Visera.Runtime.Media;
export import Visera.Runtime.Scene;
       import Visera.Core.Log;
       import Visera.Core.OS.Time;

namespace Visera
{
    export class VISERA_RUNTIME_API FRuntime : public IGlobalSingleton<FRuntime>
    {
    public:
        [[nodiscard]] inline const FHiResClock&
        GetTimer() const { return Timer; }

        std::function<void()> StudioBeginFrame = [](){};
        std::function<void()> StudioEndFrame   = [](){};

    private:
        FHiResClock Timer{};

    public:
        void Bootstrap() override
        {
            LOG_INFO("Initializing Runtime.");
            GPlatform   ->Bootstrap();
            GMedia      ->Bootstrap();
            GWindow     ->Bootstrap();
            GRHI        ->Bootstrap();
            GScene      ->Bootstrap();

#if defined(VISERA_ON_WINDOWS_SYSTEM)
            SetConsoleOutputCP(65001); // Set console output code page to UTF-8
            SetConsoleCP(65001);       // Also set input code page to UTF-8 for consistency
#endif

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_INFO("Finalizing Runtime (running time: {}s).", Timer.Elapsed().Seconds());
            GScene      ->Terminate();
            GRHI        ->Terminate();
            GWindow     ->Terminate();
            GMedia      ->Terminate();
            GPlatform   ->Terminate();

            Status = EStatus::Terminated;
        }

        FRuntime() : IGlobalSingleton("Runtime")
        { GLog->Bootstrap(); }

        ~FRuntime() override
        { GLog->Terminate(); }
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRuntime>
    GRuntime = MakeUnique<FRuntime>();
}