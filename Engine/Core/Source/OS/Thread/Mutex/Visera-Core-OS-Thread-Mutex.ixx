module;
#include <Visera-Core.hpp>
#include <mutex>
export module Visera.Core.OS.Thread.Mutex;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FMutex
    {
    public:
        Bool TryToLock() { return Handle.try_lock(); }
        void Lock()		 { Handle.lock(); }
        void Unlock()	 { Handle.unlock(); }

    private:
        std::mutex Handle;
    };
}