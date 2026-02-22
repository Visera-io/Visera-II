module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Attachments;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry;
import Visera.Core.Math.Color.Linear;
import Visera.Core.Containers.Array;

export namespace Visera
{
    constexpr UInt32 kMaxColorAttachments = 4;

    struct FRHIColorAttachmentDesc
    {
        FRHITextureID          Texture;
        ERHIAttachmentLoadOp   LoadOp     { ERHIAttachmentLoadOp::Clear };
        ERHIAttachmentStoreOp  StoreOp    { ERHIAttachmentStoreOp::Store };
        FLinearColor           ClearColor { FLinearColor::Purple() };
    };

    struct FRHIDepthStencilAttachmentDesc
    {
        FRHITextureID          Texture;
        ERHIAttachmentLoadOp   DepthLoadOp    { ERHIAttachmentLoadOp::Clear };
        ERHIAttachmentStoreOp  DepthStoreOp   { ERHIAttachmentStoreOp::Store };
        Float                  DepthClear     { 1.0f };
        ERHIAttachmentLoadOp   StencilLoadOp  { ERHIAttachmentLoadOp::DontCare };
        ERHIAttachmentStoreOp  StencilStoreOp { ERHIAttachmentStoreOp::DontCare };
        UInt32                 StencilClear   { 0 };
    };

    struct FRHIRenderPassAttachments
    {
        TArray<FRHIColorAttachmentDesc> ColorTargets;
        FRHIDepthStencilAttachmentDesc  DepthStencil;  // Texture = null/invalid means none
    };
}
