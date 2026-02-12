module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.JSON:Path;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Containers.Array;
import Visera.Core.Types.String;
import Visera.Core.Types.Pointer;
import Visera.Core.OS.Memory;

namespace Visera
{
    struct VISERA_CORE_API FJSONPathToken
    {
        enum class EType : UInt8
        {
            Key,
            Index
        };
        const char* Ptr    { nullptr };
        UInt16      Length { 0 };
        EType       Type   { EType::Key };
        UInt32      Index  { 0 };

        [[nodiscard]] FStringView GetString() const noexcept
        {
            return (Ptr && Length) ? FStringView(Ptr, static_cast<FStringView::SizeType>(Length)) : FStringView{};
        }
    };

    struct VISERA_CORE_API FStaticJSONPathToken
    {
        enum class EType : UInt8 { Key, Index };

        const char* Ptr    { nullptr };
        UInt16      Length { 0 };
        EType       Type   { EType::Key };
        UInt32      Index  { 0 };

        [[nodiscard]] FStringView GetString() const noexcept
        {
            return (Ptr && Length) ? FStringView(Ptr, static_cast<FStringView::SizeType>(Length)) : FStringView{};
        }
    };
    static_assert(sizeof(FJSONPathToken) == 16, "FJSONPathToken expected to be 16 bytes");
    static_assert(sizeof(FStaticJSONPathToken) == 16, "FStaticJSONPathToken expected to be 16 bytes");

    template <UInt64 N>
    struct TFixedString
    {
        char Data[N]{};

        consteval TFixedString(const char (&s)[N])
        {
            for (UInt64 i = 0; i < N; ++i) { Data[i] = s[i]; }
        }

        static consteval UInt64 Size() { return N; } // includes '\0'
    };

    template <UInt64 N>
    TFixedString(const char (&)[N]) -> TFixedString<N>;

    template <UInt32 MaxTokens>
    struct TParsedStaticPath
    {
        FStaticJSONPathToken Tokens[MaxTokens]{};
        UInt32               Count{ 0 };
        Bool                 Valid{ True };
    };

    template <TFixedString S, UInt32 MaxTokens>
    consteval TParsedStaticPath<MaxTokens> ParseStaticJSONPath()
    {
        TParsedStaticPath<MaxTokens> Out{};

        constexpr UInt64 N                    = S.Size();                // includes '\0'
        constexpr UInt64 Length               = (N > 0) ? (N - 1) : 0;   // drop '\0'
        constexpr UInt64 MaxStaticTokenLength = 65535u;

        UInt64 i = 0;
        while (i < Length)
        {
            if (S.Data[i] == '.')
            {
                if (i + 1 < Length && S.Data[i + 1] == '.')
                {
                    Out.Valid = False;
                    return Out;
                }
                ++i;
                continue;
            }

            // key segment
            const UInt64 keyStart = i;
            while (i < Length && S.Data[i] != '.' && S.Data[i] != '[') { ++i; }

            if (keyStart < i)
            {
                if (Out.Count >= MaxTokens)
                {
                    Out.Valid = False;
                    return Out;
                }
                const UInt64 keyLength = i - keyStart;
                if (keyLength > MaxStaticTokenLength)
                {
                    Out.Valid = False;
                    return Out;
                }
                Out.Tokens[Out.Count++] = FStaticJSONPathToken{
                    .Ptr    = S.Data + keyStart,
                    .Length = static_cast<UInt16>(keyLength),
                    .Type   = FStaticJSONPathToken::EType::Key,
                    .Index  = 0,
                };
            }

            // [index]
            if (i < Length && S.Data[i] == '[')
            {
                ++i;
                const UInt64 idxStart = i;

                while (i < Length && S.Data[i] != ']')
                {
                    const char c = S.Data[i];
                    if (c < '0' || c > '9')
                    {
                        Out.Valid = False;
                        return Out;
                    }
                    ++i;
                }

                if (i >= Length)                                        // missing ']'
                {
                    Out.Valid = False;
                    return Out;
                }
                if (idxStart == i)                                   // empty index
                {
                    Out.Valid = False;
                    return Out;
                }

                UInt32 v = 0;
                for (UInt64 p = idxStart; p < i; ++p)
                {
                    const UInt32 d = static_cast<UInt32>(S.Data[p] - '0');
                    v = v * 10u + d;
                }

                if (Out.Count >= MaxTokens)
                {
                    Out.Valid = False;
                    return Out;
                }
                const UInt64 indexLength = i - idxStart;
                if (indexLength > MaxStaticTokenLength)
                {
                    Out.Valid = False;
                    return Out;
                }
                Out.Tokens[Out.Count++] = FStaticJSONPathToken{
                    .Ptr    = S.Data + idxStart,
                    .Length = static_cast<UInt16>(indexLength),
                    .Type   = FStaticJSONPathToken::EType::Index,
                    .Index  = v,
                };

                ++i; // skip ']'
            }

            if (i < Length && S.Data[i] == '.')
            {
                if (i + 1 < Length && S.Data[i + 1] == '.')
                {
                    Out.Valid = False;
                    return Out;
                }
                ++i;
            }
        }

        return Out;
    }

}

