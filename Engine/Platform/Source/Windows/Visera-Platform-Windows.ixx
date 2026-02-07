module;
#include <windows.h>
#undef LoadLibrary
#undef CreateDirectory
#undef CreateWindow
#undef SetEnvironmentVariable
#include <Visera-Platform.hpp>
export module Visera.Platform.Windows;
#define VISERA_MODULE_NAME "Platform.Windows"
import Visera.Platform.Interface;
import Visera.Platform.Windows.Window;
import Visera.Platform.Windows.Library;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;

namespace Visera
{
    export class VISERA_PLATFORM_API FWindowsPath : public IPlatformPath
    {
    public:
        explicit FWindowsPath(const FPath& I_Path);
        explicit FWindowsPath(std::wstring_view I_Native) : Native(I_Native) {}
        [[nodiscard]] operator std::wstring_view() const noexcept { return Native; }
        [[nodiscard]] FPath ToPath() const override;

    private:
        std::wstring Native;
    };

    FWindowsPath::FWindowsPath(const FPath& I_Path)
    {
        VISERA_ASSERT(I_Path.IsNormalized());
        const std::string_view Utf8 = I_Path.GetString().GetNative();
        if (Utf8.empty()) return;
        const int WideLength = MultiByteToWideChar(CP_UTF8, 0, Utf8.data(), static_cast<int>(Utf8.size()), nullptr, 0);
        if (WideLength <= 0) return;
        Native.resize(static_cast<size_t>(WideLength));
        MultiByteToWideChar(CP_UTF8, 0, Utf8.data(), static_cast<int>(Utf8.size()), Native.data(), WideLength);
        for (wchar_t& Ch : Native)
            if (Ch == L'/') Ch = L'\\';
    }

    FPath FWindowsPath::ToPath() const
    {
        const std::wstring_view Wide = *this;
        if (Wide.empty()) return FPath(FString());
        const int Utf8Length = WideCharToMultiByte(CP_UTF8, 0, Wide.data(), static_cast<int>(Wide.size()), nullptr, 0, nullptr, nullptr);
        if (Utf8Length <= 0) return FPath(FString());
        std::string Utf8(static_cast<size_t>(Utf8Length), '\0');
        WideCharToMultiByte(CP_UTF8, 0, Wide.data(), static_cast<int>(Wide.size()), Utf8.data(), Utf8Length, nullptr, nullptr);
        for (char& Ch : Utf8)
            if (Ch == '\\') Ch = '/';
        FPath Path(FString(std::move(Utf8)));
        Path.Normalize();
        return Path;
    }

    export class VISERA_PLATFORM_API FWindowsPlatform : public IPlatform
    {
    public:
        [[nodiscard]] TUniquePtr<IPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const override;
        [[nodiscard]] TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath& I_Path) const override { return MakeShared<FWindowsLibrary>(static_cast<const FWindowsPath&>(I_Path).ToPath()); }
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetResourceDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const override;
        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const override;
        [[nodiscard]] FUUID
        GenerateUUID() const override;

    public:
        FWindowsPlatform();
        ~FWindowsPlatform() override = default;
    };

    FWindowsPlatform::FWindowsPlatform()
    : IPlatform{EPlatform::Windows}
    {
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);
    }

    TUniquePtr<IPlatformPath> FWindowsPlatform::GetExecutableDirectory() const
    {
        std::wstring Buffer(MAX_PATH, L'\0');
        DWORD Size = GetModuleFileNameW(nullptr, Buffer.data(), static_cast<DWORD>(Buffer.size()));
        Buffer.resize(Size);
        if (Size == 0) return nullptr;
        const std::wstring_view View(Buffer);
        const size_t LastSlash = View.find_last_of(L"\\/");
        if (LastSlash == std::wstring_view::npos) return MakeUnique<FWindowsPath>(View);
        return MakeUnique<FWindowsPath>(View.substr(0, LastSlash));
    }

    TUniquePtr<IPlatformPath> FWindowsPlatform::GetResourceDirectory() const
    {
        return GetExecutableDirectory();
    }

    TUniquePtr<IPlatformPath> FWindowsPlatform::GetFrameworkDirectory() const
    {
        return GetExecutableDirectory();
    }

    TUniquePtr<IPlatformWindow> FWindowsPlatform::
    CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height) const
    {
        return MakeUnique<FWindowsWindow>(I_Title, I_Width, I_Height);
    }

    Bool FWindowsPlatform::
    SetEnvironmentVariable(FStringView I_Variable,
                           FStringView I_Value) const
    {
        return SetEnvironmentVariableA(I_Variable.Data(), I_Value.Data());
    }

    /**
     * Generates a UUID using Windows OS API.
     *
     * Notes:
     * - Windows GUID binary layout is NOT the same as RFC 4122 canonical octet sequence.
     *   Data1/Data2/Data3 are stored as little-endian integers in the GUID struct, while
     *   the canonical UUID byte sequence (and the common text form) orders bytes as
     *   {time_low, time_mid, time_hi_and_version, clock_seq, node}.
     *
     * Ref: MS-DTYP GUID packet representation.
     */
    FUUID FWindowsPlatform::
    GenerateUUID() const
    {
        GUID Buffer{};
        const HRESULT HResult = ::CoCreateGuid(&Buffer);
        VISERA_ASSERT(SUCCEEDED(HResult));

        FUUID UUID;
        // time_low (Data1)
        UUID.Data[0] = static_cast<FByte>((Buffer.Data1 >> 24) & 0xFFu);
        UUID.Data[1] = static_cast<FByte>((Buffer.Data1 >> 16) & 0xFFu);
        UUID.Data[2] = static_cast<FByte>((Buffer.Data1 >>  8) & 0xFFu);
        UUID.Data[3] = static_cast<FByte>((Buffer.Data1 >>  0) & 0xFFu);

        // time_mid (Data2)
        UUID.Data[4] = static_cast<FByte>((Buffer.Data2 >> 8) & 0xFFu);
        UUID.Data[5] = static_cast<FByte>((Buffer.Data2 >> 0) & 0xFFu);

        // time_hi_and_version (Data3)
        UUID.Data[6] = static_cast<FByte>((Buffer.Data3 >> 8) & 0xFFu);
        UUID.Data[7] = static_cast<FByte>((Buffer.Data3 >> 0) & 0xFFu);

        // clock_seq + node (Data4)
        UUID.Data[8]  = static_cast<FByte>(Buffer.Data4[0]);
        UUID.Data[9]  = static_cast<FByte>(Buffer.Data4[1]);
        UUID.Data[10] = static_cast<FByte>(Buffer.Data4[2]);
        UUID.Data[11] = static_cast<FByte>(Buffer.Data4[3]);
        UUID.Data[12] = static_cast<FByte>(Buffer.Data4[4]);
        UUID.Data[13] = static_cast<FByte>(Buffer.Data4[5]);
        UUID.Data[14] = static_cast<FByte>(Buffer.Data4[6]);
        UUID.Data[15] = static_cast<FByte>(Buffer.Data4[7]);

        return UUID;
    }
}