module;
#include <Visera-Platform.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#endif
export module Visera.Platform.MacOS.FileSystem;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Interface.FileSystem;
import Visera.Platform.MacOS.Path;
import Visera.Core.OS.FileSystem;
import Visera.Core.Containers.Array;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Tuple;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Types.String;
import Visera.Core.Types.Path;
import Visera.OS.FileSystem.File;
import Visera.Core.Log;

export namespace Visera
{
    enum class EMacOSIOStatus : UInt8
    {
        Success          = 0,
        NotFound         = 1,
        PermissionDenied = 2,
        Other            = 3,
    };

    class VISERA_PLATFORM_API FMacOSPlatformFileSystem : public IPlatformFileSystem
    {
    public:
        [[nodiscard]] Bool ExistsFile(const IPlatformPath& I_Path) const override;
        [[nodiscard]] Bool ExistsDirectory(const IPlatformPath& I_Path) const override;
        [[nodiscard]] Int32 CreateDirectories(const IPlatformPath& I_Path) const override;
        [[nodiscard]] TOptional<TArray<FByte>> ReadFile(const IPlatformPath& I_Path) const override;
        [[nodiscard]] Int32 WriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const override;
        [[nodiscard]] Int32 DeleteFile(const IPlatformPath& I_Path) const override;
        [[nodiscard]] TArray<FPath> EnumerateFiles(const IPlatformPath& I_Directory, Bool I_bRecursive) const override;
        [[nodiscard]] TUniquePtr<FFile> OpenFile(const IPlatformPath& I_Path, EFileMode I_Mode) const override;
        [[nodiscard]] Int32
        ReplaceFile(const IPlatformPath& I_Source, const IPlatformPath& I_Target) const override;
        [[nodiscard]] Int32
        AtomicWriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const override;
        [[nodiscard]] TPair<TUniquePtr<FFile>, TUniquePtr<IPlatformPath>>
        CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const override;
    };

#if defined(VISERA_ON_APPLE_SYSTEM)
    Bool FMacOSPlatformFileSystem::ExistsFile(const IPlatformPath& I_Path) const
    {
        const FStringView View(static_cast<const FMacOSPath&>(I_Path).GetView());
        if (View.IsEmpty()) return False;
        struct stat St;
        if (stat(FString(View).Data(), &St) != 0) return False;
        return S_ISREG(St.st_mode);
    }

    Bool FMacOSPlatformFileSystem::ExistsDirectory(const IPlatformPath& I_Path) const
    {
        const FStringView View(static_cast<const FMacOSPath&>(I_Path).GetView());
        if (View.IsEmpty()) return False;
        struct stat St;
        if (stat(FString(View).Data(), &St) != 0) return False;
        return S_ISDIR(St.st_mode);
    }

    Int32 FMacOSPlatformFileSystem::CreateDirectories(const IPlatformPath& I_Path) const
    {
        TUniquePtr<IPlatformPath> Parent = I_Path.GetParent();
        if (Parent && !Parent->IsEmpty())
        {
            if (!ExistsDirectory(*Parent))
            {
                const Int32 Err = CreateDirectories(*Parent);
                if (Err != 0) return Err;
            }
        }
        const FStringView View(static_cast<const FMacOSPath&>(I_Path).GetView());
        if (View.IsEmpty()) return 0;
        const FString PathStr(View);
        if (mkdir(PathStr.Data(), 0755) == 0) return 0;
        if (errno == EEXIST) return ExistsDirectory(I_Path) ? 0 : 3;
        if (errno == EACCES || errno == EPERM) return 2;
        return 3;
    }

    TOptional<TArray<FByte>> FMacOSPlatformFileSystem::ReadFile(const IPlatformPath& I_Path) const
    {
        auto F = OpenFile(I_Path, EFileMode::Read | EFileMode::Binary);
        if (!F || !F->IsOpen()) return NullOpt;
        return F->ReadAll();
    }

    Int32 FMacOSPlatformFileSystem::WriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const
    {
        auto F = OpenFile(I_Path, EFileMode::Write | EFileMode::Binary);
        if (!F || !F->IsOpen()) return 3;
        if (I_Size > 0 && I_Data != nullptr && F->Write(I_Data, I_Size, 1) != 1) return 3;
        return 0;
    }

    Int32 FMacOSPlatformFileSystem::DeleteFile(const IPlatformPath& I_Path) const
    {
        const FStringView View(static_cast<const FMacOSPath&>(I_Path).GetView());
        if (View.IsEmpty()) return 3;
        if (unlink(FString(View).Data()) == 0) return 0;
        if (errno == ENOENT) return 1;
        if (errno == EACCES || errno == EPERM) return 2;
        return 3;
    }

