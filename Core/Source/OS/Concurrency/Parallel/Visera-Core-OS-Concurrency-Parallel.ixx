module;
#include <Visera-Core.hpp>
#include <oneapi/tbb.h>
export module Visera.Core.OS.Concurrency.Parallel;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    template <typename Fn> requires std::invocable<Fn&, UInt32> void
        ParallelFor(UInt32 I_Begin, UInt32 I_End, Fn&& I_Function)
    {
        tbb::parallel_for(I_Begin, I_End, std::forward<Fn>(I_Function));
    }
}