module;
#include <Visera-Runtime.hpp>
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>
export module Visera.Runtime.Scripting.DotNET;
#define VISERA_MODULE_NAME "Runtime.Scripting"
import Visera.Core.Log;
import Visera.Core.Types.Path;
import Visera.Runtime.Platform;

namespace Visera
{

    export class FDotNET
    {
    public:


    private:
        const char*    ConfigPath = "Visera.DotNETConfig.json";
        hostfxr_handle Context      {nullptr};

        TSharedPtr<ILibrary> HostFXR;
        hostfxr_initialize_for_runtime_config_fn  Fn_InitializeRuntime{nullptr};
        hostfxr_get_runtime_delegate_fn           Fn_GetRuntimeDelegate{nullptr};
        load_assembly_and_get_function_pointer_fn Fn_GetAssemblyAndGetFunctionPointer{nullptr};
        hostfxr_close_fn                          Fn_FinalizeRuntime{nullptr};

    public:
        FDotNET()
        {
            LOG_DEBUG("Initializing .NET");
            LoadHostFXR();

            if (!Fn_InitializeRuntime(ConfigPath, nullptr, &Context) || !Context)
            { LOG_FATAL("Failed to initialize HostFXR runtime!"); }

            // Get the load assembly function pointer
            if (!Fn_GetRuntimeDelegate(
                Context,
                hdt_get_function_pointer,
                reinterpret_cast<void**>(&Fn_GetAssemblyAndGetFunctionPointer)) ||
                !Fn_GetAssemblyAndGetFunctionPointer)
            { LOG_FATAL("Failed to get delegate!"); }
        }
        ~FDotNET()
        {
            LOG_DEBUG("Terminating .NET");

            if (!Fn_FinalizeRuntime(nullptr))
            { LOG_ERROR("Failed to finalize HostFXR runtime"); }
        }

    private:
        [[nodiscard]] inline void
        LoadHostFXR()
        {
            LOG_TRACE("Loading HostFXR");
            // Using the nethost library, discover the location of hostfxr and get exports
            HostFXR = GPlatform->LoadLibrary(FPath{HOSTFXR_LIBRARY_NAME});
            if (!HostFXR) { LOG_FATAL("Failed to load HostFXR"); }

            Fn_InitializeRuntime = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>
                (HostFXR->LoadFunction("hostfxr_initialize_for_runtime_config"));
            Fn_GetRuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>
                (HostFXR->LoadFunction("hostfxr_get_runtime_delegate"));
            Fn_FinalizeRuntime = reinterpret_cast<hostfxr_close_fn>
                (HostFXR->LoadFunction("hostfxr_close"));
        }
    };

}