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
            EType Type;
            union
            {
                FString Key;
                UInt32  Index;
            };

            static FToken MakeKey(FString&& I_Key)
            {
                FToken Token;
                Token.Type = EType::Key;
                new (std::addressof(Token.Key)) FString(std::move(I_Key));
                return Token;
            }

            static FToken MakeIndex(UInt32 I_Index)
            {
                FToken Token;
                Token.Type = EType::Index;
                Token.Index = I_Index;
                return Token;
            }

            FToken() : Type(EType::Key), Index(0)
            {

            }

            FToken(const FToken& I_Other) : Type(I_Other.Type)
            {
                if (Type == EType::Key)
                {
                    new (std::addressof(Key)) FString(I_Other.Key);
                }
                else
                {
                    Index = I_Other.Index;
                }
            }

            FToken(FToken&& I_Other) noexcept : Type(I_Other.Type)
            {
                if (Type == EType::Key)
                {
                    new (std::addressof(Key)) FString(std::move(I_Other.Key));
                }
                else
                {
                    Index = I_Other.Index;
                }
            }

            FToken& operator=(const FToken& I_Other)
            {
                if (this == std::addressof(I_Other))
                {
                    return *this;
                }
                if (Type == EType::Key)
                {
                    Key.~FString();
                }
                Type = I_Other.Type;
                if (Type == EType::Key)
                {
                    new (std::addressof(Key)) FString(I_Other.Key);
                }
                else
                {
                    Index = I_Other.Index;
                }
                return *this;
            }

            FToken& operator=(FToken&& I_Other) noexcept
            {
                if (this == std::addressof(I_Other))
                {
                    return *this;
                }
                if (Type == EType::Key)
                {
                    Key.~FString();
                }
                Type = I_Other.Type;
                if (Type == EType::Key)
                {
                    new (std::addressof(Key)) FString(std::move(I_Other.Key));
                }
                else
                {
                    Index = I_Other.Index;
                }
                return *this;
            }

            ~FToken()
            {
                if (Type == EType::Key)
                {
                    Key.~FString();
                }
            }
        };

    public:
        explicit FJSONPath(FStringView I_Path)
        {
            Raw = FString(I_Path);
            Tokens.Reserve(4);
            const UInt64 PathLength = Raw.GetSize();
            const char* PathData = Raw.Data();
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
                    Tokens.PushBack(
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
                            Tokens.Clear();
                            ParseFailed_ = true;
                            return;
                        }
                        ++Cursor;
                    }
                    if (Cursor == PathLength)
                    {
                        Tokens.Clear();
                        ParseFailed_ = true;
                        return;
                    }
                    if (IndexStart == Cursor)
                    {
                        Tokens.Clear();
                        ParseFailed_ = true;
                        return;
                    }
                    UInt32 ArrayIndex = 0;
                    for (UInt64 DigitPos = IndexStart; DigitPos < Cursor; ++DigitPos)
                    {
                        ArrayIndex = ArrayIndex * 10 + static_cast<UInt32>(PathData[DigitPos] - '0');
                    }
                    Tokens.PushBack(FToken::MakeIndex(ArrayIndex));
                    ++Cursor;
                }
                if (Cursor < PathLength && PathData[Cursor] == '.')
                {
                    ++Cursor;
                }
            }
        }

        /** Tokens for path resolution; only FJSON uses these to resolve against its Json root. */
        [[nodiscard]] const TArray<FToken>&
        GetTokens() const noexcept { return Tokens; }

        [[nodiscard]] Bool
        IsValid() const noexcept
        {
            return !ParseFailed_;
        }

    private:
        FString        Raw;
        TArray<FToken> Tokens;
        Bool           ParseFailed_{ false };
    };
}
