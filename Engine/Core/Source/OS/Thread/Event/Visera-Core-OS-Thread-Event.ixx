module;
#include <Visera-Core.hpp>
#include <condition_variable>
export module Visera.Core.OS.Thread.Event;
#define VISERA_MODULE_NAME "Core.OS"

export namespace Visera
{
    class VISERA_CORE_API FEvent
    {
    public:
        FEvent() : bIsSignaled(False) {}
        void
        Trigger();
        void
        Reset();
        void
        Wait();
        [[nodiscard]] Bool
        TryWait();
        [[nodiscard]] Bool
        IsSignaled() const;

    private:
        mutable std::mutex Mutex;
        std::condition_variable ConditionVariable;
        Bool bIsSignaled;
    };

    void FEvent::
    Trigger()
    {
        {
            std::lock_guard Lock(Mutex);
            bIsSignaled = True;
        }
        ConditionVariable.notify_all();
    }

    void FEvent::
    Reset()
    {
        std::lock_guard Lock(Mutex);
        bIsSignaled = False;
    }

    void FEvent::
    Wait()
    {
        std::unique_lock Lock(Mutex);
        ConditionVariable.wait(Lock, [this]
        {
            return bIsSignaled;
        });
    }

    Bool FEvent::
    TryWait()
    {
        std::lock_guard Lock(Mutex);
        return bIsSignaled;
    }

    Bool FEvent::
    IsSignaled() const
    {
        std::lock_guard Lock(Mutex);
        return bIsSignaled;
    }
}
