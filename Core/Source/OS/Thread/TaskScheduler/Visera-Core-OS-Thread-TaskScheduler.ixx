module;
#include <Visera-Core.hpp>
#include <oneapi/tbb.h>
export module Visera.Core.OS.Thread.TaskScheduler;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.OS.Thread.TaskGroup;

export namespace Visera
{
    class VISERA_CORE_API FTaskScheduler
    {
    public:
        explicit FTaskScheduler(int Threads = tbb::task_arena::automatic)
            : Arena(Threads) {}

        void Execute(auto&& Fn)
        {
            Arena.execute(std::forward<decltype(Fn)>(Fn));
        }

    private:
        tbb::task_arena Arena;
    };
}