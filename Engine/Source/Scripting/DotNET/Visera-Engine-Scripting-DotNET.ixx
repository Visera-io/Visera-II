module;
#include <Visera-Engine.hpp>
//#include <nethost.h>
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
        static inline hostfxr_run_app_fn
        RunApp                               {nullptr};
        static inline hostfxr_close_fn
        Close                                {nullptr};
    };

    export class VISERA_ENGINE_API FDotNETComponent
    {
    public:
        using FFunction = component_entry_point_fn;
        [[nodiscard]] FFunction
        GetFunction(FPlatformStringView I_Name);

        [[nodiscard]] inline Bool
        IsValid() const { return Context != nullptr && LoadAssemblyAndGetFunctionPointer != nullptr; }

    private:
        hostfxr_handle          Context             {nullptr};
        FPath                   RuntimeConfigPath;
        TMap<FPlatformString, FFunction> Functions;
        load_assembly_and_get_function_pointer_fn
        LoadAssemblyAndGetFunctionPointer {nullptr};

    public:
        FDotNETComponent();
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
        get_function_pointer_fn GetFunctionPointer  {nullptr};

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
            LOG_TRACE("Initializing .NET");
            //[Optional]: Using the nethost library, discover the location of hostfxr and get exports
            HostFXR = GPlatform->LoadLibrary(GPlatform->GetExecutableDirectory() / FPath{HOSTFXR_LIBRARY_NAME});
            if (!HostFXR->IsLoaded()) { LOG_FATAL("Failed to load HostFXR"); }

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
            LOG_TRACE("Terminating .NET");
        }
    };

    FDotNETComponent::FFunction FDotNETComponent::
    GetFunction(FPlatformStringView I_Name)
    {
        auto& Function = Functions[I_Name.data()];
        if (!Function)
        {
            static const auto DLLPath = GPlatform->GetExecutableDirectory() / PATH("Visera-App.dll");
            auto Result = LoadAssemblyAndGetFunctionPointer(
                DLLPath.GetNativePath().c_str(), // [WARN]: Use .dll instead of .dylib on MacOS!
                PLATFORM_STRING("App, Visera-App"),
                I_Name.data(),
                nullptr,
                nullptr,
                reinterpret_cast<void**>(&Function));
            
            if (Result != HostFXR::Success || !Function )
            {
                LOG_ERROR("Failed to load the function \"{}\" from \"{}\" (error:{})!",
                          FText::ToUTF8(I_Name.data()), RuntimeConfigPath, Result);
            }
        }
        return Function;
    }

    TSharedPtr<FDotNETApplication> FDotNET::
    CreateCommandLineApp(TArray<FPath>&& I_Args) const
    {
        if (I_Args.empty())
        { LOG_ERROR("Empty arguments!"); return {}; }

        LOG_DEBUG("Creating a commandline App (name:{}).", I_Args[0]);
        return MakeShared<FDotNETApplication>(std::move(I_Args));
    }

    FDotNETComponent::
    FDotNETComponent()
    {
        auto HostFXRInitInfo = hostfxr_initialize_parameters
        {
            .dotnet_root = HostFXR::DotNETRoot.GetNativePath().c_str(),
        };
        //[TODO]: Remove this test code
        RuntimeConfigPath = FPath{VISERA_ENGINE_DIR "/Global/Visera-Scripting.json"};
        if (HostFXR::InitializeForRuntimeConfig(
            RuntimeConfigPath.GetNativePath().c_str(),
            &HostFXRInitInfo,
            &Context) != HostFXR::Success || !Context)
        {
            LOG_ERROR("Failed to initialize Component (path:\"{}\")!",
                      RuntimeConfigPath);
            return;
        }

        if (HostFXR::GetRuntimeDelegate(
            Context,
            hdt_load_assembly_and_get_function_pointer,
            reinterpret_cast<void**>(&LoadAssemblyAndGetFunctionPointer)) != HostFXR::Success ||
            !LoadAssemblyAndGetFunctionPointer)
        {
            LOG_ERROR("Failed to load function pointer for \"{}\"!",
                      RuntimeConfigPath);
            return;
        }
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