export namespace Visera
{
    class FJSONPath;
    struct FStaticJSONPath;

    namespace Concepts
    {
        template<typename T> concept
        JSONPath = std::same_as<std::remove_cvref_t<T>, FJSONPath> ||
                   std::same_as<std::remove_cvref_t<T>, FStaticJSONPath>;
    }

    class VISERA_CORE_API FJSONPath final
    {
    public:
        using FToken = FJSONPathToken;

    public:
        /** Constexpr constructor: only accepts string literals (array ref). Parsing is done lazily on first use. */
        template<UInt64 N>
        constexpr explicit FJSONPath(const char (&I_Literal)[N]) noexcept
            : PathTag(EPathStorage::Literal)
        {
            PathStorage.Literal.Ptr = I_Literal;
            PathStorage.Literal.Length = N > 0 ? static_cast<UInt32>(N - 1) : 0;
        }

        explicit FJSONPath(FStringView I_Path)
            : PathTag(EPathStorage::Owned)
        {
            new (std::addressof(PathStorage.Owned)) FString(I_Path);
        }

        FJSONPath(const FJSONPath& I_Other)
            : PathTag(I_Other.PathTag)
            , bParsed(False)
            , bParseFailed(False)
        {
            if (PathTag == EPathStorage::Owned)
            {
                new (std::addressof(PathStorage.Owned)) FString(I_Other.PathStorage.Owned);
            }
            else
            {
                PathStorage.Literal.Ptr = I_Other.PathStorage.Literal.Ptr;
                PathStorage.Literal.Length = I_Other.PathStorage.Literal.Length;
            }
        }

        FJSONPath(FJSONPath&& I_Other) noexcept = default;
        FJSONPath& operator=(FJSONPath&& I_Other) noexcept = default;

        FJSONPath& operator=(const FJSONPath& I_Other)
        {
            if (this == std::addressof(I_Other)) { return *this; }
            DestroyPathStorage();
            PathTag      = I_Other.PathTag;
            ParsedState.Reset();
            bParsed      = False;
            bParseFailed = False;
            if (PathTag == EPathStorage::Owned)
            {
                new (std::addressof(PathStorage.Owned)) FString(I_Other.PathStorage.Owned);
            }
            else
            {
                PathStorage.Literal.Ptr    = I_Other.PathStorage.Literal.Ptr;
                PathStorage.Literal.Length = I_Other.PathStorage.Literal.Length;
            }
            return *this;
        }

        ~FJSONPath() { DestroyPathStorage(); }

