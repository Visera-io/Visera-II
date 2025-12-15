module;
#include <Visera-Game.hpp>
export module Visera.Game.Render.Background;
#define VISERA_MODULE_NAME "Game.Render"
import Visera.Core.Log;
import Visera.Game.AssetHub;
import Visera.Game.Render.Renderer.Interface;
import Visera.Runtime.RHI; //[TODO]: Remove

namespace Visera
{
    export class VISERA_ENGINE_API FBackgroundRenderer : public IRenderer
    {
    public:
        void
        Render(Float I_DeltaTime) override
        {
            VISERA_UNIMPLEMENTED_API;
        }

    private:

    public:
        FBackgroundRenderer()
        {

        }
    };
}
