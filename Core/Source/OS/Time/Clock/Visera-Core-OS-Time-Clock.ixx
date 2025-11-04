module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Time.Clock;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.OS.Time.TimePoint;
import Visera.Core.OS.Time.Duration;

//[Template]	TClock
//[Alias]		FHiResClock
//[Alias]		FSystemClock

export namespace Visera
{
    template<Concepts::Clock T>
    class VISERA_CORE_API TClock
    {
    public:
        static inline
        TTimePoint<T>
        Now() { return TTimePoint<T>{ T::now() }; }

        inline
        TDuration<T>
        Tick() { auto OldTime = LastTickTimePoint; LastTickTimePoint = Now(); return TDuration<T>{ LastTickTimePoint - OldTime }; }

        inline
        TDuration<T>
        Elapsed() const { return TDuration<T>{ Now() - LastTickTimePoint }; }

        inline
        TDuration<T>
        GetTotalTime() const { return TDuration<T>{ Now() - StartTimePoint }; }

        inline void
        Set(TTimePoint<T> NewTimePoint) { LastTickTimePoint = NewTimePoint; }

        inline void
        Reset() { StartTimePoint = Now(); LastTickTimePoint = StartTimePoint; }

    public:
        TClock() noexcept : StartTimePoint{ Now() } { LastTickTimePoint = StartTimePoint; }

    protected:
        TTimePoint<T> StartTimePoint;
        TTimePoint<T> LastTickTimePoint;
    };

    using FHiResClock  = TClock<std::chrono::high_resolution_clock>;
    using FSystemClock = TClock<std::chrono::system_clock>;
}
