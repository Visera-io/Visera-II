module;
#include <Visera-Core.hpp>
#include <atomic>
export module Visera.Core.OS.Thread.Sync.Atomic;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    /** Unified memory order enum for readability and explicit ordering at call sites. */
    enum class EMemoryOrder : int
    {
        Relaxed = static_cast<int>(std::memory_order_relaxed),
        Acquire = static_cast<int>(std::memory_order_acquire),
        Release = static_cast<int>(std::memory_order_release),
        AcqRel  = static_cast<int>(std::memory_order_acq_rel),
        SeqCst  = static_cast<int>(std::memory_order_seq_cst)
    };

    /** Thin wrapper over std::atomic that requires explicit memory order on Load/Store/CompareExchange. */
    template<typename T>
    struct TAtomic : private std::atomic<T>
    {
        using std::atomic<T>::atomic;

        VISERA_FORCEINLINE T Load(EMemoryOrder Order) const noexcept
        {
            return this->load(static_cast<std::memory_order>(Order));
        }

        VISERA_FORCEINLINE void Store(T Value, EMemoryOrder Order) noexcept
        {
            this->store(Value, static_cast<std::memory_order>(Order));
        }

        /** Compare-and-swap; critical for SPSC/MPSC patterns. */
        VISERA_FORCEINLINE Bool CompareExchange(T& Expected, T Desired, EMemoryOrder Success, EMemoryOrder Failure) noexcept
        {
            return this->compare_exchange_strong(Expected, Desired,
                static_cast<std::memory_order>(Success),
                static_cast<std::memory_order>(Failure));
        }

        /** Exchange value; returns previous value. */
        VISERA_FORCEINLINE T Exchange(T Desired, EMemoryOrder Order) noexcept
        {
            return this->exchange(Desired, static_cast<std::memory_order>(Order));
        }

        /** Fetch-add (integral/pointer); returns value before the add. */
        VISERA_FORCEINLINE T FetchAdd(T Arg, EMemoryOrder Order) noexcept
        {
            return this->fetch_add(Arg, static_cast<std::memory_order>(Order));
        }

        /** Fetch-sub (integral/pointer); returns value before the sub. */
        VISERA_FORCEINLINE T FetchSub(T Arg, EMemoryOrder Order) noexcept
        {
            return this->fetch_sub(Arg, static_cast<std::memory_order>(Order));
        }
    };

    /** Wrapper over std::atomic_flag with explicit memory order (e.g. for SpinLock). */
    class VISERA_CORE_API FAtomicFlag
    {
    public:
        VISERA_FORCEINLINE Bool TestAndSet(EMemoryOrder Order) noexcept
        {
            return Flag.test_and_set(static_cast<std::memory_order>(Order));
        }
        VISERA_FORCEINLINE void Clear(EMemoryOrder Order) noexcept
        {
            Flag.clear(static_cast<std::memory_order>(Order));
        }
    private:
        std::atomic_flag Flag = ATOMIC_FLAG_INIT;
    };
}
