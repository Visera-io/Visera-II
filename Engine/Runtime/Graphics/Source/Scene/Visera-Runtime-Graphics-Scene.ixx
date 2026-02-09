module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Scene;
#define VISERA_MODULE_NAME "Runtime.Graphics"
export import Visera.Runtime.Graphics.Scene.Camera;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Name;

export namespace Visera
{
    namespace EName
    {
        VISERA_RUNTIME_API inline const auto
        MainCamera = FName{"camera", 0};
    }

    class VISERA_RUNTIME_API FScene
    {
    public:


    private:
        TMap<FName, FCamera> Cameras;
    };
}