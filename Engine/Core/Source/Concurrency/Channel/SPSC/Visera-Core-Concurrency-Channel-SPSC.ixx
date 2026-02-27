module;
#include <Visera-Core.hpp>
#include <coroutine>
export module Visera.Core.Concurrency.Channel.SPSC;
#define VISERA_MODULE_NAME "Core.Concurrency"
import Visera.Core.Concurrency.Async;
import Visera.Core.OS.Thread.Queue.SPSC;
import Visera.Core.OS.Thread.Sync.Atomic;
import Visera.Core.OS.Thread.Sync.Event;
import Visera.Core.Types.Optional;

export namespace Visera
{
    /**
     * Single Producer / Single Consumer channel: event-driven Receive with non-blocking Send.
     * Wraps TSPSCQueue + FEvent. Misuse (multiple producers or consumers) is UB.
     * - Send: non-blocking; enqueues then signals. Signals both blocking Receive (via FEvent) and ReceiveAsync (via WaitingHandle); both are signaled every time.
     * - Receive: blocks until an item is available; returns TOptional (same as Queue), user calls GetValue().
     * - TryReceive: non-blocking; returns TOptional (same as Queue.Dequeue()).
     * - ReceiveAsync: returns TAsync<TOptional<T>>; suspends until an item is available; user gets optional and GetValue(). ReceiveAsync continuation runs on the Send caller's thread. If the consumer does heavy work after each receive, consider offloading it (e.g. to a task queue) to avoid blocking the producer.
     * - SendAsync: returns TAsync<void> for co_await in coroutines.
     */
    template<typename T>
    class VISERA_CORE_API TSPSCChannel final
    {
    public:
        using ElementType = T;

        /** Enqueue then signal; resumes ReceiveAsync waiter inline on this thread if any. */
        template<typename... ArgTypes> void
        Send(ArgTypes&&... Args)
        {
            Queue.Enqueue(std::forward<ArgTypes>(Args)...);
            ItemAvailable.Trigger();
            if (std::coroutine_handle<> Handle = WaitingHandle.Exchange(nullptr, EMemoryOrder::AcqRel))
            { Handle.resume(); }
        }

        /** Non-blocking; returns empty if no item. */
        [[nodiscard]] TOptional<ElementType>
        TryReceive()
        {
            return Queue.Dequeue();
        }

        /** Block until an item is available; returns TOptional (user calls GetValue()). Same pattern as Queue. */
        [[nodiscard]] TOptional<ElementType>
        Receive()
        {
            while (True)
            {
                if (auto OptionalValue = Queue.Dequeue())
                { return std::move(OptionalValue); }
                ItemAvailable.WaitAndReset();
            }
        }

        [[nodiscard]] Bool
        IsEmpty() const { return Queue.IsEmpty(); }

        /** Returns TAsync<TOptional<T>>; suspends until an item is available (resumed by Send). User gets optional and GetValue(). */
        TAsync<TOptional<ElementType>>
        ReceiveAsync()
        {
            while (True)
            {
                if (auto OptionalValue = TryReceive())
                { co_return std::move(OptionalValue); }
                co_await FReceiveAwaiter{ this };
            }
        }

        /** Returns TAsync<void> so coroutines can co_await channel.SendAsync(...). */
        template<typename... ArgTypes> TAsync<void>
        SendAsync(ArgTypes&&... Args)
        {
            Send(std::forward<ArgTypes>(Args)...);
            co_return;
        }

    private:
        /** Awaiter for ReceiveAsync: avoids TOCTOU race (Send between TryReceive and Store) via double-check after storing handle. */
        struct FReceiveAwaiter
        {
            TSPSCChannel* Channel;
            /** If queue already has an item, skip suspension; next loop iteration will TryReceive() it. */
            Bool await_ready() const noexcept { return !Channel->Queue.IsEmpty(); }
            /** Store handle first, then double-check queue; if item arrived in the window, reclaim handle and return False to avoid sleeping forever. */
            Bool await_suspend(std::coroutine_handle<> Handle) noexcept
            {
                Channel->WaitingHandle.Store(Handle, EMemoryOrder::Release);
                if (!Channel->Queue.IsEmpty())
                {
                    if (Channel->WaitingHandle.Exchange(nullptr, EMemoryOrder::AcqRel) == Handle)
                    { return False; }
                    /* Send() already took the handle and will resume us; stay suspended. */
                }
                return True;
            }
            void await_resume() const noexcept {}
        };

        TSPSCQueue<ElementType>              Queue;
        FEvent                              ItemAvailable;
        TAtomic<std::coroutine_handle<>> WaitingHandle{ nullptr };

    public:
        TSPSCChannel() = default;
        TSPSCChannel(const TSPSCChannel&)            = delete;
        TSPSCChannel& operator=(const TSPSCChannel&) = delete;
        TSPSCChannel(TSPSCChannel&&)                 = delete;
        TSPSCChannel& operator=(TSPSCChannel&&)      = delete;
    };
}
