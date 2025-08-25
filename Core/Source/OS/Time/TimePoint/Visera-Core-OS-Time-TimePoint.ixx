module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.Time.TimePoint;
#define VISERA_MODULE_NAME "Core.OS"
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
        TTimePoint() = default;
        explicit TTimePoint(const std::chrono::high_resolution_clock::time_point& TTimePoint) noexcept : Value{ TTimePoint }{}
        explicit TTimePoint(std::chrono::high_resolution_clock::time_point&& TTimePoint) noexcept : Value{ TTimePoint }{}

        operator std::chrono::high_resolution_clock::time_point() const { return Value; }
        auto&   operator=(const TTimePoint<std::chrono::high_resolution_clock>& target) { Value = target.Value; return *this; }
        auto&   operator=(TTimePoint<std::chrono::high_resolution_clock>&& target) noexcept      { Value = target.Value; return *this; }
        auto    operator-(const std::chrono::high_resolution_clock::time_point& target) const { return TDuration<std::chrono::high_resolution_clock>{ Value - target }; }
        auto    operator-(const TTimePoint<std::chrono::high_resolution_clock>& target)  const { return TDuration<std::chrono::high_resolution_clock>{ Value - target.Value }; }
    private:
        std::chrono::high_resolution_clock::time_point Value;
    };

    template<>
    class VISERA_CORE_API TTimePoint<std::chrono::system_clock>
    {
    public:
        //[TODO] Time Zone issues.
        [[nodiscard]] FString ToString() const
        { return std::format("UTC(+0) {:%Y-%m-%d %H:%M:%S}", Value); }

    public:
        TTimePoint() = default; //UNIX Time
        TTimePoint(std::chrono::system_clock::time_point Value) : Value{ std::move(Value) }{}
        TTimePoint(const TTimePoint<std::chrono::system_clock>& I_NewTimePoint) : Value{ I_NewTimePoint.Value }{}

        operator std::chrono::system_clock::time_point() const { return Value; }
        auto&   operator=(const TTimePoint<std::chrono::system_clock>& target) { Value = target.Value; return *this; }
        auto&   operator=(TTimePoint<std::chrono::system_clock>&& target)      { Value = target.Value; return *this; }
        auto    operator-(const std::chrono::system_clock::time_point& target) const { return TDuration<std::chrono::system_clock>{ Value - target }; }
        auto    operator-(const TTimePoint<std::chrono::system_clock>& target) const { return TDuration<std::chrono::system_clock>{ Value - target.Value }; }
    private:
        std::chrono::system_clock::time_point Value;
    };
}
