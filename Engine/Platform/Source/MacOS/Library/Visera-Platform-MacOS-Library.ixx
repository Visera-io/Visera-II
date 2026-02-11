module;
#include <Visera-Platform.hpp>
#if defined(VISERA_ON_APPLE_SYSTEM)
#include <dlfcn.h>
#endif
export module Visera.Platform.MacOS.Library;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Interface.Library;
import Visera.Platform.MacOS.Path;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Log;

export namespace Visera
{
    class VISERA_PLATFORM_API FMacOSLibrary : public IPlatformLibrary
    {
    public:
        [[nodiscard]] void*
        LoadFunction(const char* I_Name) const override;

        FMacOSLibrary() = delete;
        explicit FMacOSLibrary(const IPlatformPath& I_Path);
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
        else { LOG_ERROR("Can NOT load the function {} from an unloaded library {}!", I_Name, GetPath()); }

        return Function;
    }

    FMacOSLibrary::
    FMacOSLibrary(const IPlatformPath& I_Path)
    : IPlatformLibrary{I_Path}
    {
        const FMacOSPath& NativePath = static_cast<const FMacOSPath&>(I_Path);
        const FString PathStr(NativePath.ToPath().GetString());
        LOG_TRACE("Loading MacOS library {}", GetPath());

        Handle = dlopen(PathStr.Data(), RTLD_NOW | RTLD_GLOBAL);
        if (!Handle)
        {
            const char* Error = dlerror();
            LOG_ERROR("Failed to load the library {} -- {}!", GetPath(), Error);
        }
    }

    FMacOSLibrary::
    ~FMacOSLibrary()
    {
        if (IsLoaded())
        {
            LOG_TRACE("Unloading MacOS library: {}", GetPath());
            dlclose(Handle);
        }
    }
}