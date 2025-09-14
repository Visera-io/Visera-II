module;
#include <Visera-Runtime.hpp>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
export module Visera.Runtime.Scripting.DotNET;
#define VISERA_MODULE_NAME "Runtime.Scripting"
import Visera.Core.Log;
import Visera.Core.Types.Path;
import Visera.Runtime.Platform;

namespace Visera
{

    export class FDotNET
    {
        TSharedPtr<ILibrary> HostFXR;
        hostfxr_initialize_for_runtime_config_fn Fn_InitializeRuntime{nullptr};
        hostfxr_get_runtime_delegate_fn          Fn_GetRuntimeDelegate{nullptr};
        hostfxr_close_fn                         Fn_FinalizeRuntime{nullptr};

    public:
        FDotNET()
        {
            //LOG_DEBUG("Initializing .NET");
            Initialize();
        }

        void Initialize()
        {
            if (!LoadHostFXR()) { LOG_ERROR("Failed to load HostFXR"); return; }

            if (!Fn_InitializeRuntime(nullptr, nullptr, nullptr))
            {
                LOG_ERROR("Failed to initialize HostFXR runtime");
            }
            if (!Fn_FinalizeRuntime(nullptr))
            {
                LOG_ERROR("Failed to finalize HostFXR runtime");
            }
        }

    private:
        [[nodiscard]] inline Bool
        LoadHostFXR()
        {
            // Using the nethost library, discover the location of hostfxr and get exports
            HostFXR = GPlatform->LoadLibrary(FPath{HOSTFXR_LIBRARY_NAME});

            Fn_InitializeRuntime = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>
                (HostFXR->LoadFunction("hostfxr_initialize_for_runtime_config"));
            Fn_GetRuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>
                (HostFXR->LoadFunction("hostfxr_get_runtime_delegate"));
            Fn_FinalizeRuntime = reinterpret_cast<hostfxr_close_fn>
                (HostFXR->LoadFunction("hostfxr_close"));

            return Fn_InitializeRuntime  &&
                   Fn_GetRuntimeDelegate &&
                   Fn_FinalizeRuntime;
        }

        // // Load and initialize .NET Core and get desired function pointer for scenario
        // load_assembly_and_get_function_pointer_fn
        // get_dotnet_load_assembly(const char_t *config_path)
        // {
        //     // Load .NET Core
        //     void *load_assembly_and_get_function_pointer = nullptr;
        //     hostfxr_handle cxt = nullptr;
        //     auto rc = hostfxr_initialize_for_runtime_config_fn(config_path, nullptr, &cxt);
        //     if (rc != 0 || cxt == nullptr)
        //     {
        //         std::cerr << "Init failed: " << std::hex << std::showbase << rc << std::endl;
        //         hostfxr_close_fn(cxt);
        //         return nullptr;
        //     }
        //
        //     // Get the load assembly function pointer
        //     rc = hostfxr_get_runtime_delegate_fn(
        //         cxt,
        //         hdt_load_assembly_and_get_function_pointer,
        //         &load_assembly_and_get_function_pointer);
        //     if (rc != 0 || load_assembly_and_get_function_pointer == nullptr)
        //         std::cerr << "Get delegate failed: " << std::hex << std::showbase << rc << std::endl;
        //
        //     hostfxr_close_fn(cxt);
        //     return (load_assembly_and_get_function_pointer_fn)load_assembly_and_get_function_pointer;
        // }
    };

}