module;
#include <Visera-Core.hpp>
export module Visera.Core.Color;
#define VISERA_MODULE_NAME "Core.Color"
export import Visera.Core.Color.Linear;
       import Visera.Core.Color.Common;

export namespace Visera
{
    class VISERA_CORE_API FColor
    {
    public:
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
        union { struct { UInt8 B, G, R, A; }; UInt32 Bits; UInt8 Data[4]; };
#else
        union { struct { UInt8 A, R, G, B; }; UInt32 Bits; UInt8 Data[4]; };
#endif

        [[nodiscard]] static constexpr FColor
        White() noexcept { return FColor{255,255,255,255}; }
        [[nodiscard]] static constexpr FColor
        Black() noexcept { return FColor{0,0,0,255}; }
        [[nodiscard]] static constexpr FColor
        Transparent() noexcept { return FColor{0,0,0,0}; }
        [[nodiscard]] static constexpr FColor
        Red() noexcept { return FColor{255,0,0,255}; }
        [[nodiscard]] static constexpr FColor
        Green() noexcept { return FColor{0,255,0,255}; }
        [[nodiscard]] static constexpr FColor
        Blue() noexcept { return FColor{0,0,255,255}; }
        [[nodiscard]] static constexpr FColor
        Yellow() noexcept { return FColor{255,255,0,255}; }
        [[nodiscard]] static constexpr FColor
        Cyan() noexcept { return FColor{0,255,255,255}; }
        [[nodiscard]] static constexpr FColor
        Magenta() noexcept { return FColor{255,0,255,255}; }
        [[nodiscard]] static constexpr FColor
        Orange() noexcept { return FColor{243,156,18,255}; }
        [[nodiscard]] static constexpr FColor
        Purple() noexcept { return FColor{169,7,228,255}; }
        [[nodiscard]] static constexpr FColor
        Turquoise() noexcept { return FColor{26,188,156,255}; }
        [[nodiscard]] static constexpr FColor
        Silver() noexcept { return FColor{189,195,199,255}; }
        [[nodiscard]] static constexpr FColor
        Emerald() noexcept { return FColor{46,204,113,255}; }

        constexpr FColor() noexcept
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
            : B(0), G(0), R(0), A(0)
#else
            : A(0), R(0), G(0), B(0)
#endif
        {}

        constexpr FColor(UInt8 I_Red, UInt8 I_Green, UInt8 I_Blue, UInt8 I_Alpha = 255) noexcept
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
            : B(I_Blue), G(I_Green), R(I_Red), A(I_Alpha)
#else
            : A(I_Alpha), R(I_Red), G(I_Green), B(I_Blue)
#endif
        {}

        [[nodiscard]] constexpr Bool
        operator==(const FColor& I_Color) const noexcept { return Bits == I_Color.Bits; }

        [[nodiscard]] constexpr Bool
        operator!=(const FColor& I_Color) const noexcept { return Bits != I_Color.Bits; }
    };
    static_assert(sizeof(FColor) == 4);
    static_assert(std::is_standard_layout_v<FColor>);
    static_assert(std::is_trivially_copyable_v<FColor>);

    [[nodiscard]] inline VISERA_CORE_API FLinearColor
    CreateLinearColorFromPow22Color(const FColor& I_Color) noexcept
    {
        FLinearColor LinearColor{};
        LinearColor.R = FLinearColor::LUT_Pow22over255[I_Color.R];
        LinearColor.G = FLinearColor::LUT_Pow22over255[I_Color.G];
        LinearColor.B = FLinearColor::LUT_Pow22over255[I_Color.B];
        LinearColor.A = static_cast<Float>(I_Color.A) / 255.0f;
        return LinearColor;
    }
}
VISERA_MAKE_HASH(Visera::FColor, return static_cast<std::size_t>(I_Object.Bits););
VISERA_MAKE_FORMATTER(Visera::FColor, {}, "[R:{}, G:{}, B:{}, A:{}]_Color", I_Formatee.R, I_Formatee.G, I_Formatee.B, I_Formatee.A)