module;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <windows.h>
#endif
#include <Visera-Core.hpp>
export module Visera.Core.Types.Text;
#define VISERA_MODULE_NAME "Core.Types"

export namespace Visera
{
    /* UTF8 Encoded String */
    class VISERA_CORE_API FText
    {
    public:
        template <typename T> [[nodiscard]] static inline FText
        ToUTF8(const T* I_Text) { return FText{I_Text}; }

        [[nodiscard]] const FString&
        GetString() const { return String; }
        [[nodiscard]] const char*
        GetData()   const { return String.data(); }

        //auto ToString() const -> StringView { return String; }
        explicit operator FString()		const	{ return String; }
        explicit operator const char*()	const	{ return String.data(); }
        explicit FText(FStringView    I_String) : String{I_String} {}
        explicit FText(const char8_t* I_Text) : String{ reinterpret_cast<const char *>(I_Text) } {}
        explicit FText(FWideStringView I_Text);
        explicit FText(FUTF8StringView I_Text) : String{ reinterpret_cast<const char *>(I_Text.data()) } {}

    private:
        FString String;
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

        String.resize(sizeNeeded - 1, 0); // -1 to exclude null terminator
        WideCharToMultiByte(
            CP_UTF8,
            0,
            I_Text.data(),
            -1,
            String.data(),
            sizeNeeded,
            nullptr,
            nullptr);
#elif defined(VISERA_ON_APPLE_SYSTEM)
        UInt64 Size = wcstombs(nullptr, I_Text.data(), 0);
        if (Size == static_cast<UInt64>(-1)) { return; }

        String.resize(Size);
        wcstombs(String.data(), I_Text.data(), Size);
#else
        VISERA_UNIMPLEMENTED_API;
#endif
    }
}
VISERA_MAKE_FORMATTER(Visera::FText, {}, "{}", static_cast<const char*>(I_Formatee.GetData()))