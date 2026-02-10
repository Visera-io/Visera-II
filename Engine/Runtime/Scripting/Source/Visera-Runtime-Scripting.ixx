module;
#include <Visera-Scripting.hpp>
export module Visera.Runtime.Scripting;
#define VISERA_MODULE_NAME "Runtime.Scriptin"
import Visera.Runtime.Scripting.DotNET;
import Visera.Runtime.Global.Service;
import Visera.Runtime.Scripting.Log;
import Visera.Core.Types.Path;
import Visera.Core.Containers.Array;
import Visera.Runtime.Scripting.Platform;

namespace Visera
{
    export class VISERA_RUNTIME_API FScripting : public IGlobalService
    {
    public:
        using FFunction = FDotNETComponent::FFunction;
        [[nodiscard]] inline FFunction
        GetFunction(FPlatformStringView I_Function) const { return APIs->GetFunction(I_Function); }
        [[nodiscard]] inline TSharedPtr<FDotNETApplication>
        CreateCommandLineApp(TArray<FPath>&& I_Args) const { return DotNETScripting->CreateCommandLineApp(std::move(I_Args)); }

    private:
        TUniquePtr<FDotNETScripting>   DotNETScripting;
        TSharedPtr<FDotNETComponent> APIs;

    public:
        FScripting() : IGlobalService{"Scripting"} {}
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

        DotNETScripting = MakeUnique<FDotNETScripting>();
        APIs = DotNETScripting->CreateComponent(
            GPlatform->GetFrameworkDirectory() / FPath("Visera-App.dll"),
            GPlatform->GetFrameworkDirectory() / FPath{"Visera/DotNET/Visera.Runtime.runtimeconfig.json"}
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
        DotNETScripting.reset();

        Status = EStatus::Terminated;
    }
}