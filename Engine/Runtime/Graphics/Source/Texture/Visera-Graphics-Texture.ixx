module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.Texture;
#define VISERA_MODULE_NAME "Graphics.Texture"
// import Visera.RHI.Handle;
// import Visera.Core.Image;
//
// export namespace Visera
// {
//     class VISERA_GRAPHICS_API FTexture2D
//     {
//     public:
//         FRHITextureID TextureHandle;
//         FRHISamplerID SamplerHandle;
//
//     public:
//         FTexture2D() = default;
//         FTexture2D(FRHITextureID I_TextureHandle,
//                    FRHISamplerID I_SamplerHandle)
//             : TextureHandle(I_TextureHandle),
//               SamplerHandle(I_SamplerHandle)
//         {
//             VISERA_ASSERT(I_TextureHandle != FRHITextureID{} &&
//                           I_SamplerHandle != FRHISamplerID{});
//         }
//     };
// }