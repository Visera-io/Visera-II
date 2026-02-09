module;
#include <Visera-Platform.hpp>
#include <dlfcn.h>
export module Visera.Platform.MacOS.Library;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.Interface.Library;
import Visera.Core.Types.Path;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_PLATFORM_API FMacOSLibrary : public IPlatformLibrary
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
        else { LOG_ERROR("Can NOT load the function {} from an unloaded library {}!", I_Name, Path); }

        return Function;
    }

    FMacOSLibrary::
    FMacOSLibrary(const FPath& I_Path)
    : IPlatformLibrary{I_Path}
    {
        LOG_TRACE("Loading MacOS library {}", I_Path.GetNative().c_str());

        Handle = dlopen(I_Path.GetString().Data(), RTLD_NOW | RTLD_GLOBAL);
        if (!Handle)
        {
            const char* Error = dlerror();
            LOG_ERROR("Failed to load the library {} -- {}!", I_Path, Error);
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
}