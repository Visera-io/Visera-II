module;
#include <Visera-Core.hpp>
#include <thread>
export module Visera.Core.OS.Thread;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.OS.Thread.Mutex;
export import Visera.Core.OS.Thread.RWLock;
export import Visera.Core.OS.Thread.SpinLock;

export namespace Visera
{
    VISERA_CORE_API void inline
    Sleep(Float I_Seconds) { std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<UInt64>(1000 * I_Seconds))); }
}