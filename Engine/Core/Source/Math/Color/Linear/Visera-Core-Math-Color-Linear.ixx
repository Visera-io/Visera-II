module;
#include <Visera-Core.hpp>
#include <initializer_list>
export module Visera.Core.Math.Color.Linear;
#define VISERA_MODULE_NAME "Core.Math"
import Visera.Core.Math.Color.Common;

export namespace Visera
{
    class VISERA_CORE_API FLinearColor
    {
    public:
        union
        {
            struct { Float R, G, B, A; };
            Float RGBA[4] {0.0f, 0.0f, 0.0f, 0.0f};
            Float Data[4];
        };

        [[nodiscard]] static constexpr FLinearColor
        White() noexcept { return FLinearColor{1.0f,1.0f,1.0f,1.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Gray() noexcept { return FLinearColor{0.5f,0.5f,0.5f,1.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Black() noexcept { return FLinearColor{0.0f,0.0f,0.0f,1.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Transparent() noexcept { return FLinearColor{0.0f,0.0f,0.0f,0.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Red() noexcept { return FLinearColor{1.0f,0.0f,0.0f,1.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Green() noexcept { return FLinearColor{0.0f,1.0f,0.0f,1.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Blue() noexcept { return FLinearColor{0.0f,0.0f,1.0f,1.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Yellow() noexcept { return FLinearColor{1.0f,1.0f,0.0f,1.0f}; }
        [[nodiscard]] static constexpr FLinearColor
        Purple() noexcept { return FLinearColor{0.3984f, 0.00213f, 0.7826f, 1.0f}; }

        [[nodiscard]] static constexpr Float
        SRGBToLinear(UInt8 I_SRGBColor) noexcept { return LUT_sRGBToLinear[I_SRGBColor]; }

    public:
        constexpr FLinearColor() noexcept = default;
        constexpr FLinearColor(const FLinearColor&) noexcept = default;
        constexpr FLinearColor(FLinearColor&&) noexcept = default;
        constexpr FLinearColor& operator=(const FLinearColor&) noexcept = default;
        constexpr FLinearColor& operator=(FLinearColor&&) noexcept = default;
        constexpr FLinearColor(Float I_Red, Float I_Green, Float I_Blue, Float I_Alpha = 1.0f) noexcept
        : R{ I_Red }, G{ I_Green }, B{ I_Blue }, A{ I_Alpha } {}
        constexpr FLinearColor(std::initializer_list<Float> I_List) noexcept
        : R{0}, G{0}, B{0}, A{1}
        {
            Float* P[] = { &R, &G, &B, &A };
            const Float* it = I_List.begin();
            const Float* end = I_List.end();
            for (UInt32 i = 0; i < 4 && it != end; ++i, ++it) { *P[i] = *it; }
        }
        constexpr FLinearColor(UInt8 I_SRGBRed, UInt8 I_SRGBGreen, UInt8 I_SRGBBlue, UInt8 I_Alpha = 255U) noexcept
        : R{ LUT_sRGBToLinear[I_SRGBRed] }, G{ LUT_sRGBToLinear[I_SRGBGreen] }, B{ LUT_sRGBToLinear[I_SRGBBlue] }, A{ I_Alpha / 255.0f } {}

        [[nodiscard]] constexpr Bool
        operator==(const FLinearColor& I_Rhs) const noexcept { return R==I_Rhs.R && G==I_Rhs.G && B==I_Rhs.B && A==I_Rhs.A; }
        [[nodiscard]] constexpr Bool
        operator!=(const FLinearColor& I_Rhs) const noexcept { return !(*this == I_Rhs); }
    };
    static_assert(sizeof(FLinearColor) == 16);
    static_assert(std::is_standard_layout_v<FLinearColor>);

}
VISERA_MAKE_FORMATTER(Visera::FLinearColor, {}, "[R:{}, G:{}, B:{}, A:{}]", I_Formatee.R, I_Formatee.G, I_Formatee.B, I_Formatee.A)