module;
#include <windows.h>
#undef CreateFile
#undef CreateDirectory
#undef ReplaceFile
#include <io.h>
#include <fcntl.h>
#include <cstdio>
#include <Visera-Platform.hpp>
#include <Visera-Core.hpp>
export module Visera.Platform.Windows.FileSystem;
#define VISERA_MODULE_NAME "Platform.Windows"
import Visera.Platform.Interface.FileSystem;
import Visera.Platform.Windows.Path;
import Visera.Core.OS.FileSystem;
import Visera.Core.Types.Pointer.Unique;
import Visera.OS.FileSystem.File;
import Visera.Core.Log;

export namespace Visera
{
    enum class EWindowsIOStatus : UInt8
    {
        Success          = 0,
        NotFound         = 1,
        PermissionDenied = 2,
        Other            = 3,
    };

    class VISERA_PLATFORM_API FWindowsPlatformFileSystem : public IPlatformFileSystem
    {
    public:
        [[nodiscard]] Int32
        ReplaceFile(const IPlatformPath& I_Source, const IPlatformPath& I_Target) const override;
        [[nodiscard]] Int32
        AtomicWriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const override;
        [[nodiscard]] FTempFileResult
        CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const override;
    };

    Int32 FWindowsPlatformFileSystem::
    ReplaceFile(const IPlatformPath& I_Source, const IPlatformPath& I_Target) const
    {
        const std::wstring SourceWide(static_cast<const FWindowsPath&>(I_Source));
        const std::wstring TargetWide(static_cast<const FWindowsPath&>(I_Target));
        if (SourceWide.empty() || TargetWide.empty()) return 3; // Other

        BOOL Replaced = ReplaceFileW(
            TargetWide.c_str(),
            SourceWide.c_str(),
            nullptr,
            REPLACEFILE_IGNORE_MERGE_ERRORS,
            nullptr,
            nullptr);

        if (!Replaced)
        {
            const DWORD Err = GetLastError();
            if (Err == ERROR_FILE_NOT_FOUND)
                Replaced = MoveFileExW(SourceWide.c_str(), TargetWide.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
            if (!Replaced)
            {
                if (GetLastError() == ERROR_ACCESS_DENIED) return 2; // PermissionDenied
                if (GetLastError() == ERROR_FILE_NOT_FOUND) return 1; // NotFound
                return 3; // Other
            }
        }
        return 0; // Success
    }

    Int32 FWindowsPlatformFileSystem::
    AtomicWriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const
    {
        if (I_Data == nullptr && I_Size > 0)
            return 3; // Other

        TUniquePtr<IPlatformPath> Parent = I_Path.GetParent();
        if (!Parent) return 3; // Other

        static const FWindowsPath DefaultPrefix(L".VTemp-");
        FTempFileResult Temp = CreateTempFileNear(*Parent, DefaultPrefix);
        if (!Temp.File || !Temp.Path) return 3; // Other

        Bool bWriteOk = True;
        if (I_Size > 0 && Temp.File->Write(I_Data, I_Size, 1) != 1)
            bWriteOk = False;
        Temp.File->Flush();
        Temp.File.Reset(); // close before ReplaceFile

        if (!bWriteOk)
        {
            const std::wstring TempWide(static_cast<const FWindowsPath&>(*Temp.Path));
            DeleteFileW(TempWide.c_str());
            return 3; // Other
        }

        const Int32 ReplaceErr = ReplaceFile(*Temp.Path, I_Path);
        if (ReplaceErr != 0) // Success
        {
            const std::wstring TempWide(static_cast<const FWindowsPath&>(*Temp.Path));
            DeleteFileW(TempWide.c_str());
            return ReplaceErr;
        }
        return 0; // Success
    }

    FTempFileResult FWindowsPlatformFileSystem::
    CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const
    {
        FTempFileResult Result;
        const std::wstring DirWide(static_cast<const FWindowsPath&>(I_Directory));
        if (DirWide.empty()) return Result;

        std::wstring Prefix = L".VT";
        if (!I_Prefix.IsEmpty())
        {
            if (TUniquePtr<IPlatformPath> FileName = I_Prefix.GetFileName())
            {
                const std::wstring_view Name(static_cast<const FWindowsPath&>(*FileName));
                Prefix = std::wstring(Name.substr(0, std::min(Name.size(), size_t(3))));
            }
        }

        wchar_t TempPath[MAX_PATH];
        if (GetTempFileNameW(DirWide.c_str(), Prefix.c_str(), 0, TempPath) == 0)
        {
            LOG_DEBUG("CreateTempFileNear: GetTempFileNameW failed");
            return Result;
        }

        HANDLE H = CreateFileW(
            TempPath,
            GENERIC_READ | GENERIC_WRITE,
            0,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_WRITE_THROUGH,
            nullptr);

        if (H == INVALID_HANDLE_VALUE)
        {
            DeleteFileW(TempPath);
            return Result;
        }

        const int Fd = _open_osfhandle(reinterpret_cast<intptr_t>(H), _O_RDWR);
        if (Fd < 0)
        {
            CloseHandle(H);
            DeleteFileW(TempPath);
            return Result;
        }

        FILE* Fp = _fdopen(Fd, "w+b");
        if (!Fp)
        {
            _close(Fd);
            DeleteFileW(TempPath);
            return Result;
        }

        Result.File = MakeUnique<FFile>(Fp);
        Result.Path = MakeUnique<FWindowsPath>(TempPath);
        return Result;
    }
}

VISERA_MAKE_FORMATTER(Visera::EWindowsIOStatus,
    const char* Name = "Unknown";
    switch (I_Formatee)
    {
        case Visera::EWindowsIOStatus::Success:          Name = "Success";          break;
        case Visera::EWindowsIOStatus::NotFound:         Name = "NotFound";         break;
        case Visera::EWindowsIOStatus::PermissionDenied: Name = "PermissionDenied"; break;
        case Visera::EWindowsIOStatus::Other:            Name = "Other";            break;
        default: break;
    },
    "{}", Name
);
