module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Path;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Types.Text;

export namespace Visera
{
    class VISERA_CORE_API FPath
    {
    public:
        [[nodiscard]] inline const auto&
        GetNativePath() const { return Data; }

        [[nodiscard]] FPath
        operator/(const FPath& I_Path) const {  return Merge(*this, I_Path); }

        FPath() = default;
        FPath(const FPath& I_Path)     : Data{ I_Path.Data } {}
        FPath(FPath&& I_Path) noexcept : Data{ I_Path.Data } {}
        explicit FPath(const FText& I_Path)     : Data{ I_Path.GetData() } {}

    private:
        std::filesystem::path Data;

    private:
        [[nodiscard]] friend FPath
        Merge(const FPath& I_PathA, const FPath& I_PathB)
        { FPath NewPath{}; NewPath.Data = I_PathA.Data / I_PathB.Data; return NewPath; }
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
        return fmt::format_to(
            I_Context.out(),
            "{}",
            I_Path.GetNativePath().c_str()
        );
    }
};