        /** Tokens for path resolution; only FJSON uses these to resolve against its Json root. */
        [[nodiscard]] const TPMRArray<FToken>&
        GetTokens() const noexcept
        {
            EnsureParsed();
            return ParsedState ? ParsedState->Tokens : GetEmptyTokens();
        }

        [[nodiscard]] Bool
        IsValid() const noexcept
        {
            EnsureParsed();
            return !bParseFailed;
        }

        /** Returns the number of path segments (keys and indices). */
        [[nodiscard]] UInt64
        GetTokenCount() const noexcept
        {
            EnsureParsed();
            return ParsedState ? static_cast<UInt64>(ParsedState->Tokens.GetSize()) : 0;
        }

        /** Returns the string representation of the token at I_Index. Key tokens return the key; Index tokens return the index as string. No allocation. */
        [[nodiscard]] FStringView
        operator[](UInt64 I_Index) const
        {
            const auto& Tk = GetTokens();
            if (I_Index >= static_cast<UInt64>(Tk.GetSize())) { return {}; }
            return Tk[static_cast<TPMRArray<FToken>::SizeType>(I_Index)].GetString();
        }

        /** Returns the original path string view. No allocation. */
        [[nodiscard]] FStringView
        GetPathString() const noexcept
        {
            return GetPathView();
        }

    private:
        struct FParsedState
        {
            static constexpr UInt64 InlineSize = sizeof(FToken) * 4 + 32;
            Memory::TMonotonicArena<InlineSize> Arena;
            TPMRArray<FToken>                  Tokens;

            FParsedState() : Tokens(std::addressof(Arena.Get()))
            {
                Tokens.Reserve(4);
            }
        };

        static const TPMRArray<FToken>&
        GetEmptyTokens() noexcept
        {
            static const TPMRArray<FToken> EmptyTokens{ Memory::GetDefaultResource() };
            return EmptyTokens;
        }

        enum class EPathStorage : UInt8 { Literal, Owned };

        struct FLiteralStorage
        {
            const char* Ptr;
            UInt32      Length;
        };
        union FPathStorage
        {
            FLiteralStorage Literal;
            FString         Owned;
            constexpr FPathStorage() noexcept : Literal{ nullptr, 0 } {}
            ~FPathStorage() {}
        };

        [[nodiscard]] FStringView
        GetPathView() const noexcept
        {
            if (PathTag == EPathStorage::Owned)
            {
                return FStringView(PathStorage.Owned);
            }
            const auto& L = PathStorage.Literal;
            return FStringView(L.Ptr ? L.Ptr : "", L.Ptr ? static_cast<UInt64>(L.Length) : 0);
        }

        void
        DestroyPathStorage() noexcept
        {
            if (PathTag == EPathStorage::Owned)
            {
                PathStorage.Owned.~FString();
                PathTag = EPathStorage::Literal;
                new (std::addressof(PathStorage.Literal)) FLiteralStorage{ nullptr, 0 };
            }
        }

        void
        EnsureParsed() const
        {
            if (bParsed) { return; }
            bParsed = True;
            const FStringView Path = GetPathView();
            if (Path.IsEmpty()) { return; }
            ParsedState = MakeUnique<FParsedState>();
            ParseInto(Path, ParsedState->Tokens, bParseFailed);
            if (bParseFailed) { ParsedState.Reset(); }
        }

