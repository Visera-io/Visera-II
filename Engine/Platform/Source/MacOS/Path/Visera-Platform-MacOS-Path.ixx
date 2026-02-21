module;
#include <Visera-Platform.hpp>
export module Visera.Platform.MacOS.Path;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Interface.Path;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Types.Pointer.Unique;

namespace Visera
{
    export class VISERA_PLATFORM_API FMacOSPath : public IPlatformPath
    {
    public:
        using FPathChar = char;
        explicit FMacOSPath(const FPath& I_Path) : Native(I_Path.GetString().GetNative())
        {
            VISERA_ASSERT(I_Path.IsNormalized());
        }
        explicit FMacOSPath(FStringView I_Native) : Native(I_Native) {}
        [[nodiscard]] FStringView GetView() const noexcept { return Native; }
        [[nodiscard]] const FPathChar* GetPathString() const noexcept { return Native.Data(); }
        [[nodiscard]] Bool IsEmpty() const override { return Native.IsEmpty(); }
        [[nodiscard]] TUniquePtr<IPlatformPath> GetParent() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath> GetFileName() const override;
        [[nodiscard]] FPath ToPath() const override { return FPath(Native); }
        [[nodiscard]] TUniquePtr<IPlatformPath> Clone() const override { return MakeUnique<FMacOSPath>(Native); }
        [[nodiscard]] TUniquePtr<IPlatformPath> Concat(const IPlatformPath& I_Other) const override;
        [[nodiscard]] TUniquePtr<IPlatformPath> Append(FStringView I_Suffix) const override;

    private:
        FString Native;
    };

    inline TUniquePtr<IPlatformPath> FMacOSPath::GetParent() const
    {
        const auto Pos = Native.FindLast('/');
        if (Pos == FString::NPos || Pos == 0) return nullptr;
        return MakeUnique<FMacOSPath>(Native.Substr(0, Pos));
    }

    inline TUniquePtr<IPlatformPath> FMacOSPath::GetFileName() const
    {
        const auto Pos = Native.FindLast('/');
        if (Pos != FString::NPos && static_cast<UInt64>(Pos + 1) < Native.GetSize())
            return MakeUnique<FMacOSPath>(Native.Substr(Pos + 1));
        return Clone();
    }

    inline TUniquePtr<IPlatformPath> FMacOSPath::Concat(const IPlatformPath& I_Other) const
    {
        const FPath Merged = ToPath() / I_Other.ToPath();
        return MakeUnique<FMacOSPath>(Merged.GetString());
    }

    inline TUniquePtr<IPlatformPath> FMacOSPath::Append(FStringView I_Suffix) const
    {
        if (I_Suffix.IsEmpty()) return Clone();
        FString Result(Native);
        Result.Append(I_Suffix);
        return MakeUnique<FMacOSPath>(std::move(Result));
    }
}
