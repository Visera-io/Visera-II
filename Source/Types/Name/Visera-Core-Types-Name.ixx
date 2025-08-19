module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Name;
#define VISERA_MODULE_NAME "Core.Types"
import Visera.Core.Name.NamePool;

export namespace Visera
{
    using EName = EPreservedName;

    /* Case-Ignored FString */
    class FName
    {
    public:
        static inline auto
        FetchNameString(EName I_Name) 		 -> FStringView { return FNamePool::GetInstance().FetchNameString(I_Name); }
        static inline auto
        FetchNameString(const FName& I_Name)  -> FStringView { return FNamePool::GetInstance().FetchNameString(I_Name.Handle); }

        auto GetName()	         const -> FStringView { return FNamePool::GetInstance().FetchNameString(Handle); }
        auto GetHandle() const -> UInt32 { return Handle; }
        auto GetNumber() const -> UInt32 { return Number; }
        auto GetIdentifier() const -> UInt64 { return (UInt64(Handle) << 32) | Number; }
        auto IsNone()	 const -> Bool	 { return !Handle && !Number; } //[FIXME]: Pre-Register EName::None in the Engine
        auto HasNumber() const -> Bool	 { return !!Number; }

        FName() = default;
        FName(FStringView I_Name)					{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(FString(I_Name)); Handle = Handle_; Number = Number_; }
        FName(EName I_PreservedName)				{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(I_PreservedName); Handle = Handle_; Number = Number_; }
        FName(const FName& I_Another)			    = default;
        FName(FName&& I_Another)					= default;
        FName& operator=(FStringView I_Name)		{ auto [Handle_, Number_] = FNamePool::GetInstance().Register(FString(I_Name)); Handle = Handle_; Number = Number_;  return *this; }
        FName& operator=(const FName& I_Another)    = default;
        FName& operator=(FName&& I_Another)		    = default;
        Bool operator==(const FName& I_Another) const { return GetIdentifier() == I_Another.GetIdentifier(); }
    private:
        UInt32		Handle;		//FNameEntryHandle
        UInt32		Number{ 0 };
    };

} // namespace VE

namespace std
{
    template<>
    struct hash<Visera::FName>
    {
        std::size_t operator()(const Visera::FName& I_Name) const noexcept
        { return static_cast<std::size_t>(I_Name.GetIdentifier()); }
    };
}

template <>
struct fmt::formatter<Visera::FName>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::FName& I_Name, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        return fmt::format_to(
            I_Context.out(),
            "{}({})",
            I_Name.GetName(), I_Name.GetNumber()
        );
    }
};