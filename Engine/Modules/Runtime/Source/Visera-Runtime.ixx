module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime;
#define VISERA_MODULE_NAME "Runtime"
export import Visera.Runtime.Log;
       import Visera.Runtime.Scheduler;
       import Visera.Core.OS.Time;
       import Visera.Runtime.Global;

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

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_INFO("Finalizing Runtime (running time: {}s).", Timer.Elapsed().Seconds());

            Status = EStatus::Terminated;
        }

        FRuntime() : IGlobalSingleton("Runtime") {}
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FRuntime>
    GRuntime = MakeUnique<FRuntime>();
}