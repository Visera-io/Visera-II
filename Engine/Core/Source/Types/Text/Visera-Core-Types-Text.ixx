module;
#include <Visera-Core.hpp>
#include <simdutf.h>
export module Visera.Core.Types.Text;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Array;

export namespace Visera
{
    /* UTF8 Encoded String */
    class VISERA_CORE_API FText
    {
    public:
        [[nodiscard]] static constexpr Bool
        ValidateUTF8(FStringView I_String) { return simdutf::validate_utf8(I_String); }

        template <typename T> [[nodiscard]] static inline FText
        ToUTF8(const T* I_Text) 
        { 
            if constexpr (std::is_same_v<T, char> || std::is_same_v<T, const char>)
            {
                FString Str{I_Text};
                VISERA_ASSERT(ValidateUTF8(Str));
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
        GetLength() const { return static_cast<UInt64>(String.length()); }
        [[nodiscard]] Bool
        IsEmpty()   const { return String.empty(); }
        // String manipulation methods (using const FText& for type safety)
        [[nodiscard]] TArray<FText>
        Split(const FText& I_Delimiter, Bool I_RemoveEmpty = False) const;
        
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

        // Default constructor
        FText() = default;

        // Allow explicit construction from FStringView (for bytes that need validation)
        explicit FText(FStringView I_String)
        {
            String = FString{I_String};
            VISERA_ASSERT(ValidateUTF8(String));
        }

        FText(FWideStringView I_Text);
        FText(const FText&)                      = default;
        FText(FText&&)                  noexcept = default;
        FText& operator=(const FText&)           = default;
        FText& operator=(FText&&)       noexcept = default;
    };

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

        FStringView DelimiterView{reinterpret_cast<const char*>(I_Delimiter.GetData()), I_Delimiter.GetLength()};
        UInt64 Start = 0;
        UInt64 Pos = 0;
        const UInt64 DelimiterLength = I_Delimiter.GetLength();

        while ((Pos = String.find(DelimiterView, Start)) != FString::npos)
        {
            if (Pos > Start || !I_RemoveEmpty)
            {
                FString SubStr = String.substr(Start, Pos - Start);
                VISERA_ASSERT(ValidateUTF8(SubStr));
                Result.PushBack(FText{SubStr});
            }
            Start = Pos + DelimiterLength;
        }

        // Add the last part
        if (Start < String.length() || !I_RemoveEmpty)
        {
            FString SubStr = String.substr(Start);
            VISERA_ASSERT(ValidateUTF8(SubStr));
            Result.PushBack(FText{SubStr});
        }

        return Result;
    }

    FText FText::
    Trim() const
    {
        return TrimStart().TrimEnd();
    }

    FText FText::
    TrimStart() const
    {
        auto It = std::ranges::find_if_not(String, 
            [](unsigned char C) { return std::isspace(C); });
        if (It == String.end()) { return *this; }
        FString Result{It, String.end()};
        VISERA_ASSERT(ValidateUTF8(Result));
        return FText{Result};
    }

    FText FText::
    TrimEnd() const
    {
        auto ReverseView = String | std::views::reverse;
        auto It = std::ranges::find_if_not(ReverseView,
            [](unsigned char C) { return std::isspace(C); });
        if (It == ReverseView.end()) { return *this; }
        // Calculate the end position from the reverse iterator
        UInt64 End = String.length() - std::ranges::distance(ReverseView.begin(), It);
        FString Result{String.begin(), String.begin() + End};
        VISERA_ASSERT(ValidateUTF8(Result));
        return FText{Result};
    }

    FText FText::
    Replace(const FText& I_Old, const FText& I_New) const
    {
        if (I_Old.IsEmpty() || String.empty()) { return *this; }

        FString Result = String;
        FStringView OldView{reinterpret_cast<const char*>(I_Old.GetData()), I_Old.GetLength()};
        FStringView NewView{reinterpret_cast<const char*>(I_New.GetData()), I_New.GetLength()};
        UInt64 Pos = 0;
        const UInt64 OldLength = I_Old.GetLength();

        while ((Pos = Result.find(OldView, Pos)) != FString::npos)
        {
            Result.replace(Pos, OldLength, NewView);
            Pos += I_New.GetLength();
        }

        VISERA_ASSERT(ValidateUTF8(Result));
        return FText{Result};
    }

    Bool
    FText::Contains(const FText& I_Substring) const
    {
        FStringView SubstringView{reinterpret_cast<const char*>(I_Substring.GetData()), I_Substring.GetLength()};
        return String.find(SubstringView) != FString::npos;
    }

    Bool
    FText::StartsWith(const FText& I_Prefix) const
    {
        if (I_Prefix.GetLength() > String.length()) { return False; }
        FStringView PrefixView{reinterpret_cast<const char*>(I_Prefix.GetData()), I_Prefix.GetLength()};
        return String.compare(0, I_Prefix.GetLength(), PrefixView) == 0;
    }

    Bool
    FText::EndsWith(const FText& I_Suffix) const
    {
        if (I_Suffix.GetLength() > String.length()) { return False; }
        FStringView SuffixView{I_Suffix.GetData(), I_Suffix.GetLength()};
        return String.compare(String.length() - I_Suffix.GetLength(), I_Suffix.GetLength(), SuffixView) == 0;
    }

    Int64
    FText::Find(const FText& I_Substring, UInt64 I_StartPos) const
    {
        if (I_StartPos >= String.length()) { return -1; }
        FStringView SubstringView{I_Substring.GetData(), I_Substring.GetLength()};
        UInt64 Pos = String.find(SubstringView, I_StartPos);
        return Pos == FString::npos ? -1 : static_cast<Int64>(Pos);
    }

    Int64
    FText::FindLast(const FText& I_Substring) const
    {
        FStringView SubstringView{I_Substring.GetData(), I_Substring.GetLength()};
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
        VISERA_ASSERT(ValidateUTF8(Result));
        return FText{Result};
    }

    FText FText::
    Append(const FText& I_Text) const
    {
        FString Result = String;
        Result.append(I_Text.GetData()), I_Text.GetLength();
        VISERA_ASSERT(ValidateUTF8(Result));
        return FText{Result};
    }

    FText FText::
    Prepend(const FText& I_Text) const
    {
        FString Result{I_Text.GetData(), I_Text.GetLength()};
        Result.append(String);
        VISERA_ASSERT(ValidateUTF8(Result));
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
            { Result.append(I_Separator.GetData(), I_Separator.GetLength()); }

            Result.append(Text.GetString());
            First = False;
        }
        
        VISERA_ASSERT(ValidateUTF8(Result));
        return FText{Result};
    }
}
VISERA_MAKE_FORMATTER(Visera::FText, {}, "{}", I_Formatee.GetString().c_str())