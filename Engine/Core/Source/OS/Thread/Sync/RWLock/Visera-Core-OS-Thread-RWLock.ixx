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

    class VISERA_CORE_API FScopeReadLock
    {
    public:
        explicit FScopeReadLock(FRWLock* I_RWLock) : RWLock(I_RWLock)
        {
            VISERA_ASSERT(RWLock != nullptr);
            RWLock->StartReading();
        }

        ~FScopeReadLock()
        {
            RWLock->StopReading();
        }

        FScopeReadLock(const FScopeReadLock&)              = delete;
        FScopeReadLock& operator=(const FScopeReadLock&)   = delete;
        FScopeReadLock(FScopeReadLock&&)                   = delete;
        FScopeReadLock& operator=(FScopeReadLock&&)        = delete;

    private:
        FRWLock* RWLock {nullptr};
    };

    class VISERA_CORE_API FScopeWriteLock
    {
    public:
        explicit FScopeWriteLock(FRWLock* I_RWLock) : RWLock(I_RWLock)
        {
            VISERA_ASSERT(RWLock != nullptr);
            RWLock->StartWriting();
        }

        ~FScopeWriteLock()
        {
            RWLock->StopWriting();
        }

        FScopeWriteLock(const FScopeWriteLock&)              = delete;
        FScopeWriteLock& operator=(const FScopeWriteLock&)   = delete;
        FScopeWriteLock(FScopeWriteLock&&)                   = delete;
        FScopeWriteLock& operator=(FScopeWriteLock&&)        = delete;

    private:
        FRWLock* RWLock {nullptr};
    };
}