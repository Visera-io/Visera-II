module;
#include <Visera-Core.hpp>
#include <oneapi/tbb.h>
export module Visera.Core.OS.Thread.TaskGroup;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FTaskGroup
    {
    public:
        FTaskGroup() = default;

        // Run a task asynchronously
        template <typename Fn>
        void Run(Fn&& func)
        {
            Group.run(std::forward<Fn>(func));
        }

        // Wait for all tasks
        void Wait()
        {
            Group.wait();
        }

    private:
        tbb::task_group Group;
    };
}