module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.Log;
import Visera.Core.Types.Path;

export namespace Visera
{
    class VISERA_CORE_API FFileSystem
    {
    public:
        [[nodiscard]] inline const auto&
        GetRoot() const { return Root; }
        void inline
        CreateDirectory(const FPath& I_Path) const;
        void inline
        DeleteDirectory(const FPath& I_Path) const;

        explicit FFileSystem() = default;
        explicit FFileSystem(const FPath& I_Root) : Root{I_Root} {}

    private:
        FPath Root{FText(std::filesystem::current_path().u8string())};
        TMap<FString, FPath> Data{};
    };


    void FFileSystem::
    CreateDirectory(const FPath& I_Path) const
    {
        const auto Path = Root/I_Path;
        if (!std::filesystem::exists(Path.GetNativePath()))
        {
            LOG_INFO("Creating a new directory at \"{}\"", Path);
            if (!std::filesystem::create_directories(Path.GetNativePath()))
            { LOG_ERROR("Failed to create directory at \"{}\"", Path); }
        }
        else { LOG_ERROR("Directory \"{}\" exists!", Path) }
    }

    void FFileSystem::
    DeleteDirectory(const FPath& I_Path) const
    {
        const auto Path = Root/I_Path;
        if (std::filesystem::exists(Path.GetNativePath())      &&
            std::filesystem::is_directory(Path.GetNativePath()))
        {
            LOG_INFO("Deleting the directory \"{}\"", Path);
            if (!std::filesystem::remove(Path.GetNativePath()))
            { LOG_ERROR("Failed to delete the directory \"{}\"", Path); }
        }
        else { LOG_ERROR("Directory \"{}\" does NOT exist!", Path); }
    }

}
