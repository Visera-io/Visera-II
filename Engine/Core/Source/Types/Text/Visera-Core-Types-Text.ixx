module;
#include <Visera-Core.hpp>
#include <double-conversion/double-conversion.h>
export module Visera.Core.Types.Text;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Containers.Array;
import Visera.Core.Types.String;
import Visera.Core.Algorithm.Ranges;

namespace Visera
{
    Bool ValidateUTF8Impl(const char* Data, size_t Size)
    {
        size_t Index = 0;
        while (Index < Size)
        {
            const unsigned char Lead = static_cast<unsigned char>(Data[Index]);
            if (Lead < 0x80) { ++Index; continue; }
            if (Lead < 0xC2) return false;
            if (Lead < 0xE0)
            {
                if (Index + 2 > Size) return false;
                if ((static_cast<unsigned char>(Data[Index + 1]) & 0xC0) != 0x80) return false;
                Index += 2; continue;
            }
            if (Lead < 0xF0)
            {
                if (Index + 3 > Size) return false;
                if ((static_cast<unsigned char>(Data[Index + 1]) & 0xC0) != 0x80 ||
                    (static_cast<unsigned char>(Data[Index + 2]) & 0xC0) != 0x80) return false;
                if (Lead == 0xE0 && static_cast<unsigned char>(Data[Index + 1]) < 0xA0) return false;
                Index += 3; continue;
            }
            if (Lead <= 0xF4)
            {
                if (Index + 4 > Size) return false;
                if ((static_cast<unsigned char>(Data[Index + 1]) & 0xC0) != 0x80 ||
                    (static_cast<unsigned char>(Data[Index + 2]) & 0xC0) != 0x80 ||
                    (static_cast<unsigned char>(Data[Index + 3]) & 0xC0) != 0x80) return false;
                if (Lead == 0xF0 && static_cast<unsigned char>(Data[Index + 1]) < 0x90) return false;
                if (Lead == 0xF4 && static_cast<unsigned char>(Data[Index + 1]) > 0x8F) return false;
                Index += 4; continue;
            }
            return false;
        }
        return true;
    }

    size_t CountUTF8Codepoints(const char* Data, size_t Size)
    {
        size_t Count = 0;
        size_t Index = 0;
        while (Index < Size)
        {
            const unsigned char Lead = static_cast<unsigned char>(Data[Index]);
            if (Lead < 0x80) { ++Index; ++Count; continue; }
            if (Lead < 0xE0) { Index += 2; ++Count; continue; }
            if (Lead < 0xF0) { Index += 3; ++Count; continue; }
            if (Lead <= 0xF4) { Index += 4; ++Count; continue; }
            return 0;
        }
        return Count;
    }

    UInt32 DecodeUTF8Lead(const unsigned char Lead, size_t* OutLength)
    {
        if (Lead < 0x80) { *OutLength = 1; return Lead; }
        if (Lead < 0xE0) { *OutLength = 2; return static_cast<UInt32>(Lead & 0x1F); }
        if (Lead < 0xF0) { *OutLength = 3; return static_cast<UInt32>(Lead & 0x0F); }
        *OutLength = 4; return static_cast<UInt32>(Lead & 0x07);
    }

    void UTF8ToUTF32Impl(const char* Data, size_t Size, UInt32* Out, size_t* OutCount)
    {
        size_t Index = 0;
        size_t WriteIndex = 0;
        while (Index < Size)
        {
            const unsigned char Lead = static_cast<unsigned char>(Data[Index]);
            size_t SeqLength = 0;
            UInt32 CodePoint = DecodeUTF8Lead(Lead, &SeqLength);
            if (Index + SeqLength > Size) break;
            for (size_t I = 1; I < SeqLength; ++I)
                CodePoint = (CodePoint << 6) | (static_cast<unsigned char>(Data[Index + I]) & 0x3F);
            Index += SeqLength;
            Out[WriteIndex++] = CodePoint;
        }
        *OutCount = WriteIndex;
    }

    size_t UTF32ToUTF16Length(const UInt32* CodePoints, size_t Count)
    {
        size_t Length = 0;
        for (size_t I = 0; I < Count; ++I)
            Length += (CodePoints[I] >= 0x10000) ? 2u : 1u;
        return Length;
    }

    void UTF32ToUTF16Impl(const UInt32* CodePoints, size_t Count, UInt16* Out, size_t* OutLength)
    {
        size_t WriteIndex = 0;
        for (size_t I = 0; I < Count; ++I)
        {
            const UInt32 Cp = CodePoints[I];
            if (Cp < 0x10000)
                Out[WriteIndex++] = static_cast<UInt16>(Cp);
            else
            {
                const UInt32 Surrogate = Cp - 0x10000;
                Out[WriteIndex++] = static_cast<UInt16>(0xD800u + (Surrogate >> 10));
                Out[WriteIndex++] = static_cast<UInt16>(0xDC00u + (Surrogate & 0x3FFu));
            }
        }
        *OutLength = WriteIndex;
    }
}

export namespace Visera
{
    /* UTF8 Encoded String */
    class VISERA_CORE_API FText
    {
    public:
        [[nodiscard]] static Bool
        ValidateUTF8(FStringView I_String) { return ValidateUTF8Impl(I_String.Data(), I_String.GetSize()); }

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
        FText(FStringView I_String) : String{I_String} { VISERA_ASSERT(ValidateUTF8Impl(String.Data(), String.GetSize())); }
        template <size_t N> constexpr
        FText(const char (&I_Literal)[N])
        {
            String.Assign(I_Literal, N - 1);
            if (!std::is_constant_evaluated())
            { VISERA_ASSERT(ValidateUTF8(FString(I_Literal, N - 1))); }
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
        const UInt64 N = CountUTF8Codepoints(String.Data(), static_cast<size_t>(String.GetSize()));
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
        size_t Written = 0;
        UTF8ToUTF32Impl(Data, Size, Out.Data(), &Written);
        Out.Resize(static_cast<UInt64>(Written));
        return Out;
    }

    TArray<UInt16> FText::
    ToUTF16() const
    {
        TArray<UInt32> UTF32 = ToUTF32();
        if (UTF32.IsEmpty()) return TArray<UInt16>{};
        const size_t MaxUnits = UTF32ToUTF16Length(UTF32.Data(), UTF32.GetSize());
        TArray<UInt16> Out;
        Out.Resize(static_cast<UInt64>(MaxUnits));
        size_t Written = 0;
        UTF32ToUTF16Impl(UTF32.Data(), UTF32.GetSize(), Out.Data(), &Written);
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