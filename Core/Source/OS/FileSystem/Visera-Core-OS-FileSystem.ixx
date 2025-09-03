module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.Log;
import Visera.Core.Types.Path;

export namespace Visera
{
    using SFileSystemError = std::filesystem::filesystem_error;

    class VISERA_CORE_API FFileSystem
    {
    public:
        [[nodiscard]] Bool static inline
        CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath);

    public:
        [[nodiscard]] inline const auto&
        GetRoot() const { return Root; }
        [[nodiscard]] Bool inline
        CreateDirectory(const FPath& I_RelativePath) const;
        [[nodiscard]] Bool inline
        DeleteDirectory(const FPath& I_RelativePath) const;

        explicit FFileSystem() = delete;
        explicit FFileSystem(const FPath& I_Root) : Root{I_Root} {}

    private:
        FPath Root;
        TMap<FString, FPath> Data{};
    };


    Bool FFileSystem::
    CreateDirectory(const FPath& I_RelativePath) const
    {
        const auto Path = Root/I_RelativePath;
        if (!std::filesystem::exists(Path.GetNativePath()))
        {
            if (!std::filesystem::create_directories(Path.GetNativePath()))
            { LOG_ERROR("Failed to create directory at \"{}\"", Path); }
            else
            { LOG_INFO("Created a new directory at \"{}\"", Path); return True; }
        }
        else { LOG_ERROR("Failed to create an existent directory \"{}\"!", Path) }

        return False;
    }

    Bool FFileSystem::
    DeleteDirectory(const FPath& I_RelativePath) const
    {
        const auto Path = Root/I_RelativePath;
        if (std::filesystem::exists(Path.GetNativePath())      &&
            std::filesystem::is_directory(Path.GetNativePath()))
        {
            if (!std::filesystem::is_empty(Path.GetNativePath()))
            {
                LOG_ERROR("Failed to delete an non-empty directory at \"{}\"!", Path);
                return False;
            }

            if (!std::filesystem::remove(Path.GetNativePath()))
            { LOG_ERROR("Failed to delete the directory \"{}\"", Path); }
            else
            { LOG_INFO("Deleted the directory \"{}\"", Path); return True;}
        }
        else { LOG_ERROR("Failed to delete an inexistent directory \"{}\"!", Path); }

        return False;
    }

    Bool FFileSystem::
    CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath)
    {
        if (!std::filesystem::exists(I_TargetPath.GetNativePath()))
        {
            LOG_ERROR("Failed to create soft link to an non-existent directory \"{}\"!", I_TargetPath);
            return False;
        }

        if (!std::filesystem::exists(I_SourcePath.GetNativePath()))
        {
            std::filesystem::create_symlink(I_SourcePath.GetNativePath(),
                                            I_TargetPath.GetNativePath());
            LOG_INFO("Created soft link \"{}\" -> \"{}\"", I_SourcePath, I_TargetPath);
            return True;
        }

        return False;
    }

}