        static void
        ParseInto(FStringView I_Path, TPMRArray<FToken>& I_OutTokens, Bool& I_OutParseFailed)
        {
            I_OutParseFailed = False;
            I_OutTokens.Clear();
            I_OutTokens.Reserve(4);
            constexpr UInt64 MaxRuntimeTokenLength = 65535u;
            const UInt64 PathLength = I_Path.GetSize();
            const char* PathData = I_Path.Data();
            UInt64 Cursor = 0;

            while (Cursor < PathLength)
            {
                if (PathData[Cursor] == '.')
                {
                    if (Cursor + 1 < PathLength && PathData[Cursor + 1] == '.')
                    {
                        I_OutTokens.Clear();
                        I_OutParseFailed = True;
                        return;
                    }
                    ++Cursor;
                    continue;
                }
                const UInt64 KeyStart = Cursor;
                while (Cursor < PathLength && PathData[Cursor] != '.' && PathData[Cursor] != '[')
                {
                    ++Cursor;
                }
                if (KeyStart < Cursor)
                {
                    const UInt64 keyLength = Cursor - KeyStart;
                    if (keyLength > MaxRuntimeTokenLength)
                    {
                        I_OutTokens.Clear();
                        I_OutParseFailed = True;
                        return;
                    }
                    I_OutTokens.PushBack(
                        FToken{ PathData + KeyStart, static_cast<UInt16>(keyLength), FToken::EType::Key, 0 });
                }
                if (Cursor < PathLength && PathData[Cursor] == '[')
                {
                    ++Cursor;
                    const UInt64 IndexStart = Cursor;
                    while (Cursor < PathLength && PathData[Cursor] != ']')
                    {
                        if (PathData[Cursor] < '0' || PathData[Cursor] > '9')
                        {
                            I_OutTokens.Clear();
                            I_OutParseFailed = True;
                            return;
                        }
                        ++Cursor;
                    }
                    if (Cursor == PathLength || IndexStart == Cursor)
                    {
                        I_OutTokens.Clear();
                        I_OutParseFailed = True;
                        return;
                    }
                    UInt32 ArrayIndex = 0;
                    for (UInt64 DigitPos = IndexStart; DigitPos < Cursor; ++DigitPos)
                    {
                        ArrayIndex = ArrayIndex * 10 + static_cast<UInt32>(PathData[DigitPos] - '0');
                    }
                    const UInt64 indexLength = Cursor - IndexStart;
                    if (indexLength > MaxRuntimeTokenLength)
                    {
                        I_OutTokens.Clear();
                        I_OutParseFailed = True;
                        return;
                    }
                    I_OutTokens.PushBack(
                        FToken{ PathData + IndexStart, static_cast<UInt16>(indexLength), FToken::EType::Index, ArrayIndex });
                    ++Cursor;
                }
                if (Cursor < PathLength && PathData[Cursor] == '.')
                {
                    if (Cursor + 1 < PathLength && PathData[Cursor + 1] == '.')
                    {
                        I_OutTokens.Clear();
                        I_OutParseFailed = True;
                        return;
                    }
                    ++Cursor;
                }
            }
        }

        FPathStorage                      PathStorage;
        mutable TUniquePtr<FParsedState>  ParsedState;
        mutable Bool                      bParsed      { False };
        mutable Bool                      bParseFailed { False };
        EPathStorage                      PathTag;
    };


    struct VISERA_CORE_API FStaticJSONPath
    {
        const FStaticJSONPathToken* Tokens{ nullptr };
        UInt32                      Count{ 0 };
        Bool                        Valid{ True };
    };

    template <TFixedString S, UInt32 MaxTokens>
    struct TJSONPath
    {
        static_assert(MaxTokens > 0, "MaxTokens must be > 0");

        static constexpr auto Parsed = ParseStaticJSONPath<S, MaxTokens>();
        static_assert(Parsed.Valid, "Invalid JSONPath literal");

        static constexpr [[nodiscard]] FStaticJSONPath Get() noexcept
        {
            return FStaticJSONPath{
                .Tokens = Parsed.Valid ? Parsed.Tokens : nullptr,
                .Count  = Parsed.Valid ? Parsed.Count : 0,
                .Valid  = Parsed.Valid
            };
        }

        static constexpr UInt32 TokenCount() noexcept { return Parsed.Count; }
    };
}
VISERA_MAKE_FORMATTER(Visera::FJSONPath, {}, "{}", I_Formatee.GetPathString());
