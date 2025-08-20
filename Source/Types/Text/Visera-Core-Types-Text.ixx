module;
#include <Visera-Core.hpp>
#undef TEXT
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <windows.h>
#endif
export module Visera.Core.Types.Text;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    /* UTF8 Encoded String (Compiler & Project Level Setted) */
    class FText
    {
    public:
        // static inline auto
        // ToUTF8(FWideStringView I_String)    -> FString;

        //auto ToString() const -> StringView { return Data; }
        explicit operator FString()		const	{ return Data; }
        explicit operator const char*()	const	{ return Data.data(); }
        explicit FText(const char8_t* I_Text) : Data{ reinterpret_cast<const char *>(I_Text) } {}
    private:
        FString Data;
    };

    // Convert std::wstring (UTF-16) to std::string (UTF-8)
//     String FText::
//     ToUTF8(WideStringView _Source)
//     {
//         if (_Source.empty()) { return {}; }
// #if (VE_IS_WINDOWS_SYSTEM)
//         int sizeNeeded = WideCharToMultiByte(
//             CP_UTF8,
//             0,
//             _Source.data(),
//             -1,
//             nullptr,
//             0,
//             nullptr,
//             nullptr);
//         if (sizeNeeded <= 0) { return {}; }
//
//         String Sink(sizeNeeded - 1, 0); // -1 to exclude null terminator
//         WideCharToMultiByte(
//             CP_UTF8,
//             0,
//             _Source.data(),
//             -1,
//             Sink.data(),
//             sizeNeeded,
//             nullptr,
//             nullptr);
// #elif (VE_IS_APPLE_SYSTEM)
//         size_t Size = wcstombs(nullptr, _Source.data(), 0);
//         if (Size == static_cast<size_t>(-1)) { return {}; }
//
//         String Sink(Size, 0);
//         wcstombs(Sink.data(), _Source.data(), Size);
// #endif
//         return Sink;
//     }
}

template <>
struct fmt::formatter<Visera::FText>
{
    // Parse format specifiers (if any)
    constexpr auto parse(format_parse_context& I_Context) -> decltype(I_Context.begin())
    {
        return I_Context.begin();  // No custom formatting yet
    }

    // Corrected format function with const-correctness
    template <typename FormatContext>
    auto format(const Visera::FText& I_Text, FormatContext& I_Context) const
    -> decltype(I_Context.out())
    {
        return fmt::format_to(
            I_Context.out(),
            "{}",
            static_cast<const char*>(I_Text)
        );
    }
};