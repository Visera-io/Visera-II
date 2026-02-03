module;
#include <Visera-Core.hpp>
#include <simdutf.h>
#include <double-conversion/double-conversion.h>
export module Visera.Core.Types.Text;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Array;

export namespace Visera
{
    /* UTF8 Encoded String */
    class VISERA_CORE_API FText
    {
    public:
        [[nodiscard]] static Bool
        ValidateUTF8(FStringView I_String) { return simdutf::validate_utf8(I_String); }

        [[nodiscard]] const char*
        GetData()   const { return String.data(); }
        [[nodiscard]] UInt64
        GetSize() const { return String.size(); }
        [[nodiscard]] UInt64
        GetCodepointCount() const noexcept; // 👨‍👩‍👧‍👦 has multiple Codepoints!
        [[nodiscard]] Bool
        IsEmpty() const { return String.empty(); }

    private:
        FString String;

    public:
        //auto ToString() const -> StringView { return String; }
        explicit operator FString()		const	{ return String; }
        explicit operator const char*()	const	{ return String.data(); }

        // Comparison operators
        [[nodiscard]] Bool
        operator==(const FText& I_Other) const { return String == I_Other.String; }

        [[nodiscard]] Bool
        operator!=(const FText& I_Other) const { return String != I_Other.String; }

        [[nodiscard]] Bool
        operator<(const FText& I_Other) const { return String < I_Other.String; }

        [[nodiscard]] Bool
        operator<=(const FText& I_Other) const { return String <= I_Other.String; }

        [[nodiscard]] Bool
        operator>(const FText& I_Other) const { return String > I_Other.String; }

        [[nodiscard]] Bool
        operator>=(const FText& I_Other) const { return String >= I_Other.String; }

        FText&
        operator+=(const FText& I_Other) { String.append(I_Other.String); return *this; }

        FText() = default;
        FText(FStringView I_String) : String{I_String} { VISERA_ASSERT(ValidateUTF8(String)); }
        template <size_t N> constexpr
        FText(const char (&I_Literal)[N]) { String.assign(I_Literal, N - 1); }
        FText(FWideStringView I_Text);
        FText(const FText&)                      = default;
        FText(FText&&)                  noexcept = default;
        FText& operator=(const FText&)           = default;
        FText& operator=(FText&&)       noexcept = default;

        FText(const char C) noexcept { String.assign(1, C); }
        template <Concepts::FloatingPoint FloatPointType>
        FText(FloatPointType I_Integer) noexcept;
        template <Concepts::Integral IntegralType>
        FText(IntegralType I_Integer) noexcept;
        template <Concepts::Boolean BooleanType>
        FText(BooleanType I_Boolean) noexcept;
    };
    
    template <Concepts::FloatingPoint FloatPointType> FText::
    FText(FloatPointType I_FloatPointValue) noexcept
    {
        if (std::isnan(I_FloatPointValue))
        {
            String.assign("nan", 3);
            return;
        }
        if (std::isinf(I_FloatPointValue))
        {
            I_FloatPointValue > 0? String.assign("inf") :
                                   String.assign("-inf");
            return;
        }

        char Buffer[128];
        double_conversion::StringBuilder Builder(Buffer, sizeof(Buffer));

        // Recommended default: shortest round-trippable representation
        // (ASCII only, stable, fast, no locale)
        static const double_conversion::DoubleToStringConverter Converter(
            double_conversion::DoubleToStringConverter::UNIQUE_ZERO,
            "inf",
            "nan",
            'e',
            -6,
            21,
            0,
            0
        );

        const Bool bConverted = Converter.ToShortest(I_FloatPointValue, &Builder);
        VISERA_ASSERT(bConverted);

        String.assign(Buffer, Builder.position());
    }

    template <Concepts::Integral IntegralType> FText::
    FText(IntegralType I_Integer) noexcept
    {
        // Enough for any integral incl. sign.
        char Buffer[64];

        auto First = Buffer;
        auto Last  = Buffer + sizeof(Buffer);

        // Note: bool excluded by concept; char types are OK (will print number).
        auto [Ptr, Ec] = std::to_chars(First, Last, I_Integer);
        VISERA_ASSERT(Ec == std::errc{}); // buffer size guarantees this

        // ASCII digits/sign/dot/exponent are valid UTF-8 by construction.
        String.assign(Buffer, static_cast<size_t>(Ptr - Buffer));
    }

    template <Concepts::Boolean BooleanType> FText::
    FText(BooleanType I_Boolean) noexcept
    {
        I_Boolean? String.assign("true") : String.assign("false");
    }

    FText::
    FText(FWideStringView I_Text)
    {
        const UInt64 WideLength = I_Text.length();
        if (WideLength == 0) 
        {
            String.clear();
            return;
        }
        const UInt64 UTF8Length = simdutf::utf8_length_from_utf16(
            reinterpret_cast<const char16_t*>(I_Text.data()),
            WideLength
        );
        
        if (UTF8Length == 0) 
        {
            String.clear();
            return;
        }

        String.resize(UTF8Length);
        const UInt64 Written = simdutf::convert_utf16_to_utf8(
            reinterpret_cast<const char16_t*>(I_Text.data()),
            WideLength,
            String.data()
        );

        if (Written == 0 || Written != UTF8Length)
        {
            String.clear();
            return;
        }
        // Validate the converted UTF8 string
        VISERA_ASSERT(ValidateUTF8(String));
    }

    // 👨‍👩‍👧‍👦 has multiple Codepoints!
    UInt64 FText::
    GetCodepointCount() const noexcept
    {
        return simdutf::count_utf8(String.data(), String.size());
    }
}
VISERA_MAKE_FORMATTER(Visera::FText, {}, "{}", I_Formatee.GetData());