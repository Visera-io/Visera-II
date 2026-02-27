module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Thread.Sync.SpinLock;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.OS.Thread.Sync.Atomic;

export namespace Visera
{
    class VISERA_CORE_API FSpinLock
    {
    public:
        void
        Lock()   { while (Flag.TestAndSet(EMemoryOrder::Acquire)); }
        void
        Unlock() { Flag.Clear(EMemoryOrder::Release); }

        FSpinLock() = default;
        FSpinLock(const FSpinLock&) = delete;
        FSpinLock& operator=(const FSpinLock&) = delete;

    private:
        FAtomicFlag Flag;
    };
}