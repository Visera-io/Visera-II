module;
#include <Visera-Tasks.hpp>
export module Visera.Tasks;
#define VISERA_MODULE_NAME "Tasks"
export import Visera.Tasks.Interface;
export import Visera.Tasks.Graph;
       import Visera.Tasks.Pipeline;
       import Visera.Tasks.Scheduler;
       import Visera.Global;

export namespace Visera
{
    /**
     * Main facade for the Tasks system.
     * Provides unified access to task scheduling, pipelines, and events.
     * Hides internal implementation details (Scheduler module).
     */
    class VISERA_TASKS_API FTasks : public IGlobalService
    {
    public:
        [[nodiscard]] FTaskPipeline
        CreatePipeline(const FString& I_Name = "");
        [[nodiscard]] auto
        RunPipeline(FTaskPipeline& I_Pipeline);
        template<Concepts::Task TaskType> [[nodiscard]] auto
        RunPipeline(FTaskPipeline& I_Pipeline, TaskType&& I_Callback);
        template<Concepts::Task TaskType> void
        Enqueue(TaskType&& I_Task, EThreadTag I_ThreadTag = EThreadTag::AnyWorker);
        void
        WaitForAll() { if (Scheduler) { Scheduler->WaitForAll(); } }

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
                Scheduler.reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    // Implementation

    FTaskPipeline FTasks::
    CreatePipeline(const FString& I_Name)
    {
        return FTaskPipeline(I_Name);
    }

    auto FTasks::
    RunPipeline(FTaskPipeline& I_Pipeline)
    {
        VISERA_ASSERT(Scheduler != nullptr && "Scheduler must be initialized before running pipeline!");
        return Scheduler->Run(I_Pipeline);
    }

    template<Concepts::Task TaskType> auto FTasks::
    RunPipeline(FTaskPipeline& I_Pipeline, TaskType&& I_Callback)
    {
        VISERA_ASSERT(Scheduler != nullptr && "Scheduler must be initialized before running pipeline!");
        return Scheduler->Run(I_Pipeline, std::forward<TaskType>(I_Callback));
    }

    template<Concepts::Task TaskType> void FTasks::
    Enqueue(TaskType&& I_Task, EThreadTag I_ThreadTag)
    {
        VISERA_ASSERT(Scheduler != nullptr && "Scheduler must be initialized before enqueueing task!");
        Scheduler->Enqueue(std::forward<TaskType>(I_Task), I_ThreadTag);
    }
}