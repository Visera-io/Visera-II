module;
#include <Visera-Runtime.hpp>
#include <nethost.h>
#include <coreclr_delegates.h>
#include <hostfxr.h>
export module Visera.Runtime.Scripting.DotNET;
#define VISERA_MODULE_NAME "Runtime.Scripting"

namespace Visera
{

    export class FDotNET
    {
    public:
        void Initialize()
        {

        }

        // Using the nethost library, discover the location of hostfxr and get exports
        bool load_hostfxr()
        {
            // Pre-allocate a large buffer for the path to hostfxr
            // char_t buffer[256];
            // size_t buffer_size = sizeof(buffer) / sizeof(char_t);
            // int rc = get_hostfxr_path(buffer, &buffer_size, nullptr);
            // if (rc != 0)
            //     return false;

            // Load hostfxr and get desired exports
            // void *lib = load_library(buffer);
            // init_fptr = (hostfxr_initialize_for_runtime_config_fn)get_export(lib, "hostfxr_initialize_for_runtime_config");
            // get_delegate_fptr = (hostfxr_get_runtime_delegate_fn)get_export(lib, "hostfxr_get_runtime_delegate");
            // close_fptr = (hostfxr_close_fn)get_export(lib, "hostfxr_close");
            //
            // return (init_fptr && get_delegate_fptr && close_fptr);
            return False;
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