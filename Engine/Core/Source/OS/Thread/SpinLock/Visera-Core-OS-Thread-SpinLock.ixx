module;
#include <Visera-Core.hpp>
#include <atomic>
export module Visera.Core.OS.Thread.SpinLock;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FSpinLock
    {
    public:
        void
        Lock()   { while (Flag.test_and_set(std::memory_order_acquire)); }
        void
        Unlock() { Flag.clear(std::memory_order_release); }

        FSpinLock() = default;
        FSpinLock(const FSpinLock&) = delete;
        FSpinLock& operator=(const FSpinLock&) = delete;

    private:
        std::atomic_flag Flag = ATOMIC_FLAG_INIT;
    };
}