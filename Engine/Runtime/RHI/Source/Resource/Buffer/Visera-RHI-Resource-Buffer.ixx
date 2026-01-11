module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.Buffer;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;

export namespace Visera
{
    struct VISERA_RHI_API FRHIBufferCreateDesc
    {

        ERHIBufferUsage  Usages {ERHIBufferUsage::None};
    };
}