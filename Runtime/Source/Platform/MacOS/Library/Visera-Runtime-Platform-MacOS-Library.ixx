module;
#include <Visera-Runtime.hpp>
#include <dlfcn.h>
export module Visera.Runtime.Platform.MacOS.Library;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Platform.Interface.Library;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FMacOSLibrary : public ILibrary
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
        else LOG_ERROR("Can NOT load the function {} from an unloaded library \"{}\"!", I_Name, Path);

        return Function;
    }

    FMacOSLibrary::
    FMacOSLibrary(const FPath& I_Path)
    : ILibrary{I_Path}
    {
        Handle = dlopen(I_Path.GetNativePath().c_str(), RTLD_LAZY);
        if (!Handle)
        {
            LOG_ERROR("Failed to load the library \"{}\"", I_Path);
        }
    }

    FMacOSLibrary::
    ~FMacOSLibrary()
    {
        if (IsLoaded())
        {
            dlclose(Handle);
        }
    }
}