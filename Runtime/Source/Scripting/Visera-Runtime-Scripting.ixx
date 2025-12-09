module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Scripting;
#define VISERA_MODULE_NAME "Runtime.Scripting"
import Visera.Runtime.Scripting.DotNET;
import Visera.Core.Global;
import Visera.Core.Log;
import Visera.Core.Types.Path;
import Visera.Core.Types.Array;
import Visera.Runtime.Platform;

namespace Visera
{
    export class VISERA_RUNTIME_API FScripting : public IGlobalSingleton<FScripting>
    {
    public:
        using FFunction = FDotNETComponent::FFunction;
        [[nodiscard]] inline FFunction
        GetFunction(FPlatformStringView I_Function) const { return APIs->GetFunction(I_Function); }
        [[nodiscard]] inline TSharedPtr<FDotNETApplication>
        CreateCommandLineApp(TArray<FPath>&& I_Args) const { return DotNETRuntime->CreateCommandLineApp(std::move(I_Args)); }

    private:
        TUniquePtr<FDotNETRuntime>   DotNETRuntime;
        TSharedPtr<FDotNETComponent> APIs;

    public:
        FScripting() : IGlobalSingleton{"Scripting"} {}
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FScripting>
    GScripting = MakeUnique<FScripting>();

    void FScripting::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Scripting.");

        DotNETRuntime = MakeUnique<FDotNETRuntime>();
        APIs = DotNETRuntime->CreateComponent(
            GPlatform->GetExecutableDirectory() / FPath("Visera-App.dll"),
            GPlatform->GetExecutableDirectory() / FPath{"Visera-App.runtimeconfig.json"}
        );
        if (!APIs->IsValid())
        { LOG_FATAL("Failed to load APIs!"); }

        Status = EStatus::Bootstrapped;
    }

    void FScripting::
    Terminate()
    {
        LOG_TRACE("Terminating Scripting.");

        APIs.reset();
        DotNETRuntime.reset();

        Status = EStatus::Terminated;
    }
}