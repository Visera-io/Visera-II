module;
#include <Visera-Core.hpp>
#include <fstream>
#include <filesystem>
#include <system_error>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.OS.FileSystem.File;
export import Visera.Core.Types.Path;
export import Visera.Core.Containers.Array;
export import Visera.Core.Traits.Flags;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer.Unique;
       import Visera.Core.Log;

export namespace Visera
{
    enum class EIOError : UInt8
    {
        None             = 0,
        NotFound         = 1,
        PermissionDenied  = 2,
        Other            = 3,
    };

    [[nodiscard]] inline EIOError ToEIOError(const std::error_code& I_Ec) noexcept
    {
        if (!I_Ec) return EIOError::None;
        if (I_Ec == std::errc::no_such_file_or_directory) return EIOError::NotFound;
        if (I_Ec == std::errc::permission_denied) return EIOError::PermissionDenied;
        return EIOError::Other;
    }

    inline std::filesystem::path ToFilesystemPath(const FPath& I_Path) noexcept
    {
        return std::filesystem::path(I_Path.GetString().GetNative());
    }

    enum class EStreamMode : Int32
    {
        None       = 0,
        Binary     = std::ios_base::binary,
    };
    VISERA_MAKE_FLAGS(EStreamMode);

    enum class EFileMode : Int32
    {
        Binary     = 1 << 0,
        Read       = 1 << 1,
        Write      = 1 << 2,
        Append     = 1 << 3,
        ReadWrite  = 1 << 4,
        WriteRead  = 1 << 5,
        AppendRead = 1 << 6,
    };
    VISERA_MAKE_FLAGS(EFileMode);

    class VISERA_CORE_API FFileSystem
    {
    public:
        [[nodiscard]] EIOError static inline
        CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath);
        [[nodiscard]] Bool static inline
        IsDirectory(const FPath& I_Path) { return std::filesystem::is_directory(ToFilesystemPath(I_Path)); }
        [[nodiscard]] EIOError static inline
        CreateDirectory(const FPath& I_Path);
        [[nodiscard]] EIOError static inline
        DeleteDirectory(const FPath& I_Path, Bool I_bForce = False);
        [[nodiscard]] Bool static inline
        Exists(const FPath& I_Path) { return std::filesystem::exists(ToFilesystemPath(I_Path)); }
        [[nodiscard]] TUniquePtr<std::ifstream> static inline
        OpenIStream(const FPath& I_Path, EStreamMode I_Mode = EStreamMode::None);
        [[nodiscard]] TUniquePtr<std::ofstream> static inline
        OpenOStream(const FPath& I_Path, EStreamMode I_Mode = EStreamMode::None);
        [[nodiscard]] TUniquePtr<FFile> static inline
        OpenFile(const FPath& I_Path, EFileMode I_Mode);
        [[nodiscard]] static TArray<FPath>
        EnumerateFiles(const FPath& I_Directory, Bool I_bRecursive = False);

    public:
        explicit FFileSystem() = default; // Must have a default constructor
        explicit FFileSystem(const FFileSystem& I_Another)       = default;
        explicit FFileSystem(FFileSystem&& I_Another)   noexcept = default;
        FFileSystem& operator=(const FFileSystem& I_Another)     = default;
        FFileSystem& operator=(FFileSystem&& I_Another) noexcept = default;

