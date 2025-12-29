module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph.Node;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"
import Visera.RHI;
import Visera.Shader;
import Visera.Runtime.Name;
import Visera.Core.Delegate.Unicast;
import Visera.Core.Types.Array;
import Visera.Graphics.RenderGraph.Common;

export namespace Visera
{
    struct FRGResourceInfo
    {
        Bool bExternal = False;
        union
        {
            struct
            {
                UInt64          Size;
                ERHIBufferUsage Usages;
                FBufferHandle   Handle;
            }Buffer;

            struct
            {
                UInt32                  Width;
                UInt32                  Height;
                UInt32                  Depth;

                ERHIFormat              Format;
                ERHIImageUsage          Usages;
                ERHIAttachmentLoadOp    LoadOp;
                FTextureHandle          Handle;
            }Texture;
        };
    };

    struct FRGResource
    {
        ERGResourceType     Type {ERGResourceType::Unknown};
        FRGResourceInfo     ResourceInfo;
        FRGNodeHandle       Producer;
        FRGResourceHandle   OutputHandle;
        UInt32              ReferenceCount = 0;
        FName               Name {EName::None};
    };

    struct FRGNode
    {
        FName                       Name {EName::None};
        FRenderPipelineHandle       RenderPipeline;
        FRGFramebufferHandle        Framebuffer;
        TArray<FRGResourceHandle>   Inputs;
        TArray<FRGResourceHandle>   Outputs;
        TArray<FRGNodeHandle>       Edges;

        TUnicastDelegate<void(TSharedRef<FRHICommandBuffer>)>
        Execute;
    };
}