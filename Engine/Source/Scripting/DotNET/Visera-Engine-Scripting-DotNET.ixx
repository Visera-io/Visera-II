module;
#include <Visera-Engine.hpp>
#include <nethost.h>
#include <hostfxr.h>
#include <coreclr_delegates.h>
export module Visera.Engine.Scripting.DotNET;
#define VISERA_MODULE_NAME "Engine.Scripting"
import Visera.Core.Log;
import Visera.Core.Types.Path;
import Visera.Runtime.Platform;

namespace Visera
{

    export class VISERA_ENGINE_API FDotNET
    {
    public:


    private:
        FPath          DotNETRoot    {DOTNET_ROOT};
        FPath          ConfigPath    {VISERA_ENGINE_DIR "/Global/Visera-DotNETConfig.json"};
        hostfxr_handle Context       {nullptr};

        TSharedPtr<ILibrary> HostFXR;
        hostfxr_initialize_for_runtime_config_fn  Fn_InitializeRuntime{nullptr};
        hostfxr_get_runtime_delegate_fn           Fn_GetRuntimeDelegate{nullptr};
        load_assembly_and_get_function_pointer_fn Fn_LoadAssemblyAndGetFunctionPointer{nullptr};
        component_entry_point_fn                  Fn_GetComponentEntryPoint{nullptr};
        hostfxr_close_fn                          Fn_FinalizeRuntime{nullptr};

    public:
        FDotNET()
        {
            LOG_DEBUG("Initializing .NET");

            LoadHostFXR();
        }
        ~FDotNET()
        {
            LOG_DEBUG("Terminating .NET");

            if (!Fn_FinalizeRuntime(Context))
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

            // Get the load assembly function pointer
            if (Fn_GetRuntimeDelegate(
                Context,
                hdt_get_function_pointer,
                reinterpret_cast<void**>(&Fn_LoadAssemblyAndGetFunctionPointer)) != 0 ||
                !Fn_LoadAssemblyAndGetFunctionPointer)
            { LOG_FATAL("Failed to get delegate!"); }

            // Function pointer to managed delegate
            component_entry_point_fn hello = nullptr;
            int rc = Fn_LoadAssemblyAndGetFunctionPointer(
                L"D:/Programs/ViseraEngine/Visera-II/Apps/AlohaVisera/Assets/Script/DotNetLib/bin/Release/net9.0/DotNetLib.dll",
                L"DotNetLib.Lib, DotNetLib",
                L"Hello",
                nullptr /*delegate_type_name*/,
                nullptr,
                (void**)&hello);
            assert(rc == 0 && hello != nullptr && "Failure: load_assembly_and_get_function_pointer()");
            struct lib_args
            {
                const char_t *message;
                int number;
            };
            for (int i = 0; i < 3; ++i)
            {
                // <SnippetCallManaged>
                lib_args args
                {
                    L"from Visera!",
                    i
                };

                hello(&args, sizeof(args));
                // </SnippetCallManaged>
            }
        }
    };

}