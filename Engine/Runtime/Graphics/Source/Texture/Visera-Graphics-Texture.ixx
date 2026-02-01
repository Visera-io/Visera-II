module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.Texture;
#define VISERA_MODULE_NAME "Graphics.Texture"
import Visera.RHI.Common;
import Visera.Core.Image;

export namespace Visera
{
    class VISERA_GRAPHICS_API FTexture2D
    {
    public:

        FRHITextureHandle TextureHandle;
        FRHISamplerHandle SamplerHandle;

    public:
        FTexture2D() = default;
        FTexture2D(FRHITextureHandle I_TextureHandle,
                   FRHISamplerHandle I_SamplerHandle)
            : TextureHandle(I_TextureHandle),
              SamplerHandle(I_SamplerHandle)
        {
            VISERA_ASSERT(I_TextureHandle != FRHITextureHandle{} &&
                          I_SamplerHandle != FRHISamplerHandle{});
        }
    };
}