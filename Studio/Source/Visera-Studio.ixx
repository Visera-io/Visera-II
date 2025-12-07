module;
#include <Visera-Studio.hpp>
export module Visera.Studio;
#define VISERA_MODULE_NAME "Studio"
import Visera.Core.Global;
import Visera.Core.Log;
import Visera.Studio.UI;
import Visera.Engine.Event;

namespace Visera
{
    export class VISERA_STUDIO_API FStudio : public IGlobalSingleton<FStudio>
    {
    public:
        TUniquePtr<FStudioUI>& UI; // Alias of UISystem

    private:
        TUniquePtr<FStudioUI> UISystem;

    public:
        FStudio() : IGlobalSingleton("Studio"), UI {UISystem} {}
        void
        Bootstrap() override;
        void
        Terminate() override;
    };

    export inline VISERA_STUDIO_API TUniquePtr<FStudio>
    GStudio = MakeUnique<FStudio>();

    void FStudio::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping Studio");

        UISystem = MakeUnique<FStudioUI>();
        GEvent->OnFrameBegin.Subscribe([this](){ UISystem->BeginFrame(); });
        GEvent->OnFrameEnd.Subscribe([this](){ UISystem->EndFrame(); });
    }

    void FStudio::
    Terminate()
    {
        LOG_DEBUG("Terminating Studio");

        UISystem.reset();
    }

}