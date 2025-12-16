module;
#include <Visera-Core.hpp>
export module Visera.Core.Traits.Flags;
#define VISERA_MODULE_NAME "Core.Traits"

export namespace Visera
{
    // =============================
    //  Enum class bitmask support (macro: VISERA_MAKE_FLAGS)
    // =============================
    template<class E> struct
    TEnableBitMaskOperators : std::false_type {};

    template<class E> inline constexpr Bool
    EnableBitMaskOperators = TEnableBitMaskOperators<E>::value;

    template<class E> constexpr auto
    ToUnderlying(E I_Enum) noexcept { return static_cast<std::underlying_type_t<E>>(I_Enum); }

    template<class E> struct
    TBitTest
    {
        using U = std::underlying_type_t<E>;
        U Value{};
        constexpr explicit operator Bool() const noexcept { return Value != 0; }
    };

    template<class E> requires EnableBitMaskOperators<E> constexpr
    E operator | (E I_EnumA, E I_EnumB) noexcept
    { return static_cast<E>(ToUnderlying(I_EnumA) | ToUnderlying(I_EnumB)); }

    template<class E> requires EnableBitMaskOperators<E> constexpr
    E operator ^ (E I_EnumA, E I_EnumB) noexcept
    { return static_cast<E>(ToUnderlying(I_EnumA) ^ ToUnderlying(I_EnumB)); }

    template<class E> requires EnableBitMaskOperators<E> constexpr
    E operator ~ (E I_EnumA) noexcept
    { return static_cast<E>(~ToUnderlying(I_EnumA)); }

    // return TBitTest to allow: if (U & Flag)
    template<class E> requires EnableBitMaskOperators<E> constexpr
    TBitTest<E> operator & (E I_EnumA, E I_EnumB) noexcept
    { return TBitTest<E>{ ToUnderlying(I_EnumA) & ToUnderlying(I_EnumB) }; }

    template<class E> requires EnableBitMaskOperators<E> constexpr
    E& operator |= (E& I_EnumA, E I_EnumB) noexcept { return I_EnumA = I_EnumA | I_EnumB; }

    template<class E> requires EnableBitMaskOperators<E> constexpr
    E& operator ^= (E& I_EnumA, E I_EnumB) noexcept { return I_EnumA = I_EnumA ^ I_EnumB; }

    template<class E> requires EnableBitMaskOperators<E> constexpr
    E& operator &= (E& I_EnumA, E I_EnumB) noexcept
    { return I_EnumA = static_cast<E>(ToUnderlying(I_EnumA) & ToUnderlying(I_EnumB)); }
}
