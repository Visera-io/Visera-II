module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Context;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.RHI;

export namespace Visera
{
    struct VISERA_RUNTIME_API FRenderContext
    {
        UInt32 ViewportWidth  {0};
        UInt32 ViewportHeight {0};
    };
}
