module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Barrier;
#define VISERA_MODULE_NAME "Runtime.RHI"
export import Visera.Runtime.RHI.Common;
export import Visera.Runtime.RHI.Registry;

export namespace Visera
{
    struct FRHIMemoryBarrier
    {
        ERHIPipelineStage SourceStage;
        ERHIPipelineStage DestStage;
        ERHIAccessFlag    SourceAccess;
        ERHIAccessFlag    DestAccess;
    };

    struct FRHIImageBarrier
    {
        FRHITextureID     Image;
        ERHIImageLayout   OldLayout;
        ERHIImageLayout   NewLayout;
        FRHIMemoryBarrier MemoryBarrier;
    };

    struct FRHIBufferBarrier
    {
        FRHIBufferID      Buffer;
        UInt64            Offset {0};
        UInt64            Size   {0};
        FRHIMemoryBarrier MemoryBarrier;
    };
}
