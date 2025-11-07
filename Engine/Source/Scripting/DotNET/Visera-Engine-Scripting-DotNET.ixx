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
    struct VISERA_ENGINE_API HostFXR
    {
        static const inline FPath DotNETRoot {DOTNET_ROOT};
        // https://github.com/dotnet/runtime/blob/main/docs/design/features/host-error-codes.md
        enum EStatus { Success = 0, };

        static inline hostfxr_initialize_for_runtime_config_fn
        InitializeForRuntimeConfig           {nullptr};
        static inline hostfxr_initialize_for_dotnet_command_line_fn
        InitializeForDotNETCommandLine       {nullptr};
        static inline hostfxr_get_runtime_delegate_fn
        GetRuntimeDelegate                   {nullptr};
        static inline hostfxr_set_error_writer_fn
        SetErrorWriter                       {nullptr};
        static inline load_assembly_and_get_function_pointer_fn
        LoadAssemblyAndGetFunctionPointer    {nullptr};
        static inline component_entry_point_fn
        GetComponentEntryPoint               {nullptr};
        static inline hostfxr_run_app_fn
        RunApp                               {nullptr};
        static inline hostfxr_close_fn
        Close                                {nullptr};
    };

    export class VISERA_ENGINE_API FDotNETApplication
    {
    public:
        [[nodiscard]] Bool
        Run() const { return HostFXR::RunApp(Context) == HostFXR::Success; }

        [[nodiscard]] inline Bool
        IsValid() const { return Context != nullptr; }

    private:
        hostfxr_handle  Context           {nullptr};
        FPath           RuntimeConfigPath {};//{VISERA_ENGINE_DIR "/Global/Visera-DotNETConfig.json"};
        TArray<FPath>   Args{};

    public:
        FDotNETApplication() = delete;
        FDotNETApplication(TArray<FPath>&& I_Args); // Commandline App
        ~FDotNETApplication()
        {
            if (HostFXR::Close(Context) != HostFXR::Success)
            { LOG_ERROR("Failed to finalize App (path:{})!", RuntimeConfigPath); }
        }
    };

    export class VISERA_ENGINE_API FDotNET
    {
    public:
        [[nodiscard]] inline TSharedPtr<FDotNETApplication>
        CreateCommandLineApp(TArray<FPath>&& I_Args) const;

    private:
        TSharedPtr<ILibrary>      HostFXR;
        
   
    public:
        FDotNET()
        {
            LOG_DEBUG("Initializing .NET");
            //[Optional]: Using the nethost library, discover the location of hostfxr and get exports
            HostFXR = GPlatform->LoadLibrary(FPath{HOSTFXR_LIBRARY_NAME});
            if (!HostFXR) { LOG_FATAL("Failed to load HostFXR"); }

            LOG_TRACE("Loading HostFXR functions.");
            HostFXR::InitializeForRuntimeConfig = reinterpret_cast<hostfxr_initialize_for_runtime_config_fn>
                (HostFXR->LoadFunction("hostfxr_initialize_for_runtime_config"));
            HostFXR::GetRuntimeDelegate = reinterpret_cast<hostfxr_get_runtime_delegate_fn>
                (HostFXR->LoadFunction("hostfxr_get_runtime_delegate"));
            HostFXR::RunApp = reinterpret_cast<hostfxr_run_app_fn>
                (HostFXR->LoadFunction("hostfxr_run_app"));
            HostFXR::Close = reinterpret_cast<hostfxr_close_fn>
                (HostFXR->LoadFunction("hostfxr_close"));
            HostFXR::InitializeForDotNETCommandLine = reinterpret_cast<hostfxr_initialize_for_dotnet_command_line_fn>
                (HostFXR->LoadFunction("hostfxr_initialize_for_dotnet_command_line"));
            HostFXR::SetErrorWriter = reinterpret_cast<hostfxr_set_error_writer_fn>
                (HostFXR->LoadFunction("hostfxr_set_error_writer"));

            if (HostFXR::SetErrorWriter)
            {
                HostFXR::SetErrorWriter([](const char_t* I_Message)
                { LOG_ERROR("HostFXR: {}", FText::ToUTF8(I_Message)); });
            }
            else { LOG_ERROR("Failed to set the HostFXR error messenger!"); }
        }
        ~FDotNET()
        {
            LOG_DEBUG("Terminating .NET");
        }

    private:
        inline void
        LoadHostFXR()
        {
            
            // if (HostFXR::InitializeForRuntimeConfig(
            //     ConfigPath.GetNativePath().c_str(),
            //     &HostFXRInitInfo,
            //     &Context) != Success || !Context)
            // { LOG_FATAL("Failed to initialize HostFXR runtime!"); }

            //LOG_TRACE("Extracting HostFXR delegate.");
            //get_function_pointer_fn HostFXR::GetFunction;
            //if (HostFXR::GetRuntimeDelegate(
            //    Context,
            //    hdt_get_function_pointer,
            //    reinterpret_cast<void**>(&HostFXR::GetFunction)) != Success ||
            //    !HostFXR::GetFunction)
            //{ LOG_FATAL("Failed to extract HostFXR delegate!"); }

            // // Function pointer to managed delegate
            // component_entry_point_fn hello = nullptr;
            // int rc = HostFXR::GetFunction(
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
        }
    };

    TSharedPtr<FDotNETApplication> FDotNET::
    CreateCommandLineApp(TArray<FPath>&& I_Args) const
    {
        if (I_Args.empty())
        { LOG_ERROR("Empty arguments!"); return {}; }

        LOG_DEBUG("Creating a commandline App (name:{}).", I_Args[0]);
        return MakeShared<FDotNETApplication>(std::move(I_Args));
    }

    FDotNETApplication::
    FDotNETApplication(TArray<FPath>&& I_Args)
    : Args(I_Args)
    {
        VISERA_ASSERT(!Args.empty());
        RuntimeConfigPath = Args[0];
        LOG_DEBUG("Initializing App (path:\"{}\").", RuntimeConfigPath);

        TArray<const char_t*> Argv(I_Args.size());
        for (UInt8 Idx = 0; Idx < Args.size(); Idx++)
        {
            Argv[Idx] = Args[Idx].GetNativePath().c_str();
        };

        auto HostFXRInitInfo = hostfxr_initialize_parameters
        {
            .dotnet_root = HostFXR::DotNETRoot.GetNativePath().c_str(),
        };

        auto Status = HostFXR::InitializeForDotNETCommandLine(
            Args.size(),
            Argv.data(),
            &HostFXRInitInfo,
            &Context);
        if (Status != HostFXR::Success || !Context)
        { 
            LOG_ERROR("Failed to initialize App (name:{}, error:{})!",
                      RuntimeConfigPath, Status);
            return;
        }
    }
}