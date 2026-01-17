module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.Scene;
#define VISERA_MODULE_NAME "Graphics.Scene"
export import Visera.Graphics.Scene.Camera;
       import Visera.Core.Types.Map;
       import Visera.Global.Name;

export namespace Visera
{
    namespace EName
    {
        VISERA_GRAPHICS_API inline const auto
        MainCamera = FName{"camera", 0};
    }

    class VISERA_GRAPHICS_API FScene
    {
    public:


    private:
        TMap<FName, FCamera> Cameras;
    };
}