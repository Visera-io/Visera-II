module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph.Common;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"

export namespace Visera
{
    using FRenderGraphHandle = UInt32;

    using FBufferHandle         = FRenderGraphHandle;
    using FTextureHandle        = FRenderGraphHandle;
    using FRGNodeHandle         = FRenderGraphHandle;
    using FRGResourceHandle     = FRenderGraphHandle;
    using FRenderPassHandle     = FRenderGraphHandle;
    using FRGFramebufferHandle  = FRenderGraphHandle;

    enum class ERGAccess
    {
        Read,
        Write,
    };

    enum class ERGResourceType
    {
        Unknown,

        Attachment,
        Texture,
        Buffer,
        Reference,
    };
}