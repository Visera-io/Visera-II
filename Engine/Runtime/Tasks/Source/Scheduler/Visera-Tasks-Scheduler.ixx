module;
#include <Visera-Tasks.hpp>
export module Visera.Tasks.Scheduler;
#define VISERA_MODULE_NAME "Tasks.Scheduler"
import Visera.Tasks.Interface;
import Visera.Global.Log;

export namespace Visera
{
    class VISERA_TASKS_API FTaskScheduler
    {
    public:
        void
        WaitForAll() {}
    };
}
