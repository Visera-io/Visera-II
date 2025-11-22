module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scheduler;
#define VISERA_MODULE_NAME "Runtime.Scheduler"
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FScheduler : public IGlobalSingleton<FScheduler>
    {
    public:

    private:

    public:
        FScheduler() : IGlobalSingleton("Scheduler") {}
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Scheduler.");
        }
        void Terminate() override
        {
            LOG_TRACE("Terminating Scheduler.");
        }
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FScheduler>
    GScheduler = MakeUnique<FScheduler>();
}