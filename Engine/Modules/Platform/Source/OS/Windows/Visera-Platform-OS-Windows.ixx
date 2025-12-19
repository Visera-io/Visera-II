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
import Visera.Runtime.Log;

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
#endif
}