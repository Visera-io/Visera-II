module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph.Node;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"
import Visera.Shader;
import Visera.Runtime.Name;
import Visera.Core.Delegate.Unicast;
import Visera.Core.Types.Array;
import Visera.Graphics.RenderGraph.Common;

export namespace Visera
{
    // struct FRGResourceInfo
    // {
    //     Bool bExternal = False;
    //     union
    //     {
    //         struct
    //         {
    //             UInt64          Size;
    //             ERHIBufferUsage Usages;
    //             FBufferHandle   Handle;
    //         }Buffer;

    //         struct
    //         {
    //             UInt32                  Width;
    //             UInt32                  Height;
    //             UInt32                  Depth;

    //             ERHIFormat              Format;
    //             ERHIImageUsage          Usages;
    //             ERHIAttachmentLoadOp    LoadOp;
    //             FTextureHandle          Handle;
    //         }Texture;
    //     };
    // };

    // struct FRGResource
    // {
    //     ERGResourceType     Type {ERGResourceType::Unknown};    // Resource Type (Texture, Buffer, ...)
    //     FRGResourceInfo     ResourceInfo;                       // Details based on the Type
    //     FRGNodeHandle       Producer;                           // Be used to determine the edge of the graph
    //     FRGResourceHandle   OutputHandle;                       // Store the parent resource
    //     UInt32              ReferenceCount = 0;                 // For resource aliasing
    //     FName               Name;                 // Debug/Retrieve by name
    // };

    // struct FRGNode
    // {
    //     FName                       Name;

    //     FRenderPassHandle       RenderPassHandle;
    //     FRGFramebufferHandle        FramebufferHandle;
    //     //FRGRenderPass*       RenderPass

    //     TArray<FRGResourceHandle>   Inputs;
    //     TArray<FRGResourceHandle>   Outputs;
    //     TArray<FRGNodeHandle>       Edges;

    //     TUnicastDelegate<void(FRHICommandBuffer*)>
    //     Execute;
    // };
}