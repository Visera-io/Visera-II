module;
#include <Visera-Tasks.hpp>
export module Visera.Tasks.Interface;
#define VISERA_MODULE_NAME "Tasks.Interface"

export namespace Visera
{
    namespace Concepts
    {
        template <typename F> concept
        Task = Callable<F, void>;
    }

    /**
     * Thread tag enumeration for task scheduling.
     * Used to specify which thread pool a task should run on.
     */
    enum class EThreadTag : UInt8
    {
        /** Any worker thread - tasks can run on any available worker thread */
        AnyWorker,
        /** IO thread - tasks run on the I/O thread pool */
        IO,
    };

    /**
     * Interface for task schedulers.
     * Provides a unified interface for enqueueing tasks asynchronously.
     */
    class VISERA_TASKS_API ITaskScheduler
    {
    public:
        template<Concepts::Task TaskType> void
        Enqueue(TaskType&& I_Task, EThreadTag I_ThreadTag = EThreadTag::AnyWorker);

    protected:
        virtual void
        DoEnqueue(TFunction<void()> I_Task, EThreadTag I_ThreadTag) = 0;

    public:
        virtual ~ITaskScheduler() = default;
    };

    template<Concepts::Task TaskType>
    void ITaskScheduler::
    Enqueue(TaskType&& I_Task, EThreadTag I_ThreadTag)
    {
        DoEnqueue(TFunction<void()>(std::forward<TaskType>(I_Task)), I_ThreadTag);
    }
}
