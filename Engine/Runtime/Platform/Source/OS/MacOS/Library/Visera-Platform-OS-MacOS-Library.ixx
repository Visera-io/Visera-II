module;
#include <Visera-Platform.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <dlfcn.h>
#endif
export module Visera.Platform.OS.MacOS.Library;
#define VISERA_MODULE_NAME "Platform.OS"
import Visera.Platform.OS.Interface.Library;
import Visera.Global.Log;

namespace Visera
{
#if defined(VISERA_ON_APPLE_SYSTEM)
    export class VISERA_PLATFORM_API FMacOSLibrary : public ILibrary
    {
    public:
        [[nodiscard]] void*
        LoadFunction(const char* I_Name) const override;

        FMacOSLibrary() = delete;
        FMacOSLibrary(const FPath& I_Path);
        ~FMacOSLibrary() override;
    };

    void* FMacOSLibrary::
    LoadFunction(const char* I_Name) const
    {
        void* Function{nullptr};
        if (IsLoaded())
        {
            Function = dlsym(Handle, I_Name);
            if (const char* Error = dlerror())
            { LOG_ERROR("Failed to load the function {} -- {}", I_Name, Error); }
        }
        else { LOG_ERROR("Can NOT load the function {} from an unloaded library \"{}\"!", I_Name, Path); }

        return Function;
    }

    FMacOSLibrary::
    FMacOSLibrary(const FPath& I_Path)
    : ILibrary{I_Path}
    {
        LOG_TRACE("Loading MacOS library {}", Path.GetNativePath().c_str());

        Handle = dlopen(I_Path.GetNativePath().c_str(), RTLD_NOW | RTLD_GLOBAL);
        if (!Handle)
        {
            const char* Error = dlerror();
            LOG_ERROR("Failed to load the library \"{}\" -- {}!", Path, Error);
        }
    }

    FMacOSLibrary::
    ~FMacOSLibrary()
    {
        if (IsLoaded())
        {
            LOG_TRACE("Unloading MacOS library: {}", Path);
            dlclose(Handle);
        }
    }
#endif
}