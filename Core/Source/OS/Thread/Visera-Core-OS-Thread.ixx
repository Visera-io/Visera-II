module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Thread;
#define VISERA_MODULE_NAME "Core.OS.Thread"

export namespace Visera
{
    VISERA_CORE_API void inline
    Sleep(UInt32 I_MilliSeconds) { std::this_thread::sleep_for(std::chrono::milliseconds(I_MilliSeconds)); }
}
