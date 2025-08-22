module;
#include <Visera-Engine.hpp>
export module Visera.Engine;
#define VISERA_MODULE_NAME "Engine"
import Visera.Engine.Object;
import Visera.Engine.Platform;
import Visera.Engine.Render;
import Visera.Core.Log;

namespace Visera
{
    class VISERA_ENGINE_API VEngine final : public VObject
    {
    public:
        [[nodiscard]] inline auto
        GetWindow() const -> const TUniquePtr<VWindow>& { return Window; }

    private:
        TUniquePtr<VWindow> Window;

    public:
        VEngine();
    };

    export inline VISERA_ENGINE_API
    TUniquePtr<VEngine> GEngine = MakeUnique<VEngine>();

    VEngine::
    VEngine()
    {
        Window = MakeUnique<VWindow>();
    }


}
