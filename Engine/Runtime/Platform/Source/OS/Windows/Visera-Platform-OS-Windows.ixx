module;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <windows.h>
#undef LoadLibrary
#undef CreateDirectory
#undef SetEnvironmentVariable
#endif
#include <Visera-Platform.hpp>
export module Visera.Platform.OS.Windows;
#define VISERA_MODULE_NAME "Platform.OS"
import Visera.Platform.OS.Interface;
import Visera.Platform.OS.Windows.Library;
import Visera.Global.Log;

namespace Visera
{
#if defined(VISERA_ON_WINDOWS_SYSTEM)
    export class VISERA_PLATFORM_API FWindowsPlatform : public IOS
    {
    public:
        [[nodiscard]] TSharedPtr<ILibrary>
        LoadLibrary(const FPath& I_Path) const override { return MakeShared<FWindowsLibrary>(I_Path); }
        [[nodiscard]] const FPath&
        GetExecutableDirectory() const override { return ExecutableDirectory; }
        [[nodiscard]] const FPath&
        GetResourceDirectory() const override { return ExecutableDirectory; }
        [[nodiscard]] const FPath&
        GetFrameworkDirectory() const override { return ExecutableDirectory; }
        [[nodiscard]] const FPath&
        GetCacheDirectory() const override { return CacheDirectory; }
        [[nodiscard]] Bool
        SetEnvironmentVariable(FStringView I_Variable, FStringView I_Value) const override;
        [[nodiscard]] FUUID
        GenerateUUID() const override;

    private:
        FPath ExecutableDirectory;
        FPath CacheDirectory;

    public:
        FWindowsPlatform();
        ~FWindowsPlatform() override = default;
    };

    FWindowsPlatform::
    FWindowsPlatform()
    : IOS{EPlatform::Windows}
    {
        SetConsoleOutputCP(65001); // Set console output code page to UTF-8
        SetConsoleCP(65001);       // Also set input code page to UTF-8 for consistency

        std::wstring Buffer(MAX_PATH, L'\0');
        DWORD Size = GetModuleFileNameW(nullptr, Buffer.data(), static_cast<DWORD>(Buffer.size()));
        Buffer.resize(Size);

        ExecutableDirectory = FPath{Buffer}.GetParent();

        CacheDirectory = ExecutableDirectory / FPath{"Cache"};
    }

    Bool FWindowsPlatform::
    SetEnvironmentVariable(FStringView I_Variable,
                           FStringView I_Value) const
    {
        if (!SetEnvironmentVariableA(I_Variable.data(), I_Value.data()))
        {
            LOG_ERROR("Failed to set environment variable \"{}\" as \"{}\"!",
                      I_Variable, I_Value);
            return False;
        }
        LOG_DEBUG("Set environment variable \"{}\" as \"{}\".",
                  I_Variable, I_Value);
        return True;
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
#endif
}