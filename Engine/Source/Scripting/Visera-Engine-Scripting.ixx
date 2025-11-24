module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Scripting;
#define VISERA_MODULE_NAME "Engine.Scripting"
import Visera.Engine.Scripting.DotNET;
import Visera.Core.Global;
import Visera.Core.Log;
import Visera.Core.Types.Path;
import Visera.Runtime.Platform;

namespace Visera
{
    export class VISERA_ENGINE_API FScripting : public IGlobalSingleton<FScripting>
    {
    public:
        using FFunction = FDotNETComponent::FFunction;
        [[nodiscard]] inline FFunction
        GetFunction(FPlatformStringView I_Function) const { return APIs->GetFunction(I_Function); }
        [[nodiscard]] inline TSharedPtr<FDotNETApplication>
        CreateCommandLineApp(TArray<FPath>&& I_Args) const { return DotNET->CreateCommandLineApp(std::move(I_Args)); }

    private:
        TUniquePtr<FDotNET>          DotNET;
        TSharedPtr<FDotNETComponent> APIs;

    public:
        FScripting() : IGlobalSingleton{"Scripting"} {}
        void inline
        Bootstrap() override;
        void inline
        Terminate() override;
    };

    export inline VISERA_ENGINE_API TUniquePtr<FScripting>
    GScripting = MakeUnique<FScripting>();

    void FScripting::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Scripting.");

        DotNET = MakeUnique<FDotNET>();
        APIs = DotNET->CreateComponent(
            GPlatform->GetExecutableDirectory() / FPath("Visera-App.dll"),
            FPath{VISERA_ENGINE_DIR "/Assets/Config/Visera-API.json"}
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
        DotNET.reset();

        Status = EStatus::Terminated;
    }
}