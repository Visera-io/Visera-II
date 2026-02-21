module;
#include <Visera-Platform.hpp>
#include <windows.h>
export module Visera.Platform.Windows.Library;
#define VISERA_MODULE_NAME "Platform.Windows"
import Visera.Platform.Interface.Library;
import Visera.Platform.Windows.Path;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_PLATFORM_API FWindowsLibrary : public IPlatformLibrary
    {
    public:
        [[nodiscard]] void*
        LoadFunction(const char* I_Name) const override;

        FWindowsLibrary() = delete;
        explicit FWindowsLibrary(const IPlatformPath& I_Path);
        ~FWindowsLibrary() override;
    };

    void* FWindowsLibrary::
    LoadFunction(const char* I_Name) const
    {
        void* Function{nullptr};
        if (IsLoaded())
        {
            Function = reinterpret_cast<void*>(GetProcAddress(static_cast<HMODULE>(Handle), I_Name));
            if (!Function)
            {
                DWORD Error = GetLastError();
                LOG_ERROR("Failed to load the function '{}' from library {} -- Windows Error Code: {}", I_Name, GetPath(), Error);
            }
        }
        else { LOG_ERROR("Cannot load function '{}' from an unloaded library {}!", I_Name, GetPath()); }

        return Function;
    }

    FWindowsLibrary::
    FWindowsLibrary(const IPlatformPath& I_Path)
    : IPlatformLibrary{I_Path}
    {
        const FWindowsPath& NativePath = static_cast<const FWindowsPath&>(I_Path);
        const std::wstring_view WidePath = NativePath;
        LOG_TRACE("Loading Windows library: {}", GetPath());

        Handle = LoadLibraryExW(WidePath.data(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);

        if (!Handle)
        {
            DWORD Error = GetLastError();
            switch (Error)
            {
                case ERROR_MOD_NOT_FOUND:
                    LOG_ERROR("The specified module could not be found: {}", GetPath());
                    break;
                case ERROR_DLL_INIT_FAILED:
                    LOG_ERROR("DLL initialization failed for: {}", GetPath());
                    break;
                case ERROR_BAD_EXE_FORMAT:
                    LOG_ERROR("The library is not a valid executable format: {}", GetPath());
                    break;
                default:
                    LOG_ERROR("Unknown error occurred while loading library: {}", GetPath());
                    break;
            }
        }
    }

    FWindowsLibrary::
    ~FWindowsLibrary()
    {
        if (IsLoaded())
        {
            LOG_TRACE("Unloading Windows library: {}", GetPath());
            BOOL Result = FreeLibrary(static_cast<HMODULE>(Handle));
            if (!Result)
            {
                DWORD Error = GetLastError();
                LOG_ERROR("Failed to free library {} -- Windows Error Code: {}", GetPath(), Error);
            }
        }
    }
}