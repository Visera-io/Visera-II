module;
#include <Visera-Core.hpp>
#include <bit>
export module Visera.Core.Math.Bit;
#define VISERA_MODULE_NAME "Core.Math"

export namespace Visera
{
    namespace Math
    {
        template<Concepts::Integral NumT> Bool
        IsPowerOfTwo(NumT I_Number) { return (I_Number > 0) && ((I_Number & (I_Number - 1)) == 0); }

        template<Concepts::UnsingedIntegral NumT> constexpr Int32
        PopCount(NumT I_Number) noexcept { return std::popcount(I_Number); }

        template<Concepts::UnsingedIntegral NumT> constexpr Int32
        CountLeadingZeros(NumT I_Number) noexcept { return std::countl_zero(I_Number); }

        template<Concepts::UnsingedIntegral NumT> constexpr Int32
        CountTrailingZeros(NumT I_Number) noexcept { return std::countr_zero(I_Number); }

        template<Concepts::UnsingedIntegral NumT> constexpr Int32
        BitWidth(NumT I_Number) noexcept { return std::bit_width(I_Number); }

        template<Concepts::UnsingedIntegral NumT> constexpr bool
        HasSingleBit(NumT I_Number) noexcept { return std::has_single_bit(I_Number); }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        BitCeil(NumT I_Number) noexcept { return std::bit_ceil(I_Number); }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        BitFloor(NumT I_Number) noexcept { return std::bit_floor(I_Number); }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        RotateLeft(NumT I_Number, Int32 I_Rotation) noexcept { return std::rotl(I_Number, I_Rotation); }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        RotateRight(NumT I_Number, Int32 I_Rotation) noexcept { return std::rotr(I_Number, I_Rotation); }

        template<class NumT> constexpr NumT
        ByteSwap(NumT I_Number) noexcept { return std::byteswap(I_Number); }

        template<class To, class From> constexpr To
        BitCast(const From& I_Source) noexcept { return std::bit_cast<To>(I_Source); }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        AlignUp(NumT I_Number, NumT I_Alignment) noexcept
        {  return (I_Number + I_Alignment - 1) / I_Alignment * I_Alignment; }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        AlignDown(NumT I_Number, NumT I_Alignment) noexcept
        {  return I_Number / I_Alignment * I_Alignment; }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        AlignUpPow2(NumT I_Number, NumT I_Alignment) noexcept
        { return (I_Number + I_Alignment - 1) & ~(I_Alignment - 1); }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        MakeMask(Int32 I_Offset, Int32 I_Width) noexcept
        {
            if (I_Width <= 0) return 0;
            if (I_Width >= Int32(sizeof(NumT) * 8)) return ~NumT{0};
            return ( (NumT{1} << I_Width) - 1 ) << I_Offset;
        }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        ExtractBits(NumT I_Value, Int32 I_Offset, Int32 I_Width) noexcept
        { return (I_Value >> I_Offset) & ((NumT{1} << I_Width) - 1); }

        template<Concepts::UnsingedIntegral NumT> constexpr NumT
        InsertBits(NumT I_Base, NumT I_Insert, Int32 I_Offset, Int32 I_Width) noexcept
        {
            NumT Mask = MakeMask<NumT>(I_Offset, I_Width);
            return (I_Base & ~Mask) | ((I_Insert << I_Offset) & Mask);
        }
    }
}