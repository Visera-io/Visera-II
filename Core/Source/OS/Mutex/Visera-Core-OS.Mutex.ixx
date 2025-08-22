module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Mutex;
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

    class VISERA_CORE_API FRWLock
    {
    public:
        Bool TryToRead()    const { return Handle.try_lock_shared(); }
        void StartReading() const { Handle.lock_shared(); }
        void StopReading()  const { Handle.unlock_shared(); }

        Bool TryToWrite()   { return Handle.try_lock(); }
        void StartWriting() { Handle.lock(); }
        void StopWriting()  { Handle.unlock(); }

    private:
        mutable std::shared_mutex Handle;
    };
}