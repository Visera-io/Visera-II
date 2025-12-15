module;
#include <Visera-Core.hpp>
#include <shared_mutex>
export module Visera.Core.OS.Thread.RWLock;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FRWLock
    {
    public:
        [[nodiscard]] inline Bool
        TryToRead()    const { return Handle.try_lock_shared(); }
        void
        StartReading() const { Handle.lock_shared(); }
        void
        StopReading()  const { Handle.unlock_shared(); }

        [[nodiscard]] inline Bool
        TryToWrite()   { return Handle.try_lock(); }
        void
        StartWriting() { Handle.lock(); }
        void
        StopWriting()  { Handle.unlock(); }

    private:
        mutable std::shared_mutex Handle;
    };
}