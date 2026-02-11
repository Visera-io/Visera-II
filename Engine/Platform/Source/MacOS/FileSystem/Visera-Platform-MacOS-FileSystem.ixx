module;
#include <Visera-Platform.hpp>
#include <Visera-Core.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#endif
export module Visera.Platform.MacOS.FileSystem;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Interface.FileSystem;
import Visera.Platform.MacOS.Path;
import Visera.Core.OS.FileSystem;
import Visera.Core.Containers.Array;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Types.String;
import Visera.OS.FileSystem.File;
import Visera.Core.Log;

namespace Visera
{
    enum class EMacOSIOStatus : UInt8
    {
        Success          = 0,
        NotFound         = 1,
        PermissionDenied = 2,
        Other            = 3,
    };

    export class VISERA_PLATFORM_API FMacOSPlatformFileSystem : public IPlatformFileSystem
    {
    public:
        [[nodiscard]] Int32
        ReplaceFile(const IPlatformPath& I_Source, const IPlatformPath& I_Target) const override;
        [[nodiscard]] Int32
        AtomicWriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const override;
        [[nodiscard]] FTempFileResult
        CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const override;
    };

    Int32 FMacOSPlatformFileSystem::
    ReplaceFile(const IPlatformPath& I_Source, const IPlatformPath& I_Target) const
    {
        const FStringView SourceView(static_cast<const FMacOSPath&>(I_Source).GetView());
        const FStringView TargetView(static_cast<const FMacOSPath&>(I_Target).GetView());
        if (SourceView.IsEmpty() || TargetView.IsEmpty()) return 3; // Other

        const FString SourceStr(SourceView);
        const FString TargetStr(TargetView);
        if (::rename(SourceStr.Data(), TargetStr.Data()) != 0)
        {
            const int Err = errno;
            if (Err == EACCES || Err == EPERM) return 2; // PermissionDenied
            if (Err == ENOENT) return 1; // NotFound
            LOG_DEBUG("ReplaceFile: rename failed: {}", std::strerror(Err));
            return 3; // Other
        }
        return 0; // Success
    }

    Int32 FMacOSPlatformFileSystem::
    AtomicWriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const
    {
        if (I_Data == nullptr && I_Size > 0)
            return 3; // Other

        TUniquePtr<IPlatformPath> Parent = I_Path.GetParent();
        if (!Parent) return 3; // Other

        static const FMacOSPath DefaultPrefix(".VTemp-");
        FTempFileResult Temp = CreateTempFileNear(*Parent, DefaultPrefix);
        if (!Temp.File || !Temp.Path) return 3; // Other

        Bool bWriteOk = True;
        if (I_Size > 0 && Temp.File->Write(I_Data, I_Size, 1) != 1)
            bWriteOk = False;
        Temp.File->Flush();
        Temp.File.Reset(); // close before ReplaceFile

        if (!bWriteOk)
        {
            const FString TempStr(static_cast<const FMacOSPath&>(*Temp.Path).GetView());
            std::remove(TempStr.Data());
            return 3; // Other
        }

        const Int32 ReplaceErr = ReplaceFile(*Temp.Path, I_Path);
        if (ReplaceErr != 0) // Success
        {
            const FString TempStr(static_cast<const FMacOSPath&>(*Temp.Path).GetView());
            std::remove(TempStr.Data());
            return ReplaceErr;
        }
        return 0; // Success
    }

    FTempFileResult FMacOSPlatformFileSystem::
    CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const
    {
        FTempFileResult Result;
        if (I_Directory.IsEmpty()) return Result;

        static const FMacOSPath DefaultPrefix(".VTemp-");
        const IPlatformPath& EffectivePrefix = I_Prefix.IsEmpty() ? static_cast<const IPlatformPath&>(DefaultPrefix) : I_Prefix;
        TUniquePtr<IPlatformPath> Base = I_Directory.Concat(EffectivePrefix);
        if (!Base) return Result;
        TUniquePtr<IPlatformPath> TemplatePath = Base->Append("XXXXXX");
        if (!TemplatePath) return Result;

        const FString Template(static_cast<const FMacOSPath&>(*TemplatePath).GetView());
        TArray<char> Tmpl(Template.begin(), Template.end());
        Tmpl.PushBack('\0');

        const int Fd = mkstemp(Tmpl.Data());
        if (Fd < 0)
        {
            if (errno == EACCES || errno == EPERM) return Result;
            LOG_DEBUG("CreateTempFileNear: mkstemp failed: {}", std::strerror(errno));
            return Result;
        }

        FILE* Fp = fdopen(Fd, "w+b");
        if (!Fp)
        {
            close(Fd);
            return Result;
        }

        const FString TempPathStr(Tmpl.Data());
        Result.File = MakeUnique<FFile>(Fp);
        Result.Path = MakeUnique<FMacOSPath>(FStringView(TempPathStr));
        return Result;
    }
}

VISERA_MAKE_FORMATTER(Visera::EMacOSIOStatus,
    const char* Name = "Unknown";
    switch (I_Formatee)
    {
        case Visera::EMacOSIOStatus::Success:          Name = "Success";          break;
        case Visera::EMacOSIOStatus::NotFound:         Name = "NotFound";         break;
        case Visera::EMacOSIOStatus::PermissionDenied: Name = "PermissionDenied"; break;
        case Visera::EMacOSIOStatus::Other:            Name = "Other";            break;
        default: break;
    },
    "{}", Name
);
