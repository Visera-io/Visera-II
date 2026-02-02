module;
#include <Visera-Core.hpp>
#include <shared_mutex>
export module Visera.Core.OS.Thread.Sync.RWLock;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FRWLock
    {
    public:
        [[nodiscard]] inline Bool
        TryToRead()    const { return Self.try_lock_shared(); }
        void
        StartReading() const { Self.lock_shared(); }
        void
        StopReading()  const { Self.unlock_shared(); }

        [[nodiscard]] inline Bool
        TryToWrite()   { return Self.try_lock(); }
        void
        StartWriting() { Self.lock(); }
        void
        StopWriting()  { Self.unlock(); }

    private:
        mutable std::shared_mutex Self;
    };
}