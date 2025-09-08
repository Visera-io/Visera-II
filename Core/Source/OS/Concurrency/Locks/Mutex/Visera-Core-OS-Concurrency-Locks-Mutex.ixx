module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Concurrency.Locks.Mutex;
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