module;
#include <Visera-Core.hpp>
#include <chrono>
export module Visera.Core.OS.Time.Duration;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.OS.Time.Common;

export namespace Visera
{
    template<Concepts::Clock T>
    class VISERA_CORE_API TDuration
    {
    public:
        [[nodiscard]] Double
        Microseconds() const { return std::chrono::duration<Double, std::micro>(Value).count(); }
        [[nodiscard]] Double
        Milliseconds() const { return std::chrono::duration<Double, std::milli>(Value).count(); }
        [[nodiscard]] Double
        Seconds()      const { return std::chrono::duration<Double>(Value).count(); }
        [[nodiscard]] Double
        Minutes()      const { return std::chrono::duration<Double, std::ratio<60>>(Value).count(); }
        [[nodiscard]] Double
        Hours()        const { return std::chrono::duration<Double, std::ratio<3600>>(Value).count(); }
        [[nodiscard]] Double
        Days()         const { return std::chrono::duration<Double, std::ratio<86400>>(Value).count(); }

    public:
        TDuration() noexcept : Value{ 0 } {};
        TDuration(const TDuration<T>& I_Duration)         : Value{ I_Duration.Value } {}
        TDuration(TDuration<T>&& I_Duration) noexcept     : Value{ std::move(I_Duration.Value) } {}
        explicit TDuration(const T::duration& I_Duration) : Value{ I_Duration } {}
        explicit TDuration(T::duration&& I_Duration)      : Value{ std::move(I_Duration) } {}
        TDuration(T::time_point Start, T::time_point End) : Value{ End - Start } {}
        //TDuration<T>& operator=(const T::duration& TDuration) { Value = TDuration; return *this; }
        TDuration<T>& operator=(T::duration&& I_Duration) { Value = I_Duration; return *this; }

        TDuration<T>  operator+ (const TDuration<T>&  Addend)     const  { return TDuration<T>{Value + Addend.Value}; }
        TDuration<T>& operator+=(const TDuration<T>&  Addend)            { Value += Addend.Value; return *this; }
        TDuration<T>  operator- (const TDuration<T>&  Subtrahend) const  { return {Value - Subtrahend.Value}; }
        TDuration<T>& operator-=(const TDuration<T>&  Subtrahend)        { Value -= Subtrahend.Value; return *this; }
    private:
        T::duration Value;
    };
}
