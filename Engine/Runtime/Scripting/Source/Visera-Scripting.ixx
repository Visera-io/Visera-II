module;
#include <Visera-Scripting.hpp>
export module Visera.Scripting;
#define VISERA_MODULE_NAME "Scripting"
import Visera.Scripting.DotNET;
import Visera.Global.Service;
import Visera.Scripting.Log;
import Visera.Core.Types.Path;
import Visera.Core.Types.Array;
import Visera.Scripting.Platform;

namespace Visera
{
    export class VISERA_SCRIPTING_API FScripting : public IGlobalService<FScripting>
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

    export inline VISERA_SCRIPTING_API TUniquePtr<FScripting>
    GScripting = MakeUnique<FScripting>();

    void FScripting::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Scripting.");

        DotNETScripting = MakeUnique<FDotNETScripting>();
        APIs = DotNETScripting->CreateComponent(
            GPlatform->GetFrameworkDirectory() / FPath("Visera-App.dll"),
            GPlatform->GetFrameworkDirectory() / FPath{"Visera/DotNET/Visera.runtimeconfig.json"}
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