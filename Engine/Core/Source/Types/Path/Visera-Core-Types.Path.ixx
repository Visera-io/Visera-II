module;
#include <Visera-Core.hpp>
#include <filesystem>
export module Visera.Core.Types.Path;
#define VISERA_MODULE_NAME "Core.Types"
export import Visera.Core.Types.Text;

export namespace Visera
{
    class VISERA_CORE_API FPath
    {
    public:
        [[nodiscard]] static inline FPath
        CurrentPath() { return FPath{std::filesystem::current_path().u8string()}; }

    public:
        [[nodiscard]] inline Bool
        IsEmpty() const { return Data.empty(); }
        [[nodiscard]] inline FPath
        GetFileName() const { return FPath{Data.filename().u8string()}; }
        [[nodiscard]] inline FPath
        GetParent() const { return FPath{Data.parent_path().u8string()}; }
        [[nodiscard]] inline FPath
        GetExtension() const { return Data.has_extension()? FPath{Data.extension().u8string()} : FPath{}; }

        [[nodiscard]] inline auto
        GetUTF8Path() const { return FString{reinterpret_cast<const char*>(Data.u8string().c_str())}; }
        [[nodiscard]] inline const auto&
        GetNativePath() const { return Data; }

        [[nodiscard]] FPath
        operator/(const FPath& I_Path) const {  return Merge(*this, I_Path); }
        [[nodiscard]] Bool
        operator==(const FPath& I_Path) const {  return Data == I_Path.Data; }

        FPath() = default;
        FPath(const FPath& I_Path)      : Data{ I_Path.Data } { Data.make_preferred(); }
        FPath(FPath&& I_Path) noexcept  : Data{ I_Path.Data } { Data.make_preferred(); }
        FPath& operator=(const FPath& I_Path)     { Data = I_Path.Data; return *this; }
        FPath& operator=(FPath&& I_Path) noexcept { Data = I_Path.Data; return *this; }
        FPath(const FText& I_Path): Data{ I_Path.GetData() } { Data.make_preferred(); }
        FPath(FStringView     I_Path): Data{ I_Path } { Data.make_preferred(); }
        FPath(FUTF8StringView I_Path): Data{ I_Path } { Data.make_preferred(); }
        FPath(FWideStringView I_Path): Data{ I_Path } { Data.make_preferred(); }
        template <size_t N>
        FPath(const char (&lit)[N]) : Data(FStringView(lit)) { Data.make_preferred(); }

    private:
        std::filesystem::path Data;

    private:
        [[nodiscard]] friend FPath
        Merge(const FPath& I_PathA, const FPath& I_PathB)
        { FPath NewPath{}; NewPath.Data = I_PathA.Data / I_PathB.Data; return NewPath; }
    };
}
VISERA_MAKE_HASH(Visera::FPath, return std::hash<std::filesystem::path>{}(I_Object.GetNativePath()););
VISERA_MAKE_FORMATTER(Visera::FPath, {}, "{}", I_Formatee.GetUTF8Path());