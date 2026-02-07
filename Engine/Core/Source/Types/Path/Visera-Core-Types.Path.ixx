module;
#include <Visera-Core.hpp>
#include <string_view>
export module Visera.Core.Types.Path;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Text;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Char;
import Visera.Core.Math.Hash.CityHash;

export namespace Visera
{
    class FPath;
    template<> inline constexpr Bool HasIntrusiveUnsetOptionalState<FPath> = True;

    class VISERA_CORE_API FPath
    {
    public:
        [[nodiscard]] inline Bool
        IsEmpty() const { return PathString.IsEmpty(); }
        [[nodiscard]] inline TOptional<FStringView>
        GetFileName() const
        {
            const auto LastSlash = PathString.FindLast('/');
            const auto LastBack  = PathString.FindLast('\\');
            const auto Pos = (LastSlash == FString::NPos) ? LastBack :
                ((LastBack == FString::NPos) ? LastSlash : (LastSlash > LastBack ? LastSlash : LastBack));
            if (Pos != FString::NPos)
            { return TOptional<FStringView>(FStringView(PathString.Data() + (Pos + 1), PathString.GetSize() - (Pos + 1))); }
            return TOptional<FStringView>(FStringView(PathString));
        }
        [[nodiscard]] inline TOptional<FPath>
        GetParent() const
        {
            const auto LastSlash = PathString.FindLast('/');
            const auto LastBack  = PathString.FindLast('\\');
            const auto Pos = (LastSlash == FString::NPos) ? LastBack :
                ((LastBack == FString::NPos) ? LastSlash : (LastSlash > LastBack ? LastSlash : LastBack));
            if (Pos != FString::NPos)
            { return TOptional<FPath>(FPath(PathString.Substr(0, Pos), True)); }
            return TOptional<FPath>{};
        }
        [[nodiscard]] inline TOptional<FStringView>
        GetExtension() const
        {
            const auto Size = PathString.GetSize();
            if (Size == 0) { return NullOpt; }

            FString::SizeType DotPos           = FString::NPos;
            FString::SizeType FileNameBeginPos = 0;

            for (FString::SizeType I = Size; I-- > 0; )
            {
                const char Ch = PathString[I];
                if (Ch == '/' || Ch == '\\') { FileNameBeginPos = I + 1; break; }
                if (Ch == '.') { DotPos = I; break; }
            }

            if (DotPos == FString::NPos)    { return NullOpt; }
            if (DotPos == FileNameBeginPos) { return NullOpt; } // ".gitignore"

            return TOptional<FStringView>(FStringView(PathString.Data() + DotPos, Size - DotPos));
        }
        [[nodiscard]] inline const FString&
        GetString() const { return PathString; }

        [[nodiscard]] static FPath Merge(const FPath& I_Lhs, const FPath& I_Rhs)
        {
            if (I_Lhs.IsEmpty()) { return I_Rhs; }
            if (I_Rhs.IsEmpty()) { return I_Lhs; }

            const auto& A = I_Lhs.GetString();
            const auto& B = I_Rhs.GetString();

            const Bool bNeedSlash = (A.Back() != '/');
            FString Result;
            Result.Reserve(A.GetSize() + (bNeedSlash ? 1 : 0) + B.GetSize());
            Result.Append(A);
            if (bNeedSlash) { Result.Append('/'); }
            Result.Append(B);
            return FPath(std::move(Result), /*from_computed*/ True);
        }
        [[nodiscard]] FPath
        operator/(const FPath& I_Path) const { return Merge(*this, I_Path); }
        [[nodiscard]] Bool
        operator==(const FPath& I_Path) const { return PathString == I_Path.PathString; }
        [[nodiscard]] Bool
        operator==(FStringView I_PathString) const { return PathString == I_PathString; }

        /** Normalizes this path in place: single '/', no ".", "..", or trailing '/'. */
        void Normalize();
        /** Returns a new normalized path without modifying the argument. */
        [[nodiscard]] static FPath Normalized(const FPath& I_Path);
        /** Returns True if this path is in normalized form (no '\\', no "//", no "/./", no "/../", no trailing '/'). */
        [[nodiscard]] Bool IsNormalized() const;

    private:
        FPath(FString I_Path, Bool /* from_computed */) : PathString{std::move(I_Path)} {}
        FString PathString;

    public:
        /** Construct from a dynamically built path string (e.g. from filesystem enumeration). */
        explicit FPath(FString I_Path) : PathString{std::move(I_Path)} {}
        /** Construct from UTF-8 path (e.g. std::filesystem::path::u8string()). */
        explicit FPath(const std::u8string& I_Utf8Path)
            : PathString(reinterpret_cast<const char*>(I_Utf8Path.data()), I_Utf8Path.size()) {}
        /** Construct from UTF-8 path view, avoids copying when a view is already available. */
        explicit FPath(std::u8string_view I_Utf8Path)
            : PathString(reinterpret_cast<const char*>(I_Utf8Path.data()), I_Utf8Path.size()) {}

