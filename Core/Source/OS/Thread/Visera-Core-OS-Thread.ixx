module;
#include <Visera-Core.hpp>
#include <oneapi/tbb.h>
export module Visera.Core.OS.Thread;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.OS.Thread.TaskGroup;
export import Visera.Core.OS.Thread.TaskScheduler;

export namespace Visera
{
    VISERA_CORE_API void inline
    Sleep(UInt32 I_MilliSeconds) { std::this_thread::sleep_for(std::chrono::milliseconds(I_MilliSeconds)); }

    template <typename Fn> VISERA_CORE_API void
    ParallelFor(UInt32 I_Begin, UInt32 I_End, Fn&& I_Function)
    {
        tbb::parallel_for(I_Begin, I_End, std::forward<Fn>(I_Function));
    }
}