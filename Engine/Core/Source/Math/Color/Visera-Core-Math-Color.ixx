module;
#include <Visera-Core.hpp>
#include <initializer_list>
export module Visera.Core.Math.Color;
#define VISERA_MODULE_NAME "Core.Math"
export import Visera.Core.Math.Color.Linear;
export import Visera.Core.Math.Color.Common;
       import Visera.Core.Math.Arithmetic.Operation;

export namespace Visera
{
    namespace Concepts
    {
        /**
         * Concept that defines a color type with R, G, B, A components.
         * Used to constrain template parameters for color operations.
         */
        template<typename TColor>
        concept Color = requires(TColor I_Color)
        {
            I_Color.R;
            I_Color.G;
            I_Color.B;
            I_Color.A;
        };
    }

    class VISERA_CORE_API FColor
    {
    public:
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
        union { struct { UInt8 B, G, R, A; }; UInt32 Bits; UInt8 Data[4]; };
#else
        union { struct { UInt8 A, R, G, B; }; UInt32 Bits; UInt8 Data[4]; };
#endif

        static inline FColor
        SRGB8ColorFromLinear(const FLinearColor& I_LinearColor)
        {
            static auto ConvertRGB = [](Float I_Value) -> UInt8
            {
                const Float  Clamped = Math::Clamp(I_Value, 0.0f, 1.0f);
                Float SRGB = Clamped <= 0.0031308f?
                             Clamped * 12.92f
                             :
                             1.055f * Math::Pow(Clamped, 1.0f / 2.4f) - 0.055f;
                return static_cast<UInt8>(SRGB * 255.0f + 0.5f);
            };
            FColor SRGB8Color{};
            SRGB8Color.R = ConvertRGB(I_LinearColor.R);
            SRGB8Color.G = ConvertRGB(I_LinearColor.G);
            SRGB8Color.B = ConvertRGB(I_LinearColor.B);
            // Alpha is always linear
            SRGB8Color.A = static_cast<UInt8>(Math::Clamp(I_LinearColor.A, 0.0f, 1.0f) * 255.0f + 0.5f);
            return SRGB8Color;
        }

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
        constexpr FColor(const FColor&) noexcept = default;
        constexpr FColor(FColor&&) noexcept = default;
        constexpr FColor& operator=(const FColor&) noexcept = default;
        constexpr FColor& operator=(FColor&&) noexcept = default;
        constexpr FColor(std::initializer_list<UInt8> I_List) noexcept
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
            : B(0), G(0), R(0), A(255)
#else
            : A(255), R(0), G(0), B(0)
#endif
        {
            const UInt8* it = I_List.begin();
            const UInt8* end = I_List.end();
#if defined(VISERA_ON_LITTLE_ENDIAN_PLATFORM)
            if (it != end) { R = *it++; } if (it != end) { G = *it++; } if (it != end) { B = *it++; } if (it != end) { A = *it; }
#else
            if (it != end) { R = *it++; } if (it != end) { G = *it++; } if (it != end) { B = *it++; } if (it != end) { A = *it; }
#endif
        }
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
}
VISERA_MAKE_HASH(Visera::FColor, return I_Object.Bits;);
VISERA_MAKE_FORMATTER(Visera::FColor, {}, "[R:{}, G:{}, B:{}, A:{}]", I_Formatee.R, I_Formatee.G, I_Formatee.B, I_Formatee.A)