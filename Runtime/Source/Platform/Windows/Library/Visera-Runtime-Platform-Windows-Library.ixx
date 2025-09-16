module;
#include <Visera-Runtime.hpp>
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <windows.h>
#endif
export module Visera.Runtime.Platform.Windows.Library;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Platform.Interface.Library;
import Visera.Core.Log;

namespace Visera
{
#if defined(VISERA_ON_WINDOWS_SYSTEM)
    export class VISERA_RUNTIME_API FWindowsLibrary : public ILibrary
    {
    public:
        [[nodiscard]] void*
        LoadFunction(const char* I_Name) const override;

        FWindowsLibrary() = delete;
        FWindowsLibrary(const FPath& I_Path);
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
                LOG_ERROR("Failed to load the function '{}' from library \"{}\" -- Windows Error Code: {}", I_Name, Path, Error);
            }
        }
        else 
        {
            LOG_ERROR("Cannot load function '{}' from an unloaded library \"{}\"!", I_Name, Path);
        }

        return Function;
    }

    FWindowsLibrary::
    FWindowsLibrary(const FPath& I_Path)
    : ILibrary{I_Path}
    {
        LOG_TRACE("Loading Windows library: {}", I_Path);
        
        // Get the native path string
        const FString NativePath = reinterpret_cast<const char*>(I_Path.GetNativePath().u8string().c_str());
        
        // Convert path to wide string for Windows API
        int WideLength = MultiByteToWideChar(CP_UTF8, 0, NativePath.c_str(), -1, nullptr, 0);
        if (WideLength <= 0)
        {
            DWORD Error = GetLastError();
            LOG_ERROR("Failed to convert path to wide string for library \"{}\" -- Windows Error Code: {}", I_Path, Error);
            return;
        }
        
        std::vector<wchar_t> WidePath(WideLength);
        int ConvertResult = MultiByteToWideChar(CP_UTF8, 0, NativePath.c_str(), -1, WidePath.data(), WideLength);
        if (ConvertResult == 0)
        {
            DWORD Error = GetLastError();
            LOG_ERROR("Failed to convert path to wide string for library \"{}\" -- Windows Error Code: {}", I_Path, Error);
            return;
        }
        
        // Load the library using Windows API with security flags
        // LOAD_LIBRARY_SEARCH_DEFAULT_DIRS provides security by limiting search paths
        Handle = LoadLibraryExW(WidePath.data(), nullptr, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        
        if (!Handle)
        {
            DWORD Error = GetLastError();
            // Provide more helpful error messages for common error codes
            switch (Error)
            {
                case ERROR_MOD_NOT_FOUND:
                    LOG_ERROR("The specified module could not be found: {}", I_Path);
                    break;
                case ERROR_DLL_INIT_FAILED:
                    LOG_ERROR("DLL initialization failed for: {}", I_Path);
                    break;
                case ERROR_BAD_EXE_FORMAT:
                    LOG_ERROR("The library is not a valid executable format: {}", I_Path);
                    break;
                default:
                    LOG_ERROR("Unknown error occurred while loading library: {}", I_Path);
                    break;
            }
        }
    }

    FWindowsLibrary::
    ~FWindowsLibrary()
    {
        if (IsLoaded())
        {
            LOG_TRACE("Unloading Windows library: {}", Path);
            
            BOOL Result = FreeLibrary(static_cast<HMODULE>(Handle));
            if (!Result)
            {
                DWORD Error = GetLastError();
                LOG_ERROR("Failed to free library \"{}\" -- Windows Error Code: {}", Path, Error);
            }
        }
    }
#endif
}