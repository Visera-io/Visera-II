module;
#include <Visera-Core.hpp>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.Core.Types.Path;
       import Visera.Core.Log;

export namespace Visera
{
    using SFileSystemError = std::filesystem::filesystem_error;

    class VISERA_CORE_API FFileSystem
    {
    public:
        [[nodiscard]] Bool static inline
        CreateSoftLink(const FPath& I_SourcePath, const FPath& I_TargetPath);
        [[nodiscard]] Bool static inline
        Exists(const FPath& I_Path) { return std::filesystem::exists(I_Path.GetNativePath()); }
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
        CreateDirectory(const FPath& I_RelativePath) const;
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
    CreateDirectory(const FPath& I_RelativePath) const
    {
        const auto Path = GetRoot()/I_RelativePath;
        std::error_code ErrorCode{};

        if (!std::filesystem::exists(Path.GetNativePath(), ErrorCode))
        {
            if (!std::filesystem::create_directories(Path.GetNativePath(), ErrorCode))
            { LOG_ERROR("Failed to create directory at \"{}\" -- {}.", Path, ErrorCode.message()); }
            else
            { LOG_INFO("Created a new directory at \"{}\"", Path); return True; }
        }
        else { LOG_ERROR("Failed to create the directory \"{}\" -- {}.", Path, ErrorCode.message()); }

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

}

template <>
struct fmt::formatter<Visera::FFileSystem>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::FFileSystem& I_FileSystem, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        return fmt::format_to(
            I_Context.out(),
            "Root: {}",
            I_FileSystem.GetRoot()
        );
    }
};