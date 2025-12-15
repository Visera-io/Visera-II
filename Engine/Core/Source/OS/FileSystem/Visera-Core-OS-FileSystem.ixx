module;
#include <Visera-Core.hpp>
#include <fstream>
#include <filesystem>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.Types.Path;
       import Visera.Core.Traits.Flags;
       import Visera.Core.Log;

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
        [[nodiscard]] Bool static inline
        CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath);
        [[nodiscard]] Bool static inline
        CreateDirectory(const FPath& I_Path);
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

        [[nodiscard]] Bool inline
        CreateFile(const FPath& I_FileName) const;
        [[nodiscard]] Bool inline
        DeleteDirectory(const FPath& I_RelativePath, Bool I_bForce = False) const;

        explicit FFileSystem() = default; // Must have a default constructor
        explicit FFileSystem(const FFileSystem& I_Another)       = default;
        explicit FFileSystem(FFileSystem&& I_Another)   noexcept = default;
        FFileSystem& operator=(const FFileSystem& I_Another)     = default;
        FFileSystem& operator=(FFileSystem&& I_Another) noexcept = default;
        explicit FFileSystem(const FPath& I_Root) : Root{I_Root} {}

    private:
        FPath Root{};
    };

    FPath FFileSystem::
    SearchFile(const FPath& I_FileName) const
    {
        const FPath Path = Root / I_FileName;

        if (Exists(Path) && !IsDirectory(Path))
        { return Path; }

        return FPath{};
    }

    Bool FFileSystem::
    CreateFile(const FPath& I_FileName) const
    {
        const auto Path = GetRoot()/I_FileName;
        std::error_code ErrorCode{};

        if (!std::filesystem::exists(Path.GetNativePath(), ErrorCode))
        {
            // Create the file by opening a stream
            std::ofstream File(Path.GetNativePath());
            if (!File)
            {
                LOG_ERROR("Failed to create file at \"{}\" -- {}.",
                          Path, ErrorCode.message());
                return False;
            }
            LOG_INFO("Created a new file at \"{}\".", Path);
            return True;
        }

        LOG_ERROR("Failed to create the file \"{}\" -- {}.",
                  Path, ErrorCode.message());

        return False;
    }

    Bool FFileSystem::
    CreateDirectory(const FPath& I_Path)
    {
        std::error_code ErrorCode{};

        if (!std::filesystem::exists(I_Path.GetNativePath(), ErrorCode))
        {
            if (!std::filesystem::create_directories(I_Path.GetNativePath(), ErrorCode))
            { LOG_ERROR("Failed to create directory at \"{}\" -- {}.", I_Path, ErrorCode.message()); }
            else
            { LOG_INFO("Created a new directory at \"{}\"", I_Path); return True; }
        }
        else { LOG_ERROR("Failed to create the directory \"{}\" -- {}.", I_Path, ErrorCode.message()); }

        return False;
    }

    Bool FFileSystem::
    DeleteDirectory(const FPath& I_RelativePath, Bool I_bForce/* = False*/) const
    {
        const auto Path = GetRoot()/I_RelativePath;
        std::error_code ErrorCode{};

        if (std::filesystem::exists(Path.GetNativePath(), ErrorCode)      &&
            std::filesystem::is_directory(Path.GetNativePath(), ErrorCode))
        {
            if (I_bForce)
            {
                if (!std::filesystem::remove_all(Path.GetNativePath(), ErrorCode))
                { LOG_ERROR("Failed to delete the directory \"{}\" -- {}.", Path, ErrorCode.message()); }
                else
                { LOG_INFO("Deleted the directory \"{}\"", Path); return True;}
            }

            if (!std::filesystem::is_empty(Path.GetNativePath(), ErrorCode))
            {
                LOG_ERROR("Failed to delete the directory \"{}\" -- {}.", Path, ErrorCode.message());
                return False;
            }

            if (!std::filesystem::remove(Path.GetNativePath(), ErrorCode))
            { LOG_ERROR("Failed to delete the directory \"{}\" -- {}.", Path, ErrorCode.message()); }
            else
            { LOG_INFO("Deleted the directory \"{}\"", Path); return True;}
        }
        else { LOG_ERROR("Failed to delete the directory \"{}\" -- {}.", Path, ErrorCode.message()); }

        return False;
    }

    Bool FFileSystem::
    CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath)
    {
        std::error_code ErrorCode{};

        if (!std::filesystem::exists(I_TargetPath.GetNativePath(), ErrorCode))
        {
            LOG_ERROR("Failed to create a soft link to the directory \"{}\" -- {}.", I_TargetPath, ErrorCode.message());
            return False;
        }

        if (std::filesystem::exists(I_SourcePath.GetNativePath(), ErrorCode))
        {
            std::filesystem::create_symlink(I_TargetPath.GetNativePath(),
                                            I_SourcePath.GetNativePath(),
                                            ErrorCode);
            if (ErrorCode)
            { LOG_ERROR("Failed to create a soft link \"{}\" -> \"{}\" -- {}.", I_SourcePath, I_TargetPath, ErrorCode.message()); }
            else
            { LOG_INFO("Created a soft link \"{}\" -> \"{}\"", I_SourcePath, I_TargetPath); }
            return True;
        }
        else { LOG_ERROR("Failed to create a soft link \"{}\" -> \"{}\" -- {}.", I_SourcePath, I_TargetPath, ErrorCode.message()); }

        return False;
    }

    TUniquePtr<std::ifstream> FFileSystem::
    OpenIStream(const FPath& I_Path, EIOMode I_Mode)
    {
        auto IStream = MakeUnique<std::ifstream>(I_Path.GetNativePath(), ToUnderlying(I_Mode));
        if (!IStream->is_open())
        {
            LOG_ERROR("Failed to open istream \"{}\" (mode:{})!",
                      I_Path, ToUnderlying(I_Mode));
            return nullptr;
        }
        return IStream;
    }
    TUniquePtr<std::ofstream> FFileSystem::
    OpenOStream(const FPath& I_Path, EIOMode I_Mode)
    {
        auto OStream = MakeUnique<std::ofstream>(I_Path.GetNativePath(), ToUnderlying(I_Mode));
        if (!OStream->is_open())
        {
            LOG_ERROR("Failed to open ostream \"{}\" (mode:{})!",
                      I_Path, ToUnderlying(I_Mode));
            return nullptr;
        }
        return OStream;
    }
}