module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Scripting;
#define VISERA_MODULE_NAME "Engine.Scripting"
import Visera.Engine.Scripting.DotNET;
import Visera.Core.Types.Path;

namespace Visera
{
    export class VISERA_ENGINE_API FScripting : public IGlobalSingleton<FScripting>
    {
    public:
        [[nodiscard]] inline TSharedPtr<FDotNETApplication>
        CreateCommandLineApp(TArray<FPath>&& I_Args) const { return DotNET->CreateCommandLineApp(std::move(I_Args)); }

    private:
        TUniquePtr<FDotNET> DotNET;

    public:
        FScripting() : IGlobalSingleton{"Scripting"} {}
        void inline
        Bootstrap() override { DotNET = MakeUnique<FDotNET>(); Status = EStatus::Bootstrapped; }
        void inline
        Terminate() override { Status = EStatus::Terminated; }
    };

    export inline VISERA_ENGINE_API TUniquePtr<FScripting>
    GScripting = MakeUnique<FScripting>();
}