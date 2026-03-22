module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.VPath;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Core.Types.Name;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Path;
import Visera.Core.Containers.Map;
import Visera.Core.OS.Thread.Sync;
import Visera.Core.Log;
import Visera.Platform;

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
        /** Runtime construction from string. Asserts '@' prefix and valid scheme. */
        explicit VPath(FStringView I_Path)
            : Name{I_Path}
            , Scheme{ParseScheme(I_Path)}
        {
            VISERA_ASSERT(!I_Path.IsEmpty() && I_Path[0] == '@');
        }

        /** Runtime construction from pre-registered FName. Asserts '@' prefix. */
        explicit VPath(FName I_Name)
            : Name{I_Name}
            , Scheme{ParseScheme(I_Name.GetNameString())}
        {
            VISERA_ASSERT(!I_Name.IsNone() && I_Name.GetNameString()[0] == '@');
        }

        [[nodiscard]] FName GetName() const
        {
            return Name;
        }

        [[nodiscard]] FStringView GetNameString() const
        {
            return Name.GetNameString();
        }

        [[nodiscard]] EAssetScheme GetScheme() const
        {
            return Scheme;
        }

        /** Resolve to a concrete filesystem path (cached per virtual path name). */
        [[nodiscard]] FPath GetRealPath() const
        {
            struct FResolvedPathState
            {
                TMap<FName, FPath> Map;
                FRWLock            Lock;
            };
            static FResolvedPathState State;

            const FName Key = GetName();
            {
                FScopeReadLock _{&State.Lock};
                auto It = State.Map.Find(Key);
                if (It != State.Map.end())
                    return It->second;
            }

            const FPath Root = [&]() -> FPath
            {
                switch (Scheme)
                {
                case EAssetScheme::App:    return FPlatform::GetExecutableDirectory();
                case EAssetScheme::Assets: return FPlatform::GetResourceDirectory() / FPath{"Assets"};
                case EAssetScheme::User:   return FPlatform::GetUserDataDirectory();
                case EAssetScheme::Cache:  return FPlatform::GetCacheDirectory();
                }
                return FPath{};
            }();

            const FStringView Relative = GetRelativePath();
            const FPath Resolved = Root / FPath{FString{Relative}};
            {
                FScopeWriteLock _{&State.Lock};
                State.Map[Key] = Resolved;
            }
            return Resolved;
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
            return Name == I_Other.Name;
        }

    private:
        FName        Name;
        EAssetScheme Scheme;

        static EAssetScheme ParseScheme(FStringView String)
        {
            if (String.StartsWith("@app://"))    return EAssetScheme::App;
            if (String.StartsWith("@assets://")) return EAssetScheme::Assets;
            if (String.StartsWith("@user://"))   return EAssetScheme::User;
            if (String.StartsWith("@cache://"))  return EAssetScheme::Cache;
            VISERA_ASSERT(False && "VPath: unrecognised scheme");
            return EAssetScheme::Assets;
        }
    };

    inline VPath operator""_vpath(const char* I_String, size_t I_Length)
    {
        return VPath{FStringView{I_String, I_Length}};
    }
}
VISERA_MAKE_HASH(Visera::VPath, return static_cast<size_t>(I_Object.GetName().GetIdentifier()); )
VISERA_MAKE_FORMATTER(Visera::VPath, {}, "{}", I_Formatee.GetNameString());
