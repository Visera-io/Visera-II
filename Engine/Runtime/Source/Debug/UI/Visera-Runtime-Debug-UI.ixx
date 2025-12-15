module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Debug.UI;
#define VISERA_MODULE_NAME "Runtime.Debug"
import Visera.Runtime.Debug.UI.ImGui;
import Visera.Core.Log;
import Visera.Core.Global;


namespace Visera::Debug
{
    export class VISERA_RUNTIME_API FUI : public IGlobalSingleton<FUI>
    {
    public:
        virtual void
        Bootstrap()
        {
            LOG_TRACE("Bootstrapping UI");
            ImGuiContext = MakeUnique<FImGui>();
        }
        virtual void
        Terminate()
        {
            LOG_TRACE("Terminating UI");
            ImGuiContext.reset();
        }

    private:
        TUniquePtr<FImGui> ImGuiContext;

    public:
        FUI() : IGlobalSingleton("UI") {}
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FUI>
    GUI = MakeUnique<FUI>();
}