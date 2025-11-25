module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Common;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Traits.Flags;
import Visera.Core.Log;

export namespace Visera
{
    enum class ERHIFormat
    {
        R8G8B8_Float,
        R8G8B8_sRGB,
        R8G8B8_UNorm,

        B8G8R8_Float,
        B8G8R8_sRGB,
        B8G8R8_UNorm,

        R8G8B8A8_Float,
        R8G8B8A8_sRGB,
        R8G8B8A8_UNorm,

        B8G8R8A8_Float,
        B8G8R8A8_sRGB,
        B8G8R8A8_UNorm,

        Vector4F = R8G8B8A8_Float,
    };

    enum class ERHISetBindingType
    {
        Undefined = 0,
        CombinedImageSampler,
        UniformBuffer,
        StorageBuffer,
    };

    enum class ERHIShaderStages : UInt32
    {
        Undefined = 0,
        Vertex    = 1 << 0,
        Fragment  = 1 << 1,
    };
    VISERA_MAKE_FLAGS(ERHIShaderStages);
}