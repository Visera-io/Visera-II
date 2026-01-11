module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Time.TimePoint;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.OS.Time.Common;
import Visera.Core.OS.Time.Duration;

export namespace Visera
{
    template<Concepts::Clock T>
    class VISERA_CORE_API TTimePoint
    {
    public:
        TTimePoint() = default;
        TTimePoint(const T::time_point& I_NewTimePoint) : Value{ I_NewTimePoint }{}
        TTimePoint(T::time_point&& I_NewTimePoint) : Value{ std::move(I_NewTimePoint) } {}
    private:
        T::time_point Value;
    };

    template<>
    class VISERA_CORE_API TTimePoint<std::chrono::high_resolution_clock>
    {
    public:
        using clock      = std::chrono::high_resolution_clock;
        using time_point = clock::time_point;

        TTimePoint() = default;
        explicit TTimePoint(const time_point& tp) noexcept : Value{ tp } {}
        explicit TTimePoint(time_point&& tp) noexcept : Value{ std::move(tp) } {}

        TTimePoint(const TTimePoint&)            = default;
        TTimePoint(TTimePoint&&) noexcept        = default;
        TTimePoint& operator=(const TTimePoint&) = default;
        TTimePoint& operator=(TTimePoint&&) noexcept = default;

        operator time_point() const noexcept { return Value; }

        auto operator-(const time_point& target) const
        { return TDuration<clock>{ Value - target }; }

        auto operator-(const TTimePoint& target) const
        { return TDuration<clock>{ Value - target.Value }; }

    private:
        time_point Value{};
    };

    template<>
    class VISERA_CORE_API TTimePoint<std::chrono::system_clock>
    {
    public:
        using clock      = std::chrono::system_clock;
        using time_point = clock::time_point;

        [[nodiscard]] FString ToString() const
        { return std::format("UTC(+0) {:%Y-%m-%d %H:%M:%S}", Value); }

        [[nodiscard]] inline time_t
        ToSystemTimeType() const { return clock::to_time_t(Value); }
        [[nodiscard]] inline TDuration<clock>
        GetTimeFromEpoch() const { return TDuration<clock>{ Value.time_since_epoch() }; }

        TTimePoint() = default;
        explicit TTimePoint(time_point tp) : Value{ std::move(tp) } {}

        // defaults are fine
        TTimePoint(const TTimePoint&)            = default;
        TTimePoint(TTimePoint&&) noexcept        = default;
        TTimePoint& operator=(const TTimePoint&) = default;
        TTimePoint& operator=(TTimePoint&&) noexcept = default;

        operator time_point() const noexcept { return Value; }

        auto operator-(const time_point& target) const
        { return TDuration<clock>{ Value - target }; }
        auto operator-(const TTimePoint& target) const
        { return TDuration<clock>{ Value - target.Value }; }

    private:
        time_point Value{};
    };
}
