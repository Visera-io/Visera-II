module;
#include <Visera-Core.hpp>
export module Visera.Core.Types.Color:Common;
#define VISERA_MODULE_NAME "Core.Types.Color:Common"

export namespace Visera
{
    /**
     * Enum for the different kinds of gamma spaces we expect to need to convert from/to.
     */
    enum class EGammaSpace : UInt8
    {
        Invalid,
        /** No gamma correction is applied to this space, the incoming colors are assumed to already be in linear space. */
        Linear,
        /** Use the standard sRGB conversion. */
        sRGB,
        /** A simplified sRGB gamma correction is applied, pow(1/2.2). */
        Pow22,
    };
}