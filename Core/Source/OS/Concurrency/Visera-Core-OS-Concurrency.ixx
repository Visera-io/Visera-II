module;
#include <Visera-Core.hpp>
#include <oneapi/tbb/parallel_for.h>
export module Visera.Core.OS.Concurrency;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.OS.Concurrency.Containers;
export import Visera.Core.OS.Concurrency.Locks;
export import Visera.Core.OS.Concurrency.Parallel;
export import Visera.Core.OS.Concurrency.TaskGroup;
export import Visera.Core.OS.Concurrency.TaskScheduler;

export namespace Visera
{

}