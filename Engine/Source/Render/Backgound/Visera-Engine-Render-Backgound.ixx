module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Render.Background;
#define VISERA_MODULE_NAME "Engine.Render"
import Visera.Core.Log;
import Visera.Engine.AssetHub;
import Visera.Engine.Render.Renderer;
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