    TArray<FPath> FMacOSPlatformFileSystem::EnumerateFiles(const IPlatformPath& I_Directory, Bool I_bRecursive) const
    {
        TArray<FPath> Results;
        const FStringView DirView(static_cast<const FMacOSPath&>(I_Directory).GetView());
        if (DirView.IsEmpty() || !ExistsDirectory(I_Directory)) return Results;

        struct DirEntry { FString Path; Bool bRecurse; };
        TArray<DirEntry> Stack;
        FString DirStr(DirView);
        if (!DirStr.IsEmpty() && DirStr.Back() != '/') DirStr.Append('/');
        Stack.PushBack({ std::move(DirStr), I_bRecursive });

        while (!Stack.IsEmpty())
        {
            DirEntry E = Stack.Back();
            Stack.PopBack();
            DIR* D = opendir(E.Path.Data());
            if (!D) continue;

            struct dirent* Ent;
            while ((Ent = readdir(D)) != nullptr)
            {
                if (Ent->d_name[0] == '.' && (Ent->d_name[1] == '\0' || (Ent->d_name[1] == '.' && Ent->d_name[2] == '\0')))
                    continue;
                FString Full = E.Path;
                Full.Append(Ent->d_name);
                if (Ent->d_type == DT_DIR)
                {
                    if (E.bRecurse) Stack.PushBack({ Full + "/", True });
                }
                else if (Ent->d_type == DT_REG || Ent->d_type == DT_UNKNOWN)
                {
                    struct stat St;
                    if (stat(Full.Data(), &St) == 0 && S_ISREG(St.st_mode))
                        Results.PushBack(FPath(Full));
                }
            }
            closedir(D);
        }
        return Results;
    }

    TUniquePtr<FFile> FMacOSPlatformFileSystem::OpenFile(const IPlatformPath& I_Path, EFileMode I_Mode) const
    {
        const FStringView View(static_cast<const FMacOSPath&>(I_Path).GetView());
        if (View.IsEmpty()) return nullptr;

        const Bool bBinary = (I_Mode & EFileMode::Binary);
        const EFileMode Access = static_cast<EFileMode>(ToUnderlying(I_Mode) & ~ToUnderlying(EFileMode::Binary));
        const char* Mode = "r";
        if (Access & EFileMode::Read) Mode = bBinary ? "rb" : "r";
        else if (Access & EFileMode::Write) Mode = bBinary ? "wb" : "w";
        else if (Access & EFileMode::Append) Mode = bBinary ? "ab" : "a";
        else if (Access & EFileMode::ReadWrite) Mode = bBinary ? "r+b" : "r+";
        else if (Access & EFileMode::WriteRead) Mode = bBinary ? "w+b" : "w+";
        else if (Access & EFileMode::AppendRead) Mode = bBinary ? "a+b" : "a+";

        FILE* Fp = fopen(FString(View).Data(), Mode);
        if (!Fp)
        {
            LOG_DEBUG("OpenFile failed: {}", View);
            return nullptr;
        }
        return MakeUnique<FFile>(Fp);
    }
#endif

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
        auto [TempFile, TempPath] = CreateTempFileNear(*Parent, DefaultPrefix);
        if (!TempFile || !TempPath) return 3; // Other

        Bool bWriteOk = True;
        if (I_Size > 0 && TempFile->Write(I_Data, I_Size, 1) != 1)
            bWriteOk = False;
        TempFile->Flush();
        TempFile.Reset(); // close before ReplaceFile

        if (!bWriteOk)
        {
            const FString TempStr(static_cast<const FMacOSPath&>(*TempPath).GetView());
            std::remove(TempStr.Data());
            return 3; // Other
        }

        const Int32 ReplaceErr = ReplaceFile(*TempPath, I_Path);
        if (ReplaceErr != 0) // Success
        {
            const FString TempStr(static_cast<const FMacOSPath&>(*TempPath).GetView());
            std::remove(TempStr.Data());
            return ReplaceErr;
        }
        return 0; // Success
    }

    TPair<TUniquePtr<FFile>, TUniquePtr<IPlatformPath>> FMacOSPlatformFileSystem::
    CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const
    {
        TPair<TUniquePtr<FFile>, TUniquePtr<IPlatformPath>> Result;
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
        Result.first = MakeUnique<FFile>(Fp);
        Result.second = MakeUnique<FMacOSPath>(FStringView(TempPathStr));
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
