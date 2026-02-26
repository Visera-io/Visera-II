module;
#include <Visera-Core.hpp>
#include <coroutine>
export module Visera.Core.OS.Coroutine.Async;
#define VISERA_MODULE_NAME "Core.OS"
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

            TAsync get_return_object()
            {
                return TAsync{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }
            std::suspend_always initial_suspend() noexcept { return {}; }
            void return_value(T I_Value)
            {
                Value.Emplace(std::move(I_Value));
            }
            struct final_awaitable
            {
                std::coroutine_handle<> Continuation;
                Bool await_ready() const noexcept { return False; }
                Bool await_suspend(std::coroutine_handle<>) const noexcept
                {
                    if (Continuation) { Continuation.resume(); }
                    return True; // suspend this task so the resumed caller runs
                }
                void await_resume() const noexcept {}
            };
            final_awaitable final_suspend() noexcept { return { Continuation }; }
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

        /** Run the task to completion on the current thread and return the result. Blocks until done. */
        T Get()
        {
            if (Handle && !Handle.done())
            {
                while (!Handle.done()) { Handle.resume(); }
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

            TAsync get_return_object()
            {
                return TAsync{ std::coroutine_handle<promise_type>::from_promise(*this) };
            }
            std::suspend_always initial_suspend() noexcept { return {}; }
            void return_void() {}
            struct final_awaitable
            {
                std::coroutine_handle<> Continuation;
                Bool await_ready() const noexcept { return False; }
                Bool await_suspend(std::coroutine_handle<>) const noexcept
                {
                    if (Continuation) { Continuation.resume(); }
                    return True;
                }
                void await_resume() const noexcept {}
            };
            final_awaitable final_suspend() noexcept { return { Continuation }; }
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

        void Get()
        {
            if (Handle && !Handle.done())
            {
                while (!Handle.done()) { Handle.resume(); }
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