        template <size_t N> explicit constexpr
        FPath(const char (&I_Literal)[N]) : PathString{I_Literal, N - 1}
        {
            static_assert(N >= 1);
            if (!std::is_constant_evaluated())
            {
                VISERA_ASSERT(FText::ValidateUTF8(I_Literal));
                VISERA_ASSERT(IsNormalized(I_Literal));
            }
        }

        FPath(FIntrusiveUnsetOptionalState) noexcept {}
        VISERA_CORE_API
        friend Bool operator==(const FPath& I_Lhs, FIntrusiveUnsetOptionalState) noexcept;

    private:
        static FString NormalizeString(FStringView I_Path);
        static Bool IsNormalizedPath(FStringView I_Path, Bool I_AllowTrailingSlash = False);

        template <size_t N> static constexpr Bool IsNormalized(const char (&I_Path)[N], Bool I_AllowTrailingSlash = False)
        {
            constexpr size_t I_Size = N - 1;
            if (I_Size == 0) return True;
            for (size_t I = 0; I < I_Size; ++I)
                if (I_Path[I] == '\\') return False;
            if (!I_AllowTrailingSlash && I_Size > 1 && I_Path[I_Size - 1] == '/') return False;
            for (size_t I = 1; I < I_Size; ++I)
                if (I_Path[I - 1] == '/' && I_Path[I] == '/') return False;
            for (size_t I = 0; I + 2 < I_Size; ++I)
                if (I_Path[I] == '/' && I_Path[I + 1] == '.' && I_Path[I + 2] == '/') return False;
            if (I_Size >= 2 && I_Path[I_Size - 2] == '/' && I_Path[I_Size - 1] == '.') return False;
            for (size_t I = 0; I + 3 < I_Size; ++I)
                if (I_Path[I] == '/' && I_Path[I + 1] == '.' && I_Path[I + 2] == '.' && I_Path[I + 3] == '/') return False;
            if (I_Size >= 3 && I_Path[I_Size - 3] == '/' && I_Path[I_Size - 2] == '.' && I_Path[I_Size - 1] == '.') return False;
            return True;
        }
    };

    inline void FPath::Normalize()
    {
        PathString = NormalizeString(PathString);
    }

    inline FPath FPath::Normalized(const FPath& I_Path)
    {
        return FPath(NormalizeString(I_Path.GetString()), True);
    }

    inline Bool FPath::IsNormalized() const
    {
        return IsNormalizedPath(PathString);
    }

    inline FString FPath::NormalizeString(FStringView I_Path)
    {
        if (I_Path.IsEmpty()) return FString();
        FString Result;
        const auto N = I_Path.GetSize();
        FString::SizeType I = 0;
        while (I < N)
        {
            FString::SizeType Start = I;
            while (I < N && I_Path[I] != '/') ++I;
            FString::SizeType SegLen = I - Start;
            if (I < N) ++I;
            if (SegLen == 0) continue;
            if (SegLen == 1 && I_Path[Start] == '.') continue;
            if (SegLen == 2 && I_Path[Start] == '.' && I_Path[Start + 1] == '.')
            {
                const auto Pos = Result.FindLast('/');
                if (Pos != FString::NPos)
                    Result = Result.Substr(0, Pos);
                else
                    Result.Clear();
                continue;
            }
            if (!Result.IsEmpty()) Result.Append('/');
            Result.Append(I_Path.GetNative().data() + Start, SegLen);
        }
        return Result;
    }

    inline Bool FPath::IsNormalizedPath(FStringView I_Path, Bool I_AllowTrailingSlash)
    {
        const auto Size = I_Path.GetSize();
        if (Size == 0) return True;
        for (FString::SizeType J = 0; J < Size; ++J)
            if (I_Path[J] == '\\') return False;
        if (!I_AllowTrailingSlash && Size > 1 && (I_Path[Size - 1] == '/' || I_Path[Size - 1] == '\\')) return False;
        for (FString::SizeType J = 1; J < Size; ++J)
            if (I_Path[J - 1] == '/' && I_Path[J] == '/') return False;
        for (FString::SizeType J = 0; J + 2 < Size; ++J)
            if (I_Path[J] == '/' && I_Path[J + 1] == '.' && I_Path[J + 2] == '/') return False;
        if (Size >= 2 && I_Path[Size - 2] == '/' && I_Path[Size - 1] == '.') return False;
        for (FString::SizeType J = 0; J + 3 < Size; ++J)
            if (I_Path[J] == '/' && I_Path[J + 1] == '.' && I_Path[J + 2] == '.' && I_Path[J + 3] == '/') return False;
        if (Size >= 3 && I_Path[Size - 3] == '/' && I_Path[Size - 2] == '.' && I_Path[Size - 1] == '.') return False;
        return True;
    }
    inline Bool operator==(const FPath& I_Lhs, FIntrusiveUnsetOptionalState) noexcept
    { return I_Lhs.IsEmpty(); }
    static_assert(sizeof(TOptional<FPath>) == sizeof(FPath));
}
VISERA_MAKE_HASH(Visera::FPath, return Visera::Math::CityHash64(I_Object.GetString()););
VISERA_MAKE_FORMATTER(Visera::FPath, {}, "\"{}\"", I_Formatee.GetString());