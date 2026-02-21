module;
#include <Visera-Core.hpp>
#include <thread>
#include <atomic>
export module Visera.Core.OS.Thread;
import Visera.Core.Types.Function;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.OS.Thread.Sync;
export import Visera.Core.OS.Thread.Queue;

export namespace Visera
{
    class VISERA_CORE_API FThread
    {
    public:
        using FFn = TFunction<void()>;

        void
        Start(FFn I_Fn)
        {
            VISERA_ASSERT(!Worker.joinable());
            bStopRequested.store(False, std::memory_order_release);
            Worker = std::thread([this, Fn = std::move(I_Fn)]() mutable{ Fn(); });
        }

        void
        RequestStop()
        { bStopRequested.store(True, std::memory_order_release); }

        [[nodiscard]] Bool
        ShouldStop() const
        { return bStopRequested.load(std::memory_order_acquire); }

        void
        Join()
        { if (Worker.joinable()) { Worker.join(); } }

    private:
        std::thread      Worker;
        std::atomic_bool bStopRequested{False};

    public:
        FThread() = default;
        ~FThread() { Join(); }

        FThread(const FThread&) = delete;
        FThread& operator=(const FThread&) = delete;
        FThread(FThread&&) = delete;
        FThread& operator=(FThread&&) = delete;
    };
}