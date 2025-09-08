module;
#include <Visera-Core.hpp>
export module Visera.Core.Color;
#define VISERA_MODULE_NAME "Core.Color"
export import Visera.Core.Color.Common;
export import Visera.Core.Color.Linear;

export namespace Visera
{
    struct VISERA_CORE_API FColor
    {
    public:
        // Variables.
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
        union { struct { UInt8 B, G, R, A; }; UInt32 Bits{0}; };
#else
        union { struct { UInt8 A, R, G, B; }; UInt32 Bits{0}; };
#endif
        static const FColor White;
        static const FColor Black;
        static const FColor Transparent;
        static const FColor Red;
        static const FColor Green;
        static const FColor Blue;
        static const FColor Yellow;
        static const FColor Cyan;
        static const FColor Magenta;
        static const FColor Orange;
        static const FColor Purple;
        static const FColor Turquoise;
        static const FColor Silver;
        static const FColor Emerald;

        FColor() : R{ 0 }, G{ 0 }, B{ 0 }, A{ 0 } {};
        constexpr FColor(UInt8 I_Red, UInt8 I_Green, UInt8 I_Blue, UInt8 I_Alpha = 255)
            // put these into the body for proper ordering with INTEL vs non-INTEL_BYTE_ORDER
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
            : B(I_Blue), G(I_Green), R(I_Red), A(I_Alpha)
#else
            : A(I_Alpha), R(I_Red), G(I_Green), B(I_Blue)
#endif
        { }

        constexpr Bool operator==(const FColor I_Color) const { return Bits == I_Color.Bits; }
    };

    [[nodiscard]] inline VISERA_CORE_API FLinearColor
    CreateLinearColorFromPow22Color(FColor I_Color)
    {
        FLinearColor LinearColor{};
        LinearColor.R = FLinearColor::LUT_Pow22over255[I_Color.R];
        LinearColor.G = FLinearColor::LUT_Pow22over255[I_Color.G];
        LinearColor.B = FLinearColor::LUT_Pow22over255[I_Color.B];
        LinearColor.A = static_cast<Float>(I_Color.A) / 255.0f;  // Alpha is linear.

        return LinearColor;
    }

    const FColor FColor::White          (255,255,255);
    const FColor FColor::Black          (0,0,0);
    const FColor FColor::Transparent    (0, 0, 0, 0);
    const FColor FColor::Red            (255,0,0);
    const FColor FColor::Green          (0,255,0);
    const FColor FColor::Blue           (0,0,255);
    const FColor FColor::Yellow         (255,255,0);
    const FColor FColor::Cyan           (0,255,255);
    const FColor FColor::Magenta        (255,0,255);
    const FColor FColor::Orange         (243, 156, 18);
    const FColor FColor::Purple         (169, 7, 228);
    const FColor FColor::Turquoise      (26, 188, 156);
    const FColor FColor::Silver         (189, 195, 199);
    const FColor FColor::Emerald        (46, 204, 113);
}

namespace std
{
    template<>
    struct hash<Visera::FColor>
    {
        std::size_t operator()(const Visera::FColor I_Color) const noexcept
        {
            return static_cast<std::size_t>(I_Color.Bits);
        }
    };
}

template <>
struct fmt::formatter<Visera::FColor>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::FColor& I_Color, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        return fmt::format_to(
            I_Context.out(),
            "[R:{}, G:{}, B:{}, A:{}]",
            I_Color.R, I_Color.G, I_Color.B, I_Color.A
        );
    }
};