    private:
        [[nodiscard]] static inline const char*
        GetFileModeString(EFileMode I_Mode);
    };

    EIOError FFileSystem::
    CreateDirectory(const FPath& I_Path)
    {
        const auto Path = ToFilesystemPath(I_Path);
        std::error_code Ec;
        if (!std::filesystem::exists(Path, Ec))
        {
            std::filesystem::create_directories(Path, Ec);
        }
        if (Ec) { LOG_DEBUG("CreateDirectory failed: {} - {}", Path.generic_string(), Ec.message()); }
        return ToEIOError(Ec);
    }

    EIOError FFileSystem::
    DeleteDirectory(const FPath& I_Path, Bool I_bForce/* = False*/)
    {
        const auto Path = ToFilesystemPath(I_Path);
        std::error_code Ec;
        if (std::filesystem::exists(Path, Ec) && std::filesystem::is_directory(Path, Ec))
        {
            if (I_bForce)
            { std::filesystem::remove_all(Path, Ec); }
            else
            {
                if (std::filesystem::is_empty(Path, Ec))
                { std::filesystem::remove(Path, Ec); }
            }
        }
        if (Ec) { LOG_DEBUG("DeleteDirectory failed: {} - {}", Path.generic_string(), Ec.message()); }
        return ToEIOError(Ec);
    }

    EIOError FFileSystem::
    CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath)
    {
        const auto SourcePath = ToFilesystemPath(I_SourcePath);
        const auto TargetPath = ToFilesystemPath(I_TargetPath);
        std::error_code Ec;
        if (!std::filesystem::exists(TargetPath, Ec))
        { return ToEIOError(Ec); }
        if (std::filesystem::exists(SourcePath, Ec))
        { return ToEIOError(Ec); }
        std::filesystem::create_symlink(TargetPath, SourcePath, Ec);
        if (Ec) { LOG_DEBUG("CreateSoftLink failed: {} -> {} - {}", SourcePath.generic_string(), TargetPath.generic_string(), Ec.message()); }
        return ToEIOError(Ec);
    }

    TUniquePtr<std::ifstream> FFileSystem::
    OpenIStream(const FPath& I_Path, EStreamMode I_Mode)
    {
        const auto Path = ToFilesystemPath(I_Path);
        auto IStream = MakeUnique<std::ifstream>(Path, ToUnderlying(I_Mode));
        if (!IStream->is_open())
        {
            LOG_DEBUG("Failed to open input stream: {}", Path.generic_string());
            return nullptr;
        }
        return std::move(IStream);
    }

    TUniquePtr<std::ofstream> FFileSystem::
    OpenOStream(const FPath& I_Path, EStreamMode I_Mode)
    {
        const auto Path = ToFilesystemPath(I_Path);
        auto OStream = MakeUnique<std::ofstream>(Path, ToUnderlying(I_Mode));
        if (!OStream->is_open())
        {
            LOG_DEBUG("Failed to open output stream: {}", Path.generic_string());
            return nullptr;
        }
        return std::move(OStream);
    }

    inline const char* FFileSystem::
    GetFileModeString(EFileMode I_Mode)
    {
        const Bool bBinary = (I_Mode & EFileMode::Binary);
        const EFileMode AccessMode = static_cast<EFileMode>(ToUnderlying(I_Mode) & ~ToUnderlying(EFileMode::Binary));

        if (AccessMode & EFileMode::Read)
        { return bBinary ? "rb" : "r"; }
        else if (AccessMode & EFileMode::Write)
        { return bBinary ? "wb" : "w"; }
        else if (AccessMode & EFileMode::Append)
        { return bBinary ? "ab" : "a"; }
        else if (AccessMode & EFileMode::ReadWrite)
        { return bBinary ? "r+b" : "r+"; }
        else if (AccessMode & EFileMode::WriteRead)
        { return bBinary ? "w+b" : "w+"; }
        else if (AccessMode & EFileMode::AppendRead)
        { return bBinary ? "a+b" : "a+"; }
        else
        { return bBinary ? "rb" : "r"; } // Default to read mode
    }

    TUniquePtr<FFile> FFileSystem::
    OpenFile(const FPath& I_Path, EFileMode I_Mode)
    {
        const char* ModeStr = GetFileModeString(I_Mode);
        const auto Path = ToFilesystemPath(I_Path);
        const std::string PathString = Path.generic_string();
        FILE* Handle = std::fopen(PathString.c_str(), ModeStr);
        if (Handle == nullptr)
        {
            LOG_DEBUG("Failed to open file: {}", PathString);
            return nullptr;
        }
        return MakeUnique<FFile>(Handle);
    }

    TArray<FPath> FFileSystem::
    EnumerateFiles(const FPath& I_Directory, Bool I_bRecursive)
    {
        TArray<FPath> Results;
        const auto DirPath = ToFilesystemPath(I_Directory);
        std::error_code Ec;
        if (!std::filesystem::exists(DirPath, Ec) || !std::filesystem::is_directory(DirPath, Ec))
        { return Results; }

        try
        {
            if (I_bRecursive)
            {
                for (const auto& Entry : std::filesystem::recursive_directory_iterator(DirPath, Ec))
                {
                    if (Entry.is_regular_file(Ec))
                    { Results.PushBack(FPath(Entry.path().u8string())); }
                }
            }
            else
            {
                for (const auto& Entry : std::filesystem::directory_iterator(DirPath, Ec))
                {
                    if (Entry.is_regular_file(Ec))
                    { Results.PushBack(FPath(Entry.path().u8string())); }
                }
            }
        }
        catch (const std::filesystem::filesystem_error& I_Err)
        {
            LOG_DEBUG("EnumerateFiles failed: {}", I_Err.what());
        }

        return Results;
    }
}