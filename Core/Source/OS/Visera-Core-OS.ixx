module;
#include <Visera-Core.hpp>
export module Visera.Core.OS;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.OS.Memory;
export import Visera.Core.OS.Mutex;

export namespace Visera
{
    namespace OS
    {
        inline void
        Sleep(UInt32 I_MilliSeconds) { std::this_thread::sleep_for(std::chrono::milliseconds(I_MilliSeconds)); }

    }
}
