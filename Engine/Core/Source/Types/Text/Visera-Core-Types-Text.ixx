module;
#include <Visera-Core.hpp>
#include <simdutf.h>
#include <double-conversion/double-conversion.h>
export module Visera.Core.Types.Text;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Array;
import Visera.Core.Types.String;
import Visera.Core.Algorithm.Ranges;

export namespace Visera
{
    /* UTF8 Encoded String */
    class VISERA_CORE_API FText
    {
    public:
        [[nodiscard]] static constexpr Bool
        ValidateUTF8(FStringView I_String) { return simdutf::validate_utf8(I_String.GetNative()); }

        /** Decode to UTF-32 code points (FText is valid UTF-8 at construction). */
        [[nodiscard]] TArray<UInt32>
        ToUTF32() const;
        /** Decode to UTF-16 code units (native endianness). */
        [[nodiscard]] TArray<UInt16>
        ToUTF16() const;
        /** Returns unique code points present in this text (FText is UTF-8 at construction). */
        [[nodiscard]] TArray<UInt32>
        GetUniqueCodepoints() const;

        [[nodiscard]] const FString&
        GetString()   const &  { return String; }
        [[nodiscard]] FString
        GetString()         && { return std::move(String); }
        [[nodiscard]] FString
        GetString()   const && { return String; }
        [[nodiscard]] const char*
        GetData()   const { return String.Data(); }
        [[nodiscard]] UInt64
        GetSize() const { return String.GetSize(); }
        [[nodiscard]] UInt64
        GetCodepointCount() const noexcept; // 👨‍👩‍👧‍👦 has multiple Codepoints!
        [[nodiscard]] Bool
        IsEmpty() const { return String.IsEmpty(); }

    private:
        FString        String;
        mutable UInt32 CachedCodepointCount{~0U}; // Lazy cache for GetCodepointCount(); ~0U means invalid.

    public:
        //auto ToString() const -> StringView { return String; }
        explicit operator FString()		const	{ return String; }
        explicit operator const char*()	const	{ return String.Data(); }

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
        operator+=(const FText& I_Other) { String.Append(I_Other.String); CachedCodepointCount = ~0U; return *this; }

        FText() = default;
        FText(FStringView I_String) : String{I_String} { VISERA_ASSERT(simdutf::validate_utf8(String.Data(), String.GetSize())); }
        template <size_t N> constexpr
        FText(const char (&I_Literal)[N])
        {
            String.Assign(I_Literal, N - 1);
            if (!std::is_constant_evaluated())
            {
                VISERA_ASSERT(ValidateUTF8(I_Literal, N - 1));
            }
        }
        FText(const FText&)                      = default;
        FText(FText&&)                  noexcept = default;
        FText& operator=(const FText&)           = default;
        FText& operator=(FText&&)       noexcept = default;

        FText(const char I_Char) noexcept { String.Assign(1, I_Char); }
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
            String.Assign("nan", 3);
            return;
        }
        if (std::isinf(I_FloatPointValue))
        {
            I_FloatPointValue > 0? String.Assign("inf") :
                                   String.Assign("-inf");
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

        const Bool BConverted = Converter.ToShortest(I_FloatPointValue, &Builder);
        VISERA_ASSERT(BConverted);

        String.Assign(Buffer, Builder.position());
    }

    template <Concepts::Integral IntegralType> FText::
    FText(IntegralType I_Integer) noexcept
    {
        // Enough for any integral incl. sign.
        char Buffer[64];

        auto First = Buffer;
        auto Last  = Buffer + sizeof(Buffer);

        // Note: bool excluded by concept; char types are OK (will print number).
        auto [Ptr, Err] = std::to_chars(First, Last, I_Integer);
        VISERA_ASSERT(Err == std::errc{}); // buffer size guarantees this

        // ASCII digits/sign/dot/exponent are valid UTF-8 by construction.
        String.Assign(Buffer, static_cast<size_t>(Ptr - Buffer));
    }

    template <Concepts::Boolean BooleanType> FText::
    FText(BooleanType I_Boolean) noexcept
    {
        I_Boolean? String.Assign("true") : String.Assign("false");
    }

    // 👨‍👩‍👧‍👦 has multiple Codepoints!
    UInt64 FText::
    GetCodepointCount() const noexcept
    {
        if (CachedCodepointCount != ~0U) { return CachedCodepointCount; }
        const UInt64 N = simdutf::count_utf8(String.Data(), static_cast<size_t>(String.GetSize()));
        CachedCodepointCount = (N <= 0xFFFFFFFFu) ? static_cast<UInt32>(N) : ~0U;
        return N;
    }

    TArray<UInt32> FText::
    ToUTF32() const
    {
        TArray<UInt32> Out;
        const UInt64 Count = GetCodepointCount();
        if (Count == 0) return Out;
        const char* Data = GetData();
        const size_t Size = GetSize();
        Out.Resize(Count);
        const size_t Written = simdutf::convert_valid_utf8_to_utf32(
            Data, Size, reinterpret_cast<char32_t*>(Out.Data()));
        if (Written == 0) return TArray<UInt32>{};
        Out.Resize(static_cast<UInt64>(Written));
        return Out;
    }

    TArray<UInt16> FText::
    ToUTF16() const
    {
        TArray<UInt16> Out;
        const char* Data = GetData();
        const size_t Size = GetSize();
        if (Size == 0) return Out;
        const size_t MaxUnits = simdutf::utf16_length_from_utf8(Data, Size);
        if (MaxUnits == 0) return Out;
        Out.Resize(static_cast<UInt64>(MaxUnits));
        const size_t Written = simdutf::convert_valid_utf8_to_utf16(
            Data, Size, reinterpret_cast<char16_t*>(Out.Data()));
        if (Written == 0) return TArray<UInt16>{};
        Out.Resize(static_cast<UInt64>(Written));
        return Out;
    }

    TArray<UInt32> FText::
    GetUniqueCodepoints() const
    {
        TArray<UInt32> Out = ToUTF32();
        if (Out.IsEmpty()) return Out;
        Algorithm::Sort(Out);
        UInt64 WriteIndex = 0;
        for (UInt64 Index = 0; Index < Out.GetSize(); ++Index)
        {
            if (WriteIndex == 0 || Out[Index] != Out[WriteIndex - 1])
                Out[WriteIndex++] = Out[Index];
        }
        Out.Resize(WriteIndex);
        return Out;
    }
}
VISERA_MAKE_FORMATTER(Visera::FText, {}, "\"{}\"", I_Formatee.GetData());