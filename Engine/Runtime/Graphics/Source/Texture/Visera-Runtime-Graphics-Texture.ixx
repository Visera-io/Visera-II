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
//         FRHITextureHandle TextureHandle;
//         FRHISamplerHandle SamplerHandle;
//
//     public:
//         FTexture2D() = default;
//         FTexture2D(FRHITextureHandle I_TextureHandle,
//                    FRHISamplerHandle I_SamplerHandle)
//             : TextureHandle(I_TextureHandle),
//               SamplerHandle(I_SamplerHandle)
//         {
//             VISERA_ASSERT(I_TextureHandle != FRHITextureHandle{} &&
//                           I_SamplerHandle != FRHISamplerHandle{});
//         }
//     };
// }