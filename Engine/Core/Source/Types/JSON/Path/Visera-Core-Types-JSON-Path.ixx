module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.JSON:Path;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Array;
import Visera.Core.Types.String;

export namespace Visera
{
    class VISERA_CORE_API FJSONPath final
    {
    public:
        struct VISERA_CORE_API FToken
        {
            enum class EType : UInt8
            {
                Key,
                Index
            };
            EType   Type{ EType::Key };
            FString Key;
            UInt32  Index{ 0 };

            static FToken MakeKey(FString&& I_Key)
            {
                FToken T;
                T.Type  = EType::Key;
                T.Key   = std::move(I_Key);
                T.Index = 0;
                return T;
            }

            static FToken MakeIndex(UInt32 I_Index)
            {
                FToken T;
                T.Type  = EType::Index;
                //T.Key   = FString();
                T.Index = I_Index;
                return T;
            }
        };

    public:
        /** Constexpr constructor for JQL literals (e.g. "foo.bar[0]"_JQL). Parsing is done lazily on first use. */
        constexpr explicit FJSONPath(const char* I_Str, UInt32 I_Len) noexcept
            : PathTag(EPathStorage::Literal)
        {
            PathStorage.Literal.Ptr = I_Str ? I_Str : "";
            PathStorage.Literal.Len = I_Str ? I_Len : 0;
        }

        explicit FJSONPath(FStringView I_Path)
            : PathTag(EPathStorage::Owned)
        {
            new (std::addressof(PathStorage.Owned)) FString(I_Path);
        }

        FJSONPath(const FJSONPath& I_Other)
            : PathTag(I_Other.PathTag)
            , Tokens(I_Other.Tokens)
            , bParsed(I_Other.bParsed)
            , bParseFailed(I_Other.bParseFailed)
        {
            if (PathTag == EPathStorage::Owned)
            {
                new (std::addressof(PathStorage.Owned)) FString(I_Other.PathStorage.Owned);
            }
            else
            {
                PathStorage.Literal.Ptr = I_Other.PathStorage.Literal.Ptr;
                PathStorage.Literal.Len = I_Other.PathStorage.Literal.Len;
            }
        }

        FJSONPath(FJSONPath&& I_Other) noexcept
            : PathTag(I_Other.PathTag)
            , Tokens(std::move(I_Other.Tokens))
            , bParsed(I_Other.bParsed)
            , bParseFailed(I_Other.bParseFailed)
        {
            if (PathTag == EPathStorage::Owned)
            {
                new (std::addressof(PathStorage.Owned)) FString(std::move(I_Other.PathStorage.Owned));
            }
            else
            {
                PathStorage.Literal.Ptr = I_Other.PathStorage.Literal.Ptr;
                PathStorage.Literal.Len = I_Other.PathStorage.Literal.Len;
            }
        }

        FJSONPath& operator=(const FJSONPath& I_Other)
        {
            if (this == std::addressof(I_Other)) { return *this; }
            DestroyPathStorage();
            PathTag     = I_Other.PathTag;
            Tokens       = I_Other.Tokens;
            bParsed      = I_Other.bParsed;
            bParseFailed = I_Other.bParseFailed;
            if (PathTag == EPathStorage::Owned)
            {
                new (std::addressof(PathStorage.Owned)) FString(I_Other.PathStorage.Owned);
            }
            else
            {
                PathStorage.Literal.Ptr = I_Other.PathStorage.Literal.Ptr;
                PathStorage.Literal.Len = I_Other.PathStorage.Literal.Len;
            }
            return *this;
        }

        FJSONPath& operator=(FJSONPath&& I_Other) noexcept
        {
            if (this == std::addressof(I_Other)) { return *this; }
            DestroyPathStorage();
            PathTag     = I_Other.PathTag;
            Tokens       = std::move(I_Other.Tokens);
            bParsed      = I_Other.bParsed;
            bParseFailed = I_Other.bParseFailed;
            if (PathTag == EPathStorage::Owned)
            {
                new (std::addressof(PathStorage.Owned)) FString(std::move(I_Other.PathStorage.Owned));
            }
            else
            {
                PathStorage.Literal.Ptr = I_Other.PathStorage.Literal.Ptr;
                PathStorage.Literal.Len = I_Other.PathStorage.Literal.Len;
            }
            return *this;
        }

        ~FJSONPath() { DestroyPathStorage(); }

        /** Tokens for path resolution; only FJSON uses these to resolve against its Json root. */
        [[nodiscard]] const TArray<FToken>&
        GetTokens() const noexcept
        {
            EnsureParsed();
            return Tokens;
        }

        [[nodiscard]] Bool
        IsValid() const noexcept
        {
            EnsureParsed();
            return !bParseFailed;
        }

    private:
        enum class EPathStorage : UInt8 { Literal, Owned };

        struct FLiteralStorage
        {
            const char* Ptr;
            UInt32      Len;
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
            return FStringView(L.Ptr ? L.Ptr : "", L.Ptr ? static_cast<UInt64>(L.Len) : 0);
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
            if (GetPathView().IsEmpty()) { return; }
            ParseInto(GetPathView(), Tokens, bParseFailed);
        }

        static void
        ParseInto(FStringView I_Path, TArray<FToken>& I_OutTokens, Bool& I_OutParseFailed)
        {
            I_OutParseFailed = False;
            I_OutTokens.Clear();
            I_OutTokens.Reserve(4);
            const UInt64 PathLength = I_Path.GetSize();
            const char* PathData = I_Path.Data();
            UInt64 Cursor = 0;

            while (Cursor < PathLength)
            {
                if (PathData[Cursor] == '.')
                {
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
                    I_OutTokens.PushBack(
                        FToken::MakeKey(FString(FStringView(PathData + KeyStart, static_cast<UInt64>(Cursor - KeyStart)))));
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
                    I_OutTokens.PushBack(FToken::MakeIndex(ArrayIndex));
                    ++Cursor;
                }
                if (Cursor < PathLength && PathData[Cursor] == '.')
                {
                    ++Cursor;
                }
            }
        }

        EPathStorage            PathTag;
        FPathStorage            PathStorage;
        mutable TArray<FToken>  Tokens;
        mutable Bool            bParsed      { False };
        mutable Bool            bParseFailed { False };
    };
}
