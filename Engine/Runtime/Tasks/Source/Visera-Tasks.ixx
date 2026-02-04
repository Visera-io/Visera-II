module;
#include <Visera-Tasks.hpp>
export module Visera.Tasks;
#define VISERA_MODULE_NAME "Tasks"
export import Visera.Tasks.Interface;
       import Visera.Tasks.Scheduler;
       import Visera.Global;
       import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    class VISERA_TASKS_API FTasks : public IGlobalService
    {
    public:

    private:
        TUniquePtr<FTaskScheduler> Scheduler;

    public:
        FTasks() : IGlobalService(EName::Tasks)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                // Initialize default scheduler in bootstrap
                Scheduler = MakeUnique<FTaskScheduler>();
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                if (Scheduler)
                {
                    // Wait for all tasks to complete before termination
                    Scheduler->WaitForAll();
                }
                // Destroy scheduler
                Scheduler.Reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };
}