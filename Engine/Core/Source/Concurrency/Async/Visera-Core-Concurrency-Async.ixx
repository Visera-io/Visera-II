module;
#include <Visera-Core.hpp>
#include <coroutine>
export module Visera.Core.Concurrency.Async;
#define VISERA_MODULE_NAME "Core.Concurrency"
import Visera.Core.OS.Thread.Sync.Event;
import Visera.Core.Types.Optional;

export namespace Visera
{
    /** C++20 coroutine task type. Lazy: the coroutine body runs when the task is co_await'ed (or when Run()/Get() is used). */
    template<typename T>
    class VISERA_CORE_API TAsync
    {
    public:
        struct promise_type
        {
            TOptional<T>            Value;
            std::coroutine_handle<> Continuation;
            FEvent                  DoneEvent;

            TAsync get_return_object()
            {
                return TAsync{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }
            std::suspend_always initial_suspend() noexcept { return {}; }
            void return_value(T I_Value)
            {
                Value.Emplace(std::move(I_Value));
            }
            /** Trigger before resume so Get() waiters are woken while this frame is still valid; resuming the continuation may destroy the TAsync and this frame. */
            struct final_awaitable
            {
                std::coroutine_handle<> Continuation;
                promise_type*           Promise;
                Bool await_ready() const noexcept { return False; }
                Bool await_suspend(std::coroutine_handle<>) const noexcept
                {
                    Promise->DoneEvent.Trigger();
                    if (Continuation) { Continuation.resume(); }
                    return True;
                }
                void await_resume() const noexcept {}
            };
            final_awaitable final_suspend() noexcept { return { Continuation, this }; }
            void unhandled_exception() { throw; }
        };

        /** Awaitable so that co_await async_task resumes this task and yields the result. */
        struct awaitable
        {
            std::coroutine_handle<promise_type> Handle;
            Bool await_ready() const noexcept
            {
                return !Handle || Handle.done();
            }
            std::coroutine_handle<> await_suspend(std::coroutine_handle<> I_Continuation) noexcept
            {
                Handle.promise().Continuation = I_Continuation;
                Handle.resume();
                return std::noop_coroutine();
            }
            T await_resume()
            {
                return std::move(Handle.promise().Value).GetValue();
            }
        };

        awaitable operator co_await() const& noexcept
        {
            return awaitable{ Handle };
        }
        awaitable operator co_await() && noexcept
        {
            return awaitable{ Handle };
        }

        /** Run the task to completion and return the result. Blocks until done. One resume() to start; then Wait() once for DoneEvent (no loop, so no double-resume if another thread drives the coroutine). */
        T Get()
        {
            if (Handle && !Handle.done())
            {
                promise_type& Promise = Handle.promise();
                Handle.resume();
                if (!Handle.done())
                { Promise.DoneEvent.Wait(); }
            }
            if (Handle && Handle.promise().Value.HasValue())
            {
                return std::move(Handle.promise().Value).GetValue();
            }
            return T{};
        }

        [[nodiscard]] Bool IsDone() const noexcept
        {
            return !Handle || Handle.done();
        }

        TAsync() noexcept : Handle(nullptr) {}
        explicit TAsync(std::coroutine_handle<promise_type> I_Handle) noexcept : Handle(I_Handle) {}
        TAsync(TAsync&& I_Other) noexcept : Handle(I_Other.Handle) { I_Other.Handle = nullptr; }
        TAsync& operator=(TAsync&& I_Other) noexcept
        {
            if (Handle) { Handle.destroy(); }
            Handle = I_Other.Handle;
            I_Other.Handle = nullptr;
            return *this;
        }
        ~TAsync()
        {
            if (Handle) { Handle.destroy(); }
        }
        TAsync(const TAsync&) = delete;
        TAsync& operator=(const TAsync&) = delete;

    private:
        std::coroutine_handle<promise_type> Handle;
    };

    /** void specialization: no return value. */
    template<>
    class VISERA_CORE_API TAsync<void>
    {
    public:
        struct promise_type
        {
            std::coroutine_handle<> Continuation;
            FEvent                 DoneEvent;

            TAsync get_return_object()
            {
                return TAsync{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }
            std::suspend_always initial_suspend() noexcept { return {}; }
            void return_void() {}
            /** Trigger before resume so Get() waiters are woken while this frame is still valid; resuming the continuation may destroy the TAsync and this frame. */
            struct final_awaitable
            {
                std::coroutine_handle<> Continuation;
                promise_type*           Promise;
                Bool await_ready() const noexcept { return False; }
                Bool await_suspend(std::coroutine_handle<>) const noexcept
                {
                    Promise->DoneEvent.Trigger();
                    if (Continuation) { Continuation.resume(); }
                    return True;
                }
                void await_resume() const noexcept {}
            };
            final_awaitable final_suspend() noexcept { return { Continuation, this }; }
            void unhandled_exception() { throw; }
        };

        struct awaitable
        {
            std::coroutine_handle<promise_type> Handle;
            Bool await_ready() const noexcept { return !Handle || Handle.done(); }
            std::coroutine_handle<> await_suspend(std::coroutine_handle<> I_Continuation) noexcept
            {
                Handle.promise().Continuation = I_Continuation;
                Handle.resume();
                return std::noop_coroutine();
            }
            void await_resume() const noexcept {}
        };

        awaitable operator co_await() const& noexcept { return awaitable{ Handle }; }
        awaitable operator co_await() && noexcept { return awaitable{ Handle }; }

        /** Run the task to completion. One resume() to start; then Wait() once for DoneEvent (no loop, so no double-resume if another thread drives the coroutine). */
        void Get()
        {
            if (Handle && !Handle.done())
            {
                promise_type& Promise = Handle.promise();
                Handle.resume();
                if (!Handle.done())
                { Promise.DoneEvent.Wait(); }
            }
        }

        [[nodiscard]] Bool IsDone() const noexcept { return !Handle || Handle.done(); }

        TAsync() noexcept : Handle(nullptr) {}
        explicit TAsync(std::coroutine_handle<promise_type> I_Handle) noexcept : Handle(I_Handle) {}
        TAsync(TAsync&& I_Other) noexcept : Handle(I_Other.Handle) { I_Other.Handle = nullptr; }
        TAsync& operator=(TAsync&& I_Other) noexcept
        {
            if (Handle) { Handle.destroy(); }
            Handle = I_Other.Handle;
            I_Other.Handle = nullptr;
            return *this;
        }
        ~TAsync() { if (Handle) { Handle.destroy(); } }
        TAsync(const TAsync&) = delete;
        TAsync& operator=(const TAsync&) = delete;

    private:
        std::coroutine_handle<promise_type> Handle;
    };
}
