module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Scene.Renderable;
#define VISERA_MODULE_NAME "Runtime.Graphics"
       import Visera.Runtime.RHI;
       import Visera.Runtime.Graphics.Material;
       import Visera.Core.Types.Pointer;

export namespace Visera
{
    class VISERA_RUNTIME_API IRenderable
    {
    public:
        [[nodiscard]] virtual FRHIViewport
        GetViewport() const = 0;
        [[nodiscard]] virtual TSharedPtr<FMaterial>
        GetMaterial() const = 0;

        virtual ~IRenderable() = default;
    };
}