module;
#include <Visera-Tasks.hpp>
export module Visera.Runtime.Tasks;
#define VISERA_MODULE_NAME "Runtime.Task"
export import Visera.Runtime.Tasks.Interface;
       import Visera.Runtime.Tasks.Scheduler;
       import Visera.Runtime.Global;
       import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    class VISERA_RUNTIME_API FTasks : public IGlobalService
    {
    public:

    private:
        TUniquePtr<FTaskScheduler> Scheduler;

    public:
        FTasks(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
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