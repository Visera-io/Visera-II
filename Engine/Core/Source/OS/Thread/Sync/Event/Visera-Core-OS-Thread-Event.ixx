module;
#include <Visera-Core.hpp>
#include <condition_variable>
export module Visera.Core.OS.Thread.Sync.Event;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FEvent
    {
    public:
        FEvent() : bSignaled(False) {}
        void
        Trigger();
        void
        Reset();
        void
        Wait();
        /** Wait up to I_TimeoutNs nanoseconds. Returns true if signaled, false if timeout. */
        [[nodiscard]] Bool
        WaitFor(UInt64 I_TimeoutNs);
        [[nodiscard]] Bool
        TryWait();
        [[nodiscard]] Bool
        IsSignaled() const;

    private:
        mutable std::mutex      Mutex;
        std::condition_variable ConditionVariable;
        Bool                    bSignaled;
    };

    void FEvent::
    Trigger()
    {
        {
            std::lock_guard Lock(Mutex);
            bSignaled = True;
        }
        ConditionVariable.notify_all();
    }

    void FEvent::
    Reset()
    {
        std::lock_guard Lock(Mutex);
        bSignaled = False;
    }

    void FEvent::
    Wait()
    {
        std::unique_lock Lock(Mutex);
        ConditionVariable.wait(Lock, [this]
        {
            return bSignaled;
        });
    }

    Bool FEvent::
    WaitFor(UInt64 I_TimeoutNs)
    {
        std::unique_lock Lock(Mutex);
        const auto Done = ConditionVariable.wait_for(Lock, std::chrono::nanoseconds(I_TimeoutNs), [this]
        {
            return bSignaled;
        });
        return Done;
    }

    Bool FEvent::
    TryWait()
    {
        std::lock_guard Lock(Mutex);
        return bSignaled;
    }

    Bool FEvent::
    IsSignaled() const
    {
        std::lock_guard Lock(Mutex);
        return bSignaled;
    }
}
