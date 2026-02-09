module;
#include <Visera-Tasks.hpp>
export module Visera.Runtime.Tasks.Scheduler;
#define VISERA_MODULE_NAME "Runtime.Tasks"
import Visera.Runtime.Tasks.Interface;
import Visera.Core.Log;

export namespace Visera
{
    class VISERA_RUNTIME_API FTaskScheduler
    {
    public:
        void
        WaitForAll() {}
    };
}
