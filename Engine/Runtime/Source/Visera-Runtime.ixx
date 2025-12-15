module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Platform;
export import Visera.Runtime.Window;
export import Visera.Runtime.Input;
export import Visera.Runtime.Debug;
export import Visera.Runtime.RHI;
export import Visera.Runtime.Media;
export import Visera.Runtime.Physics;
export import Visera.Runtime.Scripting;
export import Visera.Runtime.Scheduler;
       import Visera.Core.OS.Time;
       import Visera.Core.Global;
       import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FRuntime : public IGlobalSingleton<FRuntime>
    {
    public:
        [[nodiscard]] inline const FHiResClock&
        GetTimer() const { return Timer; }

    private:
        FHiResClock Timer{};

    public:
        void Bootstrap() override
        {
            LOG_INFO("Initializing Runtime.");

            GDebug      ->Bootstrap();
            GPlatform   ->Bootstrap();
            //GScripting  ->Bootstrap();
#if !defined(VISERA_OFFSCREEN_MODE)
            GWindow     ->Bootstrap();
            GInput      ->Bootstrap();
#endif
            GRHI        ->Bootstrap();
            GMedia      ->Bootstrap();
            GPhysics    ->Bootstrap();

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_INFO("Finalizing Runtime (running time: {}s).", Timer.Elapsed().Seconds());

            GPhysics    ->Terminate();
            GMedia      ->Terminate();
            GRHI        ->Terminate();
#if !defined(VISERA_OFFSCREEN_MODE)
            GInput      ->Terminate();
            GWindow     ->Terminate();
#endif
            //GScripting  ->Terminate();
            GPlatform   ->Terminate();
            GDebug      ->Terminate();

            Status = EStatus::Terminated;
        }

        FRuntime() : IGlobalSingleton("Runtime") {}
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRuntime>
    GRuntime = MakeUnique<FRuntime>();
}