module;
#include <Visera-Core.hpp>
#include <simdutf.h>
#include <regex>
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

        template <typename T> [[nodiscard]] static inline FText
        ToUTF8(const T* I_Text) 
        { 
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, const char>)
            {
                FString Str{I_Text};
                return FText{Str};
            }
            else
            {
                return FText{I_Text};
            }
        }

        [[nodiscard]] const FString&
        GetString() const { return String; }
        [[nodiscard]] const char*
        GetData()   const { return String.data(); }
        [[nodiscard]] UInt64
        GetSize() const { return String.size(); }
        [[nodiscard]] UInt64
        GetCodepointCount() const noexcept; // 👨‍👩‍👧‍👦 has multiple Codepoints!
        [[nodiscard]] Bool
        IsEmpty() const { return String.empty(); }
        // String manipulation methods (using const FText& for type safety)
        [[nodiscard]] TArray<FText>
        Split(const FText& I_Delimiter, Bool I_RemoveEmpty = False) const;

        [[nodiscard]] Bool
        Match(const FText& I_Pattern) const;

        [[nodiscard]] TArray<FText>
        FindAll(const FText& I_Pattern) const;
        
        [[nodiscard]] FText
        Trim() const;
        
        [[nodiscard]] FText
        TrimStart() const;
        
        [[nodiscard]] FText
        TrimEnd() const;
        
        [[nodiscard]] FText
        Replace(const FText& I_Old, const FText& I_New) const;
        
        [[nodiscard]] Bool
        Contains(const FText& I_Substring) const;
        
        [[nodiscard]] Bool
        StartsWith(const FText& I_Prefix) const;
        
        [[nodiscard]] Bool
        EndsWith(const FText& I_Suffix) const;
        
        [[nodiscard]] Int64
        Find(const FText& I_Substring, UInt64 I_StartPos = 0) const;
        
        [[nodiscard]] Int64
        FindLast(const FText& I_Substring) const;
        
        [[nodiscard]] FText
        Substring(UInt64 I_Start, UInt64 I_Length = UInt64(-1)) const;
        
        [[nodiscard]] FText
        Append(const FText& I_Text) const;
        
        [[nodiscard]] FText
        Prepend(const FText& I_Text) const;
        
        [[nodiscard]] static FText
        Join(const TArray<FText>& I_Texts, const FText& I_Separator);

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

        [[nodiscard]] FText
        operator+(const FText& I_Other) const { return this->Append(I_Other); }

        FText&
        operator+=(const FText& I_Other) { String.append(I_Other.String); return *this; }

        FText() = default;
        FText(FStringView I_String) { VISERA_ASSERT(ValidateUTF8(I_String)); String = I_String; }
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

        String.assign(Buffer, static_cast<size_t>(Builder.position()));
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

    // String manipulation implementations
    TArray<FText>
    FText::Split(const FText& I_Delimiter, Bool I_RemoveEmpty) const
    {
        TArray<FText> Result;
        if (I_Delimiter.IsEmpty() || String.empty())
        {
            if (!I_RemoveEmpty || !String.empty())
            {
                Result.PushBack(*this);
            }
            return Result;
        }

        FStringView DelimiterView{I_Delimiter.GetData(), I_Delimiter.GetSize()};
        UInt64 Start = 0;
        UInt64 Pos = 0;
        const UInt64 DelimiterLength = I_Delimiter.GetSize();

        while ((Pos = String.find(DelimiterView, Start)) != FString::npos)
        {
            if (Pos > Start || !I_RemoveEmpty)
            {
                FString SubStr = String.substr(Start, Pos - Start);
                Result.PushBack(FText{SubStr});
            }
            Start = Pos + DelimiterLength;
        }

        // Add the last part
        if (Start < String.length() || !I_RemoveEmpty)
        {
            FString SubStr = String.substr(Start);
            Result.PushBack(FText{SubStr});
        }

        return Result;
    }

    FText FText::
    Trim() const
    {
        if (String.empty()) { return FText{}; }

        auto IsSpace = [](unsigned char ch) -> bool
        { return std::isspace(ch) != 0; };

        const char* const Data = String.data();
        const size_t Length = String.size();

        size_t Left = 0;
        while (Left < Length && IsSpace(static_cast<unsigned char>(Data[Left]))) { ++Left; }

        if (Left == Length) { return FText{}; }

        size_t Right = Length;
        while (Right > Left && IsSpace(static_cast<unsigned char>(Data[Right - 1]))) { --Right; }

        // Avoid copy
        if (Left == 0 && Right == Length) { return *this;}

        return FText{FStringView{Data + Left, Right - Left}};
    }

    FText FText::
    TrimStart() const
    {
        // Fast path: empty string
        if (String.empty())
        {
            return FText{};
        }

        // NOTE:
        // We only trim ASCII whitespace (std::isspace). This is UTF-8 safe because
        // ASCII whitespace is always single-byte and never splits a multi-byte codepoint.
        auto IsSpace = [](unsigned char I_Char) -> Bool
        {
            return std::isspace(static_cast<int>(I_Char)) != 0;
        };

        const char* const Data = String.data();
        const UInt64      Size = static_cast<UInt64>(String.size());

        // Scan from left to find the first non-whitespace byte.
        UInt64 Left = 0;
        while (Left < Size && IsSpace(static_cast<unsigned char>(Data[Left])))
        {
            ++Left;
        }

        // All bytes are whitespace -> return empty text.
        if (Left == Size)
        {
            return FText{};
        }

        // No trimming needed -> return self (no allocation).
        if (Left == 0)
        {
            return *this;
        }

        return FText{FStringView{Data + Left, static_cast<size_t>(Size - Left)}};
    }

    FText FText::
    TrimEnd() const
    {
        // Fast path: empty string
        if (String.empty())
        {
            return FText{};
        }

        auto IsSpace = [](unsigned char I_Char) -> Bool
        {
            return std::isspace(static_cast<int>(I_Char)) != 0;
        };

        const char* const Data = String.data();
        const UInt64      Size = static_cast<UInt64>(String.size());

        // Scan from right to find the last non-whitespace byte.
        // 'Right' is the one-past-the-end index of the trimmed result.
        UInt64 Right = Size;
        while (Right > 0 && IsSpace(static_cast<unsigned char>(Data[Right - 1])))
        {
            --Right;
        }

        // All bytes are whitespace -> return empty text.
        if (Right == 0)
        {
            return FText{};
        }

        // No trimming needed -> return self (no allocation).
        if (Right == Size)
        {
            return *this;
        }

        return FText{FStringView{Data, static_cast<size_t>(Right)}};
    }

    FText FText::
    Replace(const FText& I_Old, const FText& I_New) const
    {
        if (I_Old.IsEmpty() || String.empty()) { return *this; }

        FString Result = String;
        FStringView OldView{reinterpret_cast<const char*>(I_Old.GetData()), I_Old.GetSize()};
        FStringView NewView{reinterpret_cast<const char*>(I_New.GetData()), I_New.GetSize()};
        UInt64 Pos = 0;
        const UInt64 OldLength = I_Old.GetSize();

        while ((Pos = Result.find(OldView, Pos)) != FString::npos)
        {
            Result.replace(Pos, OldLength, NewView);
            Pos += I_New.GetSize();
        }
        return FText{Result};
    }

    Bool
    FText::Contains(const FText& I_Substring) const
    {
        FStringView SubstringView{reinterpret_cast<const char*>(I_Substring.GetData()), I_Substring.GetSize()};
        return String.find(SubstringView) != FString::npos;
    }

    Bool
    FText::StartsWith(const FText& I_Prefix) const
    {
        if (I_Prefix.GetSize() > String.length()) { return False; }
        FStringView PrefixView{reinterpret_cast<const char*>(I_Prefix.GetData()), I_Prefix.GetSize()};
        return String.compare(0, I_Prefix.GetSize(), PrefixView) == 0;
    }

    Bool
    FText::EndsWith(const FText& I_Suffix) const
    {
        if (I_Suffix.GetSize() > String.length()) { return False; }
        FStringView SuffixView{I_Suffix.GetData(), I_Suffix.GetSize()};
        return String.compare(String.length() - I_Suffix.GetSize(), I_Suffix.GetSize(), SuffixView) == 0;
    }

    Int64
    FText::Find(const FText& I_Substring, UInt64 I_StartPos) const
    {
        if (I_StartPos >= String.length()) { return -1; }
        FStringView SubstringView{I_Substring.GetData(), I_Substring.GetSize()};
        UInt64 Pos = String.find(SubstringView, I_StartPos);
        return Pos == FString::npos ? -1 : static_cast<Int64>(Pos);
    }

    Int64
    FText::FindLast(const FText& I_Substring) const
    {
        FStringView SubstringView{I_Substring.GetData(), I_Substring.GetSize()};
        UInt64 Pos = String.rfind(SubstringView);
        return Pos == FString::npos ? -1 : static_cast<Int64>(Pos);
    }

    FText FText::
    Substring(UInt64 I_Start, UInt64 I_Length) const
    {
        if (I_Start >= String.length()) { return FText{}; }
        UInt64 Start = I_Start;
        UInt64 Length = (I_Length == static_cast<UInt64>(-1)) ?
                        String.length() - Start : static_cast<UInt64>(I_Length);
        Length = std::min(Length, String.length() - Start);
        FString Result = String.substr(Start, Length);
        return FText{Result};
    }

    FText FText::
    Append(const FText& I_Text) const
    {
        FString Result = String;
        Result.append(I_Text.GetString());
        return FText{Result};
    }

    FText FText::
    Prepend(const FText& I_Text) const
    {
        FString Result{I_Text.GetData(), I_Text.GetSize()};
        Result.append(String);
        return FText{Result};
    }

    FText FText::
    Join(const TArray<FText>& I_Texts, const FText& I_Separator)
    {
        if (I_Texts.IsEmpty()) { return FText{}; }
        
        FString Result;
        Bool First = True;
        for (const auto& Text : I_Texts)
        {
            if (!First)
            { Result.append(I_Separator.GetData(), I_Separator.GetSize()); }

            Result.append(Text.GetString());
            First = False;
        }

        return FText{Result};
    }

    Bool
    FText::Match(const FText& I_Pattern) const
    {
        try
        {
            std::regex Regex{I_Pattern.String.begin(), I_Pattern.String.end()};
            return std::regex_match(String, Regex);
        }
        catch (const std::regex_error&)
        {
            return False;
        }
    }

    TArray<FText> FText::
    FindAll(const FText& I_Pattern) const
    {
        TArray<FText> Result;
        try
        {
            std::regex Regex{I_Pattern.String.begin(), I_Pattern.String.end()};
            std::sregex_iterator It{String.begin(), String.end(), Regex};
            std::sregex_iterator End{};

            for (; It != End; ++It)
            {
                const std::smatch& Match = *It;
                FString MatchStr = Match.str();
                Result.PushBack(FText{MatchStr});
            }
        }
        catch (const std::regex_error&)
        {
            // Return empty array on regex error
        }
        return Result;
    }

    // 👨‍👩‍👧‍👦 has multiple Codepoints!
    UInt64 FText::
    GetCodepointCount() const noexcept
    {
        if (String.empty()) { return 0; }

        const char* Data = String.data();
        const auto  Size = String.size();

        // Optional: in debug you can assert UTF-8 validity; release assumes invariant.
        // VISERA_ASSERT(ValidateUTF8(FStringView{data, len}));

        // Worst case: 1 code point per byte (ASCII) so len is safe upper bound.
        // Convert to UTF-32 and count written code points.
        TArray<char32_t> Buffer(Size);

        const auto Written = simdutf::convert_utf8_to_utf32(Data, Size, Buffer.Data());
        // convert_* returns 0 on error in simdutf
        VISERA_ASSERT(Written != 0 || Size == 0);

        return Written;
    }
}
VISERA_MAKE_FORMATTER(Visera::FText, {}, "{}", I_Formatee.GetString().c_str())