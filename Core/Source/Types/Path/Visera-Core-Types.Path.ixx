module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Path;
#define VISERA_MODULE_NAME "Core.Types"
export import Visera.Core.Types.Text;
       import Visera.Core.Hash;

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
        FPath(std::u8string_view I_Path): Data{ I_Path }     { Data.make_preferred(); }

    private:
        std::filesystem::path Data;

    private:
        [[nodiscard]] friend FPath
        Merge(const FPath& I_PathA, const FPath& I_PathB)
        { FPath NewPath{}; NewPath.Data = I_PathA.Data / I_PathB.Data; return NewPath; }
    };
}

namespace std
{
    template<>
    struct hash<Visera::FPath>
    {
        std::size_t operator()(const Visera::FPath& I_Path) const noexcept
        {  return std::hash<std::filesystem::path>{}(I_Path.GetNativePath()); }
    };
}

template <>
struct fmt::formatter<Visera::FPath>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::FPath& I_Path, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        Visera::FText Path{I_Path.GetNativePath().u8string()};
        return fmt::format_to(
            I_Context.out(),
            "{}",
            Path
        );
    }
};