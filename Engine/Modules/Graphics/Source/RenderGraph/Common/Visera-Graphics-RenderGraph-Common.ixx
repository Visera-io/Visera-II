module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderGraph.Common;
#define VISERA_MODULE_NAME "Graphics.RenderGraph"

export namespace Visera
{
    using FBufferHandle  = UInt32;
    using FTextureHandle = UInt32;
    using FRGNodeHandle = UInt32;
    using FRGResourceHandle = UInt32;
    using FRenderPipelineHandle = UInt32;
    using FRGFramebufferHandle = UInt32;

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