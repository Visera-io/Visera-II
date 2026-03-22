module;
#include <objbase.h>
#include <processthreadsapi.h>
#include <windows.h>
#undef LoadLibrary
#undef CreateDirectory
#undef CreateWindow
#undef SetEnvironmentVariable
#undef GetEnvironmentVariable
#include <Visera-Platform.hpp>
export module Visera.Platform.Windows;
#define VISERA_MODULE_NAME "Platform.Windows"
export import Visera.Platform.Interface;
export import Visera.Platform.Windows.Path;
export import Visera.Platform.Windows.Device;
export import Visera.Platform.Windows.Window;
export import Visera.Platform.Windows.Library;
export import Visera.Platform.Windows.FileSystem;
       import Visera.Platform.GLFW;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Text;
       import Visera.Core.Log;

export namespace Visera
{
    class VISERA_PLATFORM_API FWindowsPlatform : public IPlatform
    {
    public:
        [[nodiscard]] TUniquePtr<IPlatformWindow>
        CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height, Bool I_Resizable, Bool I_Center, Bool I_Fullscreen) const override;
        [[nodiscard]] TSharedPtr<IPlatformLibrary>
        LoadLibrary(const IPlatformPath& I_Path) const override { return MakeShared<FWindowsLibrary>(I_Path); }
        [[nodiscard]] IPlatformFileSystem*
        GetFileSystem() const override { return &FileSystem; }
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetExecutableDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetResourceDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetFrameworkDirectory() const override;
        [[nodiscard]] TUniquePtr<IPlatformPath>
        GetUserDataDirectory() const override;
        [[nodiscard]] FPath
        GetLogsDirectory() const override;
        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const override;
        [[nodiscard]] TOptional<FString>
        GetEnvironmentVariable(FStringView I_Variable) const override;
        [[nodiscard]] FUUID
        GenerateUUID() const override;
        void
        SetCurrentThreadName(FStringView I_Name) const override;
        void
        PollEvents() const override { GLFW.PollEvents(); }
        void
        WaitEvents() const override { GLFW.WaitEvents(); }
        [[nodiscard]] IPlatformWindow*
        GetFocusedWindow() const override { return GLFW.GetFocusedWindow(); }
        [[nodiscard]] FStringView
        GetPlatformName() const override { return "Windows"; }

    public:
        FWindowsPlatform();
        ~FWindowsPlatform() override = default;

    private:
        static std::wstring MakePlatformString(FStringView I_Text);

        mutable FWindowsPlatformFileSystem FileSystem;
        mutable FGLFWPlatform              GLFW;
    };

#ifndef VISERA_APP_NAME
#define VISERA_APP_NAME "Visera"
#endif

    FWindowsPlatform::FWindowsPlatform()
    {
        SetConsoleOutputCP(65001);
        SetConsoleCP(65001);

        /* Configure log file sink per platform convention: LocalAppData/AppName/Logs. */
        const FPath LogsDirectory = GetLogsDirectory();
        if (!LogsDirectory.IsEmpty())
        {
            (void)GetFileSystem()->CreateDirectories(FWindowsPath(LogsDirectory));
            FLog::SetSinkPath(LogsDirectory);
        }
    }

    FPath FWindowsPlatform::GetLogsDirectory() const
    {
        const TOptional<FString> LocalAppData = GetEnvironmentVariable("LOCALAPPDATA");
        if (!LocalAppData.HasValue() || LocalAppData.GetValue().IsEmpty()) { return FPath(); }
        const FPath Base(LocalAppData.GetValue());
        return Base / FPath(VISERA_APP_NAME) / FPath("Logs");
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

    TUniquePtr<IPlatformPath> FWindowsPlatform::GetUserDataDirectory() const
    {
        const TOptional<FString> LocalAppData = GetEnvironmentVariable("LOCALAPPDATA");
        if (!LocalAppData.HasValue() || LocalAppData.GetValue().IsEmpty()) { return nullptr; }
        const FPath Base(LocalAppData.GetValue());
        const FPath UserDataDirectory = Base / FPath(VISERA_APP_NAME);
        return MakeUnique<FWindowsPath>(UserDataDirectory);
    }

    std::wstring FWindowsPlatform::MakePlatformString(FStringView I_Text)
    {
        if (I_Text.IsEmpty()) { return {}; }
        const int WideLen = MultiByteToWideChar(CP_UTF8, 0, I_Text.Data(), static_cast<int>(I_Text.GetSize()), nullptr, 0);
        if (WideLen <= 0) { return {}; }
        std::wstring Out(static_cast<std::size_t>(WideLen), L'\0');
        MultiByteToWideChar(CP_UTF8, 0, I_Text.Data(), static_cast<int>(I_Text.GetSize()), Out.data(), WideLen);
        return Out;
    }

    /** Create window: FWindowsWindow (inherits FGLFWWindow) so Query*State use GetKeyboardState+VK mapping in this module. */
    TUniquePtr<IPlatformWindow> FWindowsPlatform::
    CreateWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height, Bool I_Resizable, Bool I_Center, Bool I_Fullscreen) const
    {
        return MakeUnique<FWindowsWindow>(I_Title, I_Width, I_Height, I_Resizable, I_Center, I_Fullscreen);
    }

    Bool FWindowsPlatform::
    SetEnvironmentVariable(FStringView I_Variable,
                           FStringView I_Value) const
    {
        return SetEnvironmentVariableA(I_Variable.Data(), I_Value.Data());
    }

    TOptional<FString> FWindowsPlatform::
    GetEnvironmentVariable(FStringView I_Variable) const
    {
        const std::wstring VarW = MakePlatformString(I_Variable);
        if (VarW.empty()) { return std::nullopt; }
        DWORD Size = GetEnvironmentVariableW(VarW.c_str(), nullptr, 0);
        if (Size == 0) { return std::nullopt; }
        std::wstring Buf(Size, L'\0');
        if (GetEnvironmentVariableW(VarW.c_str(), Buf.data(), Size) == 0) { return std::nullopt; }
        Buf.resize(Size - 1);
        const int Utf8Len = WideCharToMultiByte(CP_UTF8, 0, Buf.data(), static_cast<int>(Buf.size()), nullptr, 0, nullptr, nullptr);
        if (Utf8Len <= 0) { return std::nullopt; }
        FString Utf8(static_cast<FString::SizeType>(Utf8Len), '\0');
        WideCharToMultiByte(CP_UTF8, 0, Buf.data(), static_cast<int>(Buf.size()), Utf8.Data(), Utf8Len, nullptr, nullptr);
        return Utf8;
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

    void FWindowsPlatform::SetCurrentThreadName(FStringView I_Name) const
    {
        if (I_Name.IsEmpty()) { return; }
        std::wstring Wide = MakePlatformString(I_Name);
        if (Wide.empty()) { return; }
        if (auto Result = SetThreadDescription(GetCurrentThread(), Wide.c_str()); FAILED(Result))
        { LOG_WARN("SetThreadDescription failed: HRESULT=0x{:08X}", static_cast<UInt32>(Result)); }
    }
}