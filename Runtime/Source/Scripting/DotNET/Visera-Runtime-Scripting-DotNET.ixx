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

    export class VISERA_RUNTIME_API FDotNET
    {
    public:


    private:
        FPath          DotNETRoot    {DOTNET_ROOT};
        FPath          ConfigPath    {VISERA_RUNTIME_DIR "/Global/Visera-DotNETConfig.json"};
        hostfxr_handle Context       {nullptr};

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

            // Get the load assembly function pointer
            if (Fn_GetRuntimeDelegate(
                Context,
                hdt_get_function_pointer,
                reinterpret_cast<void**>(&Fn_GetAssemblyAndGetFunctionPointer)) != 0 ||
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
        inline void
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

            auto HostFXRInitInfo = hostfxr_initialize_parameters
            {
                .dotnet_root = DotNETRoot.GetNativePath().c_str(),
            };
            if (Fn_InitializeRuntime(
                ConfigPath.GetNativePath().c_str(),
                &HostFXRInitInfo,
                &Context) != 0 || !Context)
            { LOG_FATAL("Failed to initialize HostFXR runtime!"); }
        }
    };

}