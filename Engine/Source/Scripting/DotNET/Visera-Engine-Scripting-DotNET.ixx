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
        hostfxr_run_app_fn                        Fn_RunApp{nullptr};
        hostfxr_close_fn                          Fn_FinalizeRuntime{nullptr};

        // https://github.com/dotnet/runtime/blob/main/docs/design/features/host-error-codes.md
        enum EStatus
        {
            Success = 0,
        };
    public:
        FDotNET()
        {
            LOG_DEBUG("Initializing .NET");

            LoadHostFXR();
        }
        ~FDotNET()
        {
            LOG_DEBUG("Terminating .NET");

            if (Fn_FinalizeRuntime(Context) != Success)
            { LOG_ERROR("Failed to finalize HostFXR runtime"); }
        }

    private:
        inline void
        LoadHostFXR()
        {
            LOG_TRACE("Loading HostFXR.");
            // Using the nethost library, discover the location of hostfxr and get exports
            HostFXR = GPlatform->LoadLibrary(FPath{HOSTFXR_LIBRARY_NAME});
            if (!HostFXR) { LOG_FATAL("Failed to load HostFXR"); }

            Fn_InitializeRuntime = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>
                (HostFXR->LoadFunction("hostfxr_initialize_for_runtime_config"));
            Fn_GetRuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>
                (HostFXR->LoadFunction("hostfxr_get_runtime_delegate"));
            Fn_RunApp = reinterpret_cast<hostfxr_run_app_fn>
                (HostFXR->LoadFunction("hostfxr_run_app"));
            Fn_FinalizeRuntime = reinterpret_cast<hostfxr_close_fn>
                (HostFXR->LoadFunction("hostfxr_close"));
            auto init_for_cmd_line_fptr = reinterpret_cast<hostfxr_initialize_for_dotnet_command_line_fn>
                (HostFXR->LoadFunction("hostfxr_initialize_for_dotnet_command_line"));


            using hostfxr_set_error_writer_fn = void (*)(hostfxr_error_writer_fn);
            auto Fn_SetErrorWriter = reinterpret_cast<hostfxr_set_error_writer_fn>(
                HostFXR->LoadFunction("hostfxr_set_error_writer"));

            if (Fn_SetErrorWriter)
            {
                Fn_SetErrorWriter([](const char_t* I_Message)
                { LOG_ERROR("HostFXR: {}", FText::ToUTF8(I_Message)); });
            }
            else { LOG_ERROR("Failed to set the HostFXR error messenger!"); }

            LOG_TRACE("Initializing HostFXR.");
            /*TArray<const char_t*> Args
            {
                L"D:/Programs/ViseraEngine/Visera-II/Apps/AlohaVisera/Assets/Script/bin/Release/net10.0/Script.dll",
                L"app_arg_1",
                L"app_arg_2",
            };*/
            //
            // auto HostFXRInitInfo = hostfxr_initialize_parameters
            // {
            //     .dotnet_root = DotNETRoot.GetNativePath().c_str(),
            // };
            //
            // auto AppRC = init_for_cmd_line_fptr(
            //     Args.size(),
            //     Args.data(),
            //     &HostFXRInitInfo,
            //     &Context);
            // if (AppRC != Success || !Context)
            // {std::cerr << "Init failed: " << std::hex << std::showbase << AppRC << std::endl;
            //     LOG_FATAL("Failed to initialize App (error:{})!", AppRC);
            // }

            // if (Fn_InitializeRuntime(
            //     ConfigPath.GetNativePath().c_str(),
            //     &HostFXRInitInfo,
            //     &Context) != Success || !Context)
            // { LOG_FATAL("Failed to initialize HostFXR runtime!"); }

            LOG_TRACE("Extracting HostFXR delegate.");
            get_function_pointer_fn Fn_GetFunction;
            if (Fn_GetRuntimeDelegate(
                Context,
                hdt_get_function_pointer,
                reinterpret_cast<void**>(&Fn_GetFunction)) != Success ||
                !Fn_GetFunction)
            { LOG_FATAL("Failed to extract HostFXR delegate!"); }

            // // Function pointer to managed delegate
            // component_entry_point_fn hello = nullptr;
            // int rc = Fn_GetFunction(
            //     //L"D:/Programs/ViseraEngine/Visera-II/Apps/AlohaVisera/Assets/Script/DotNetLib/bin/Release/net10.0/Script.dll",
            //     L"App, App",
            //     L"Hello",
            //     UNMANAGEDCALLERSONLY_METHOD /*delegate_type_name*/,
            //     nullptr,
            //     nullptr,
            //     (void**)&hello);
            // VISERA_ASSERT(rc == Success && hello != nullptr && "Failure: load_assembly_and_get_function_pointer()");
            // struct lib_args
            // {
            //     const char_t *message;
            //     int number;
            // };
            // for (int i = 0; i < 3; ++i)
            // {
            //     // <SnippetCallManaged>
            //     lib_args args
            //     {
            //         L"from Visera!",
            //         i
            //     };
            //
            //     hello(&args, sizeof(args));
            //     // </SnippetCallManaged>
            // }

            LOG_TRACE("Running HostFXR App.");
            Fn_RunApp(Context);
        }
    };

}