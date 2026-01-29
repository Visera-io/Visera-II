module;
#include <Visera-Tasks.hpp>
#include <taskflow/taskflow.hpp>
export module Visera.Tasks.Scheduler;
#define VISERA_MODULE_NAME "Tasks.Scheduler"
import Visera.Tasks.Interface;
import Visera.Tasks.Pipeline;
import Visera.Global.Log;

export namespace Visera
{
    /**
     * Wrapper around taskflow::Executor for scheduling and executing task pipelines.
     * Provides a Visera-style interface for running tasks asynchronously.
     * Implements ITaskScheduler interface for unified task enqueueing.
     */
    class VISERA_TASKS_API FTaskScheduler : public ITaskScheduler
    {
    public:
        [[nodiscard]] auto
        Run(FTaskPipeline& I_Pipeline);

        template<Concepts::Callable<void()> Callable>
        [[nodiscard]] auto
        Run(FTaskPipeline& I_Pipeline, Callable&& I_Callback);

        void
        WaitForAll();

        [[nodiscard]] UInt32
        GetNumWorkers() const;

    private:
        [[nodiscard]] static UInt32
        GetWorkerCount(UInt32 I_NumWorkers);

        void
        DoEnqueue(TFunction<void()> I_Task, EThreadTag I_ThreadTag) override;

        tf::Executor Executor;

    public:
        explicit FTaskScheduler(UInt32 I_NumWorkers = 0);
    };

    // Implementation

    FTaskScheduler::FTaskScheduler(UInt32 I_NumWorkers)
        : Executor(GetWorkerCount(I_NumWorkers))
    {
        LOG_DEBUG("Task scheduler initialized with {} workers", Executor.num_workers());
    }

    auto FTaskScheduler::
    Run(FTaskPipeline& I_Pipeline)
    {
        return Executor.run(I_Pipeline.GetTaskflow());
    }

    template<Concepts::Callable<void()> Callable>
    auto FTaskScheduler::
    Run(FTaskPipeline& I_Pipeline, Callable&& I_Callback)
    {
        return Executor.run(I_Pipeline.GetTaskflow(), std::forward<Callable>(I_Callback));
    }

    void FTaskScheduler::
    WaitForAll()
    {
        Executor.wait_for_all();
    }

    UInt32 FTaskScheduler::
    GetNumWorkers() const
    {
        return static_cast<UInt32>(Executor.num_workers());
    }

    void FTaskScheduler::
    DoEnqueue(TFunction<void()> I_Task, EThreadTag I_ThreadTag)
    {
        // For now, ignore thread tag and use async
        // [TODO]: Implement thread tag-based scheduling if needed
        Executor.async(std::move(I_Task));
    }

    UInt32 FTaskScheduler::
    GetWorkerCount(UInt32 I_NumWorkers)
    {
        if (I_NumWorkers > 0)
        {
            return I_NumWorkers;
        }

        const UInt32 HardwareConcurrency = static_cast<UInt32>(std::thread::hardware_concurrency());
        if (HardwareConcurrency == 0)
        {
            LOG_WARN("hardware_concurrency() returned 0, falling back to 1 worker thread");
            return 1;
        }

        return HardwareConcurrency;
    }
}
