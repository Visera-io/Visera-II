module;
#include <Visera-Core.hpp>
#include <thread>
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

        static void
        Sleep(UInt32 I_MilliSeconds)
        { std::this_thread::sleep_for(std::chrono::milliseconds(I_MilliSeconds)); }

        void
        Start(FFn I_Fn)
        {
            VISERA_ASSERT(!Worker.joinable());
            bStopRequested.Store(False, EMemoryOrder::Release);
            Worker = std::thread([this, Fn = std::move(I_Fn)]() mutable{ Fn(); });
        }

        void
        RequestStop()
        { bStopRequested.Store(True, EMemoryOrder::Release); }

        [[nodiscard]] Bool
        ShouldStop() const
        { return bStopRequested.Load(EMemoryOrder::Acquire); }

        void
        Join()
        { if (Worker.joinable()) { Worker.join(); } }

    private:
        std::thread      Worker;
        TAtomic<Bool> bStopRequested{False};

    public:
        FThread() = default;
        ~FThread() { Join(); }

        FThread(const FThread&) = delete;
        FThread& operator=(const FThread&) = delete;
        FThread(FThread&&) = delete;
        FThread& operator=(FThread&&) = delete;
    };
}