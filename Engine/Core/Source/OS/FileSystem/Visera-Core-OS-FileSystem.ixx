module;
#include <Visera-Core.hpp>
#include <fstream>
#include <filesystem>
#include <cstdio>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.OS.FileSystem.File;
export import Visera.Core.Types.Path;
export import Visera.Core.Traits.Flags;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    using SFileSystemError = std::filesystem::filesystem_error;

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
        [[nodiscard]] FErrorCode static inline
        CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath);
        [[nodiscard]] Bool static inline
        IsDirectory(const FPath& I_Path) { return std::filesystem::is_directory(I_Path.GetNativePath()); }
        [[nodiscard]] FErrorCode static inline
        CreateDirectory(const FPath& I_Path);
        [[nodiscard]] FErrorCode static inline
        DeleteDirectory(const FPath& I_Path, Bool I_bForce = False);
        [[nodiscard]] Bool static inline
        Exists(const FPath& I_Path) { return std::filesystem::exists(I_Path.GetNativePath()); }
        [[nodiscard]] TUniquePtr<std::ifstream> static inline
        OpenIStream(const FPath& I_Path, EStreamMode I_Mode = EStreamMode::None);
        [[nodiscard]] TUniquePtr<std::ofstream> static inline
        OpenOStream(const FPath& I_Path, EStreamMode I_Mode = EStreamMode::None);
        [[nodiscard]] TUniquePtr<FFile> static inline
        OpenFile(const FPath& I_Path, EFileMode I_Mode);

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

    FErrorCode FFileSystem::
    CreateDirectory(const FPath& I_Path)
    {
        FErrorCode ErrorCode;
        if (!std::filesystem::exists(I_Path.GetNativePath(), ErrorCode))
        {
            std::filesystem::create_directories(I_Path.GetNativePath(), ErrorCode);
        }
        return ErrorCode;
    }

    FErrorCode FFileSystem::
    DeleteDirectory(const FPath& I_Path, Bool I_bForce/* = False*/)
    {
        FErrorCode ErrorCode;

        if (std::filesystem::exists(I_Path.GetNativePath(), ErrorCode)      &&
            std::filesystem::is_directory(I_Path.GetNativePath(), ErrorCode))
        {
            if (I_bForce)
            { std::filesystem::remove_all(I_Path.GetNativePath(), ErrorCode); }
            else
            {
                if (std::filesystem::is_empty(I_Path.GetNativePath(), ErrorCode))
                { std::filesystem::remove(I_Path.GetNativePath(), ErrorCode); }
            }
        }
        return ErrorCode;
    }

    FErrorCode FFileSystem::
    CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath)
    {
        FErrorCode ErrorCode;

        // I_TargetPath should exist (the file/directory to link to)
        if (!std::filesystem::exists(I_TargetPath.GetNativePath(), ErrorCode))
        { return ErrorCode; }

        // I_SourcePath should not exist (the symlink to be created)
        if (std::filesystem::exists(I_SourcePath.GetNativePath(), ErrorCode))
        { return ErrorCode; }

        // create_symlink(target, link_path) - create a symlink at link_path pointing to target
        std::filesystem::create_symlink(I_TargetPath.GetNativePath(),
                                        I_SourcePath.GetNativePath(),
                                        ErrorCode);
        return ErrorCode;
    }

    TUniquePtr<std::ifstream> FFileSystem::
    OpenIStream(const FPath& I_Path, EStreamMode I_Mode)
    {
        auto IStream = MakeUnique<std::ifstream>(I_Path.GetNativePath(), ToUnderlying(I_Mode));
        return IStream->is_open()? std::move(IStream) : nullptr;
    }

    TUniquePtr<std::ofstream> FFileSystem::
    OpenOStream(const FPath& I_Path, EStreamMode I_Mode)
    {
        auto OStream = MakeUnique<std::ofstream>(I_Path.GetNativePath(), ToUnderlying(I_Mode));
        return OStream->is_open()? std::move(OStream) : nullptr;
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
        const FString PathString = I_Path.GetUTF8Path();
        FILE* Handle = std::fopen(PathString.Data(), ModeStr);
        if (Handle == nullptr) { return nullptr; }
        return MakeUnique<FFile>(Handle);
    }
}