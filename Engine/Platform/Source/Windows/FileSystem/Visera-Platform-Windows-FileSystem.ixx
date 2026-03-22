module;
#include <Visera-Platform.hpp>
#include <io.h>
#include <fcntl.h>
#include <cstdio>
#include <windows.h>
#undef CreateFile
#undef DeleteFile
#undef CreateDirectory
#undef ReplaceFile
export module Visera.Platform.Windows.FileSystem;
#define VISERA_MODULE_NAME "Platform.Windows"
import Visera.Platform.Interface.FileSystem;
import Visera.Platform.Windows.Path;
import Visera.Core.OS.FileSystem;
import Visera.Core.Containers.Array;
import Visera.Core.Types.Path;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Types.String;
import Visera.Core.Types.Optional;
import Visera.Core.Types.Tuple;
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

    Bool FWindowsPlatformFileSystem::ExistsFile(const IPlatformPath& I_Path) const
    {
        const std::wstring Wide(static_cast<const FWindowsPath&>(I_Path));
        if (Wide.empty()) return False;
        const DWORD Attrs = GetFileAttributesW(Wide.c_str());
        return Attrs != INVALID_FILE_ATTRIBUTES && !(Attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    Bool FWindowsPlatformFileSystem::ExistsDirectory(const IPlatformPath& I_Path) const
    {
        const std::wstring Wide(static_cast<const FWindowsPath&>(I_Path));
        if (Wide.empty()) return False;
        const DWORD Attrs = GetFileAttributesW(Wide.c_str());
        return Attrs != INVALID_FILE_ATTRIBUTES && (Attrs & FILE_ATTRIBUTE_DIRECTORY);
    }

    Int32 FWindowsPlatformFileSystem::CreateDirectories(const IPlatformPath& I_Path) const
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
        const std::wstring Wide(static_cast<const FWindowsPath&>(I_Path));
        if (Wide.empty()) return 0;
        if (CreateDirectoryW(Wide.c_str(), nullptr) != 0) return 0;
        const DWORD Err = GetLastError();
        if (Err == ERROR_ALREADY_EXISTS) return ExistsDirectory(I_Path) ? 0 : 3;
        if (Err == ERROR_ACCESS_DENIED) return 2;
        return 3;
    }

    TOptional<TArray<FByte>> FWindowsPlatformFileSystem::ReadFile(const IPlatformPath& I_Path) const
    {
        auto F = OpenFile(I_Path, EFileMode::Read | EFileMode::Binary);
        if (!F || !F->IsOpen()) return NullOpt;
        return F->ReadAll();
    }

    Int32 FWindowsPlatformFileSystem::WriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const
    {
        auto F = OpenFile(I_Path, EFileMode::Write | EFileMode::Binary);
        if (!F || !F->IsOpen()) return 3;
        if (I_Size > 0 && I_Data != nullptr && F->Write(I_Data, I_Size, 1) != 1) return 3;
        return 0;
    }

    Int32 FWindowsPlatformFileSystem::DeleteFile(const IPlatformPath& I_Path) const
    {
        const std::wstring Wide(static_cast<const FWindowsPath&>(I_Path));
        if (Wide.empty()) return 3;
        if (::DeleteFileW(Wide.c_str()) != 0) return 0;
        const DWORD Err = GetLastError();
        if (Err == ERROR_FILE_NOT_FOUND || Err == ERROR_PATH_NOT_FOUND) return 1;
        if (Err == ERROR_ACCESS_DENIED) return 2;
        return 3;
    }

    TArray<FPath> FWindowsPlatformFileSystem::EnumerateFiles(const IPlatformPath& I_Directory, Bool I_bRecursive) const
    {
        TArray<FPath> Results;
        const std::wstring DirWide(static_cast<const FWindowsPath&>(I_Directory));
        if (DirWide.empty() || !ExistsDirectory(I_Directory)) return Results;

        struct DirEntry { std::wstring Path; Bool bRecurse; };
        TArray<DirEntry> Stack;
        Stack.PushBack({ DirWide + (DirWide.back() == L'\\' || DirWide.back() == L'/' ? L"" : L"\\"), I_bRecursive });

        while (!Stack.IsEmpty())
        {
            DirEntry E = Stack.Back();
            Stack.PopBack();
            const std::wstring Search = E.Path + L"*";
            WIN32_FIND_DATAW Fd;
            HANDLE H = FindFirstFileW(Search.c_str(), &Fd);
            if (H == INVALID_HANDLE_VALUE) continue;

            do
            {
                if (Fd.cFileName[0] == L'.' && (Fd.cFileName[1] == L'\0' || (Fd.cFileName[1] == L'.' && Fd.cFileName[2] == L'\0')))
                    continue;
                const std::wstring Full = E.Path + Fd.cFileName;
                if (Fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (E.bRecurse) Stack.PushBack({ Full + L"\\", True });
                }
                else
                {
                    Results.PushBack(FPath(FWindowsPath(Full).ToPath()));
                }
            } while (FindNextFileW(H, &Fd) != 0);
            FindClose(H);
        }
        return Results;
    }

    TUniquePtr<FFile> FWindowsPlatformFileSystem::OpenFile(const IPlatformPath& I_Path, EFileMode I_Mode) const
    {
        const std::wstring Wide(static_cast<const FWindowsPath&>(I_Path));
        if (Wide.empty()) return nullptr;

        const Bool bBinary = (I_Mode & EFileMode::Binary);
        const EFileMode Access = static_cast<EFileMode>(ToUnderlying(I_Mode) & ~ToUnderlying(EFileMode::Binary));
        const wchar_t* Mode = L"r";
        if (Access & EFileMode::Read) Mode = bBinary ? L"rb" : L"r";
        else if (Access & EFileMode::Write) Mode = bBinary ? L"wb" : L"w";
        else if (Access & EFileMode::Append) Mode = bBinary ? L"ab" : L"a";
        else if (Access & EFileMode::ReadWrite) Mode = bBinary ? L"r+b" : L"r+";
        else if (Access & EFileMode::WriteRead) Mode = bBinary ? L"w+b" : L"w+";
        else if (Access & EFileMode::AppendRead) Mode = bBinary ? L"a+b" : L"a+";

        FILE* Fp = _wfopen(Wide.c_str(), Mode);
        if (!Fp)
        {
            const auto PathStr = FWindowsPath(Wide).ToPath().GetString().GetNative();
            LOG_DEBUG("OpenFile failed: {}", PathStr);
            return nullptr;
        }
        return MakeUnique<FFile>(Fp);
    }

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

        // GetTempFileNameW requires an existing directory; create parent chain if needed (e.g. first write to assets/shaders/).
        if (!Parent->IsEmpty() && !ExistsDirectory(*Parent))
        {
            const Int32 MkErr = CreateDirectories(*Parent);
            if (MkErr != 0) { return MkErr; }
        }

        static const FWindowsPath DefaultPrefix(L".vtemp-");
        auto [TempFile, TempPath] = CreateTempFileNear(*Parent, DefaultPrefix);
        if (!TempFile || !TempPath) return 3; // Other

        Bool bWriteOk = True;
        if (I_Size > 0 && TempFile->Write(I_Data, I_Size, 1) != 1)
            bWriteOk = False;
        TempFile->Flush();
        TempFile.Reset(); // close before ReplaceFile

        if (!bWriteOk)
        {
            const std::wstring TempWide(static_cast<const FWindowsPath&>(*TempPath));
            DeleteFileW(TempWide.c_str());
            return 3; // Other
        }

        const Int32 ReplaceErr = ReplaceFile(*TempPath, I_Path);
        if (ReplaceErr != 0) // Success
        {
            const std::wstring TempWide(static_cast<const FWindowsPath&>(*TempPath));
            DeleteFileW(TempWide.c_str());
            return ReplaceErr;
        }
        return 0; // Success
    }

    TPair<TUniquePtr<FFile>, TUniquePtr<IPlatformPath>> FWindowsPlatformFileSystem::
    CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const
    {
        TPair<TUniquePtr<FFile>, TUniquePtr<IPlatformPath>> Result;
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

        Result.first = MakeUnique<FFile>(Fp);
        Result.second = MakeUnique<FWindowsPath>(TempPath);
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
