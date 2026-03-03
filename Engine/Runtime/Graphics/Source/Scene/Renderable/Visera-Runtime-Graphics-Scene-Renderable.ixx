module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Scene.Renderable;
#define VISERA_MODULE_NAME "Runtime.Graphics"

export namespace Visera
{
    class VISERA_RUNTIME_API IRenderable
    {
    public:

    public:
        virtual ~IRenderable() = default;
    };
}