module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Random.RNG.PCG;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Arithmetic;

export namespace Visera
{
    class VISERA_CORE_API FPCG32
    {
        static constexpr auto PCG32DefaultState     = 0x853c49e6748fea9bULL;
        static constexpr auto PCG32DefaultIncrement = 0xda3e39cb94b95bdbULL;
        static constexpr auto PCG32Multiplier       = 0x5851f42d4c957f2dULL;

    public:
        template<Concepts::Arithmetical T = Float> [[nodiscard]] T
        Uniform()
        {
            if constexpr (std::is_same_v<T, UInt32>)
            {
                UInt64  Oldstate = State;
                State = Oldstate * PCG32Multiplier + PCG32DefaultIncrement;
                auto XORShifted = static_cast<UInt32>(((Oldstate >> 18u) ^ Oldstate) >> 27u);
                auto Rot        = static_cast<UInt32>(Oldstate >> 59u);
                return (XORShifted >> Rot) | (XORShifted << ((~Rot + 1u) & 31));
            }
            else if constexpr (std::is_same_v<T, UInt64>)
            {
                UInt64 V0 = Uniform<UInt32>();
                UInt64 V1 = Uniform<UInt32>();
                return (V0 << 32) | V1;
            }
            else if constexpr (std::is_same_v<T, Int32>)
            {
                // https://stackoverflow.com/a/13208789
                UInt32 V = Uniform<UInt32>();
                if (V <= static_cast<UInt32>(Math::UpperBound<Int32>()))
                { return static_cast<Int32>(V); }

                VISERA_ASSERT(V >= static_cast<UInt32>(std::numeric_limits<Int32>::min()));

                return V - Math::UpperBound<Int32>() + Math::LowerBound<Int32>();
            }
            else if constexpr (std::is_same_v<T, int64_t>)
            {
                // https://stackoverflow.com/a/13208789
                UInt64 V = Uniform<UInt64>();
                if (V <= (UInt64)Math::UpperBound<Int64>())
                    // Safe to type convert directly.
                        return int64_t(V);
                VISERA_ASSERT(V >= (UInt64)Math::LowerBound<Int64>());
                return int64_t(V - Math::LowerBound<Int64>()) +
                       Math::LowerBound<Int64>();
            }
            else if constexpr (std::is_same_v<T, Float>)
            {
                return Math::Min<Float>(1.0f - Math::Epsilon<Float>(), Uniform<UInt32>() * 0x1p-32f);
            }
            else if constexpr (std::is_same_v<T, Double>)
            {
                return Math::Min<Double>(1.0 - Math::Epsilon<Double>(), Uniform<UInt64>() * 0x1p-64);
            }
            else
            {
                static_assert(std::is_same_v<T, void>, "FPCG32::Uniform<T>() unsupported type.");
            }
        }

        void SetSequence(UInt64 I_SequenceIndex, UInt32 I_Seed);

        FPCG32() = default;
        FPCG32(UInt64 I_SequenceIndex, UInt32 I_Seed) { SetSequence(I_SequenceIndex, I_Seed); }

    private:
        UInt64 State      {PCG32DefaultState};
        UInt64 Increment  {PCG32DefaultIncrement};
    };

    void FPCG32::
    SetSequence(UInt64 I_SequenceIndex/* = GetRandomSeed() */,
                UInt32 I_Seed         /* = GetRandomSeed() */)
    {
        // I_SequenceIndex = I_SequenceIndex? I_SequenceIndex : GetRandomSeed();
        // I_Seed = I_Seed? I_Seed : GetRandomSeed();
        State = 0U;
        Increment = (I_SequenceIndex << 1U) | 1U;
        (void)Uniform<UInt32>();
        State += I_Seed;
        (void)Uniform<UInt32>();
    }
} // namespace Visera