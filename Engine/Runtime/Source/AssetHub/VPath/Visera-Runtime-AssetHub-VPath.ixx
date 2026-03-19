module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.VPath;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Core.Types.Name;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Log;

export namespace Visera
{
    enum class EAssetScheme : UInt8
    {
        App,        // @app://
        Assets,     // @assets://
        User,       // @user://
        Cache,      // @cache://
    };

    class VISERA_RUNTIME_API VPath
    {
    public:
        /** Compile-time validated construction from string literal via _vpath UDL. */
        consteval VPath(const char* I_Literal, size_t I_Length)
            : Literal{I_Literal}, LiteralLength{I_Length}
        {
            if (I_Length == 0 || I_Literal[0] != '@')
                throw "VPath must start with '@'";
            if (!HasValidScheme(I_Literal, I_Length))
                throw "VPath has unrecognised scheme (expected @app://, @assets://, @user://, or @cache://)";
        }

        /** Runtime construction from string. Asserts '@' prefix and valid scheme. */
        explicit VPath(FStringView I_Path)
            : Name{I_Path}
        {
            VISERA_ASSERT(!I_Path.IsEmpty() && I_Path[0] == '@');
        }

        /** Runtime construction from pre-registered FName. Asserts '@' prefix. */
        explicit VPath(FName I_Name)
            : Name{I_Name}
        {
            VISERA_ASSERT(!I_Name.IsNone() && I_Name.GetNameString()[0] == '@');
        }

        [[nodiscard]] FName GetName() const
        {
            EnsureName();
            return Name;
        }

        [[nodiscard]] FStringView GetNameString() const
        {
            EnsureName();
            return Name.GetNameString();
        }

        [[nodiscard]] EAssetScheme GetScheme() const
        {
            const FStringView String = GetNameString();
            if (String.StartsWith("@app://"))    return EAssetScheme::App;
            if (String.StartsWith("@assets://")) return EAssetScheme::Assets;
            if (String.StartsWith("@user://"))   return EAssetScheme::User;
            if (String.StartsWith("@cache://"))  return EAssetScheme::Cache;
            VISERA_ASSERT(False && "VPath: unrecognised scheme");
            return EAssetScheme::Assets;
        }

        /** Returns the portion after the scheme (e.g. "images/sky.png" from "@assets://images/sky.png"). */
        [[nodiscard]] FStringView GetRelativePath() const
        {
            const FStringView String = GetNameString();
            if (auto Position = String.Find("://"); Position != FStringView::NPos)
                return String.Substr(Position + 3);
            return String;
        }

        /** Extracts file extension including the dot (e.g. ".png"). */
        [[nodiscard]] TOptional<FStringView> GetExtension() const
        {
            const FStringView String = GetNameString();
            for (Int64 Index = static_cast<Int64>(String.GetSize()) - 1; Index >= 0; --Index)
            {
                if (String[Index] == '.')
                    return TOptional<FStringView>{String.Substr(static_cast<UInt64>(Index))};
                if (String[Index] == '/')
                    break;
            }
            return NullOpt;
        }

        Bool operator==(const VPath& I_Other) const
        {
            EnsureName();
            I_Other.EnsureName();
            return Name == I_Other.Name;
        }

    private:
        const char* Literal       = nullptr;
        size_t      LiteralLength = 0;
        mutable FName Name;

        void EnsureName() const
        {
            if (Literal && Name.IsNone())
                Name = FName{FStringView{Literal, LiteralLength}};
        }

        /** constexpr-friendly scheme check for the consteval constructor. */
        static consteval Bool HasValidScheme(const char* I_String, size_t I_Length)
        {
            auto Starts = [](const char* Haystack, size_t HaystackLength, const char* Needle) consteval -> Bool
            {
                for (size_t Index = 0; Needle[Index] != '\0'; ++Index)
                {
                    if (Index >= HaystackLength) return false;
                    char LoweredHaystack = (Haystack[Index] >= 'A' && Haystack[Index] <= 'Z')
                                         ? static_cast<char>(Haystack[Index] + ('a' - 'A'))
                                         : Haystack[Index];
                    if (LoweredHaystack != Needle[Index]) return false;
                }
                return true;
            };
            return Starts(I_String, I_Length, "@app://")
                || Starts(I_String, I_Length, "@assets://")
                || Starts(I_String, I_Length, "@user://")
                || Starts(I_String, I_Length, "@cache://");
        }
    };

    consteval VPath operator""_vpath(const char* I_String, size_t I_Length)
    {
        return VPath{I_String, I_Length};
    }
}
VISERA_MAKE_HASH(Visera::VPath, return static_cast<size_t>(I_Object.GetName().GetIdentifier()); )
VISERA_MAKE_FORMATTER(Visera::VPath, {}, "{}", I_Formatee.GetNameString());
