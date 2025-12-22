module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph.Common;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"

export namespace Visera
{
    enum class ERGAccess
    {
        Read,
        Write,
    };
}