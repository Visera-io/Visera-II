module;
#include <Visera-Core.hpp>
#include <fstream>
#include <filesystem>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.Types.Path;
       import Visera.Core.Traits.Flags;

export namespace Visera
{
    using SFileSystemError = std::filesystem::filesystem_error;

    enum class EIOMode : Int32
    {
        None   = 0,
        Binary = std::ios_base::binary,
    };
    VISERA_MAKE_FLAGS(EIOMode);

    class VISERA_CORE_API FFileSystem
    {
    public:
        [[nodiscard]] FErrorCode static inline
        CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath);
        [[nodiscard]] FErrorCode static inline
        CreateDirectory(const FPath& I_Path);
        [[nodiscard]] FErrorCode static inline
        DeleteDirectory(const FPath& I_Path, Bool I_bForce = False);
        [[nodiscard]] Bool static inline
        Exists(const FPath& I_Path) { return std::filesystem::exists(I_Path.GetNativePath()); }
        [[nodiscard]] TUniquePtr<std::ifstream> static inline
        OpenIStream(const FPath& I_Path, EIOMode I_Mode = EIOMode::None);
        [[nodiscard]] TUniquePtr<std::ofstream> static inline
        OpenOStream(const FPath& I_Path, EIOMode I_Mode = EIOMode::None);
        [[nodiscard]] Bool static inline
        IsDirectory(const FPath& I_Path) { return std::filesystem::is_directory(I_Path.GetNativePath()); }

    public:
        [[nodiscard]] inline Bool
        HasRoot() const { return !Root.IsEmpty(); }
        [[nodiscard]] inline const FPath&
        GetRoot() const { return Root; }
        [[nodiscard]] FPath
        SearchFile(const FPath& I_FileName) const;

        explicit FFileSystem() = default; // Must have a default constructor
        explicit FFileSystem(const FFileSystem& I_Another)       = default;
        explicit FFileSystem(FFileSystem&& I_Another)   noexcept = default;
        FFileSystem& operator=(const FFileSystem& I_Another)     = default;
        FFileSystem& operator=(FFileSystem&& I_Another) noexcept = default;
        explicit FFileSystem(const FPath& I_Root) : Root{I_Root} {}

    private:
        FPath           Root{};
    };

    FPath FFileSystem::
    SearchFile(const FPath& I_FileName) const
    {
        const FPath Path = Root / I_FileName;

        if (Exists(Path) && !IsDirectory(Path))
        { return Path; }

        return FPath{};
    }

    FErrorCode FFileSystem::
    CreateDirectory(const FPath& I_Path)
    {
        FErrorCode ErrorCode;
        if (std::filesystem::exists(I_Path.GetNativePath(), ErrorCode))
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
                {
                    std::filesystem::remove(I_Path.GetNativePath(), ErrorCode);
                }
            }
        }
        return ErrorCode;
    }

    FErrorCode FFileSystem::
    CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath)
    {
        FErrorCode ErrorCode;

        if (!std::filesystem::exists(I_TargetPath.GetNativePath(), ErrorCode))
        { return ErrorCode; }

        if (std::filesystem::exists(I_SourcePath.GetNativePath(), ErrorCode))
        {
            std::filesystem::create_symlink(I_TargetPath.GetNativePath(),
                                            I_SourcePath.GetNativePath(),
                                            ErrorCode);
        }
        return ErrorCode;
    }

    TUniquePtr<std::ifstream> FFileSystem::
    OpenIStream(const FPath& I_Path, EIOMode I_Mode)
    {
        auto IStream = MakeUnique<std::ifstream>(I_Path.GetNativePath(), ToUnderlying(I_Mode));
        return IStream->is_open()? std::move(IStream) : nullptr;
    }

    TUniquePtr<std::ofstream> FFileSystem::
    OpenOStream(const FPath& I_Path, EIOMode I_Mode)
    {
        auto OStream = MakeUnique<std::ofstream>(I_Path.GetNativePath(), ToUnderlying(I_Mode));
        return OStream->is_open()? std::move(OStream) : nullptr;
    }
}