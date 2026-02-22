module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Barrier.Buffer;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Registry;
       import Visera.Runtime.RHI.Vulkan.Common;
       import vulkan_hpp;

export namespace Visera
{
    struct FRHIBufferBarrier
    {
        FRHIBufferID Buffer;
        UInt64       Offset {0};
        UInt64       Size   {0};
    };
}
