module;
#include <Visera-Core.hpp>
export module Visera.Core.Math.Bit;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera
{
    namespace Math
    {
        template<Concepts::Integeral NumT> Bool
        IsPowerOfTwo(NumT I_Number) { return (I_Number > 0) && ((I_Number & (I_Number - 1)) == 0); }

        template<Concepts::UnsingedIntegeral NumT> constexpr Int32
        PopCount(NumT I_Number) noexcept { return std::popcount(I_Number); }

        template<Concepts::UnsingedIntegeral NumT> constexpr Int32
        CountLeadingZeros(NumT I_Number) noexcept { return std::countl_zero(I_Number); }

        template<Concepts::UnsingedIntegeral NumT> constexpr Int32
        CountTrailingZeros(NumT I_Number) noexcept { return std::countr_zero(I_Number); }

        template<Concepts::UnsingedIntegeral NumT> constexpr Int32
        BitWidth(NumT I_Number) noexcept { return std::bit_width(I_Number); }

        template<Concepts::UnsingedIntegeral NumT> constexpr bool
        HasSingleBit(NumT I_Number) noexcept { return std::has_single_bit(I_Number); }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        BitCeil(NumT I_Number) noexcept { return std::bit_ceil(I_Number); }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        BitFloor(NumT I_Number) noexcept { return std::bit_floor(I_Number); }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        RotateLeft(NumT I_Number, Int32 r) noexcept { return std::rotl(I_Number, r); }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        RotateRight(NumT I_Number, Int32 r) noexcept { return std::rotr(I_Number, r); }

        template<class NumT> constexpr NumT
        ByteSwap(NumT I_Number) noexcept { return std::byteswap(I_Number); }

        template<class To, class From> constexpr To
        BitCast(const From& f) noexcept { return std::bit_cast<To>(f); }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        AlignUp(NumT I_Number, NumT I_Alignment) noexcept
        {  return (I_Number + I_Alignment - 1) / I_Alignment * I_Alignment; }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        AlignDown(NumT I_Number, NumT I_Alignment) noexcept
        {  return I_Number / I_Alignment * I_Alignment; }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        AlignUpPow2(NumT I_Number, NumT I_Alignment) noexcept
        { return (I_Number + I_Alignment - 1) & ~(I_Alignment - 1); }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        MakeMask(Int32 offset, Int32 width) noexcept
        {
            if (width <= 0) return 0;
            if (width >= Int32(sizeof(NumT) * 8)) return ~NumT{0};
            return ( (NumT{1} << width) - 1 ) << offset;
        }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        ExtractBits(NumT value, Int32 offset, Int32 width) noexcept
        { return (value >> offset) & ((NumT{1} << width) - 1); }

        template<Concepts::UnsingedIntegeral NumT> constexpr NumT
        InsertBits(NumT base, NumT insert, Int32 offset, Int32 width) noexcept
        {
            NumT mask = MakeMask<NumT>(offset, width);
            return (base & ~mask) | ((insert << offset) & mask);
        }
    }
}