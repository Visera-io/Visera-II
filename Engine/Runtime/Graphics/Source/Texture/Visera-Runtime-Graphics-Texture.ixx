module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Texture;
#define VISERA_MODULE_NAME "Runtime.Graphics"
// import Visera.Runtime.RHI.Handle;
// import Visera.Core.Image;
//
// export namespace Visera
// {
//     class VISERA_RUNTIME_API FTexture2D
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