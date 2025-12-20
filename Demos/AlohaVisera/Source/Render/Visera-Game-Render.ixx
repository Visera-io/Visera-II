module;
#include <Visera-Game.hpp>
export module Visera.Game.Render;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Runtime.Global;
import Visera.Runtime.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FRender : public IGlobalSingleton<FRender>
    {
    public:
        void inline
        Tick(Float I_DeltaTime)
        {
        }

    private:

    public:
        void
        Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Render.");
        }
        void
        Terminate() override
        {
            LOG_TRACE("Terminating Render.");
        }

        FRender() : IGlobalSingleton("Render") {}
    };

    export inline VISERA_ENGINE_API TUniquePtr<FRender>
    GRender = MakeUnique<FRender>();
}
