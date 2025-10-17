module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Text;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    /* UTF8 Encoded String (Compiler & Project Level Setted) */
    class VISERA_CORE_API FText
    {
    public:
        template <typename T> [[nodiscard]] static inline FText
        ToUTF8(const T* I_Text) { return FText{I_Text}; }

        [[nodiscard]] const FString&
        GetData() const { return Data; }

        //auto ToString() const -> StringView { return Data; }
        explicit operator FString()		const	{ return Data; }
        explicit operator const char*()	const	{ return Data.data(); }
        explicit FText(FStringView    I_Data) : Data{I_Data} {}
        explicit FText(const char8_t* I_Text) : Data{ reinterpret_cast<const char *>(I_Text) } {}
        explicit FText(FWideStringView I_Text);
        explicit FText(FUTF8StringView I_Text) : Data{ reinterpret_cast<const char *>(I_Text.data()) } {}

    private:
        FString Data;
    };

    FText::
    FText(FWideStringView I_Text)
    {
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            int sizeNeeded = WideCharToMultiByte(
                CP_UTF8,
                0,
                I_Text.data(),
                -1,
                nullptr,
                0,
                nullptr,
                nullptr);
            if (sizeNeeded <= 0) { return; }

            Data.resize(sizeNeeded - 1, 0); // -1 to exclude null terminator
            WideCharToMultiByte(
                CP_UTF8,
                0,
                I_Text.data(),
                -1,
                Data.data(),
                sizeNeeded,
                nullptr,
                nullptr);
#elif defined(VISERA_ON_APPLE_SYSTEM)
        VISERA_UNIMPLEMENTED_API;
            // size_t Size = wcstombs(nullptr, _Source.data(), 0);
            // if (Size == static_cast<size_t>(-1)) { return {}; }
            //
            // String Sink(Size, 0);
            // wcstombs(Sink.data(), _Source.data(), Size);
#else
        VISERA_UNIMPLEMENTED_API;
#endif
    }
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