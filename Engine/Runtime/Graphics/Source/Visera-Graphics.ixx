module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics;
#define VISERA_MODULE_NAME "Graphics"
export import Visera.Graphics.Debug;
export import Visera.Graphics.Scene;
export import Visera.Graphics.Material;
       import Visera.Graphics.Texture;
       import Visera.Graphics.RenderGraph;
       import Visera.Global;
       import Visera.RHI;
       import Visera.Core.Image;
       import Visera.Core.Types.Map;
       import Visera.Core.Types.Array;

export namespace Visera
{
   using FTextureID = FRHITextureHandle;

   class VISERA_GRAPHICS_API FGraphics : public IGlobalService
   {
   public:
      [[nodiscard]] FTextureID
      CreateTexture2D(TSharedRef<FImage> I_Image);
      [[nodiscard]] TSharedPtr<FTexture2D>
      GetTexture2D(FTextureID I_TextureID) { VISERA_ASSERT(Textures.Contains(I_TextureID)); return Textures[I_TextureID]; }

      void Tick(Float I_Time)
      {
         RenderGraph.Execute(nullptr);
         RHI->Submit(CommandList);
         CommandList.Reset();
         for (auto Buffer : PendingFreeBuffers)
         {
            RHI->DestroyBuffer(Buffer);
         }
         PendingFreeBuffers.Clear();
      }

   private:
      FRenderGraph    RenderGraph;
      FRHI*           RHI;
      FRHICommandList CommandList;
      TMap<FTextureID, TSharedPtr<FTexture2D>> Textures;
      TArray<FRHIBufferHandle> PendingFreeBuffers;

   public:
      FGraphics() : IGlobalService(EName::Graphics)
      {
         Dependencies =
         {
            EName::RHI,
         };

         if (!OnBootstrap.TryBind([this]
         {
            RHI = Get<FRHI>(EName::RHI);
            return True;
         }))
         { LOG_FATAL("Failed to bind bootstrap function!"); }

         if (!OnTerminate.TryBind([this]
         {
            return True;
         }))
         { LOG_FATAL("Failed to bind terminate function!"); }
      }
   };


   FTextureID FGraphics::
   CreateTexture2D(TSharedRef<FImage> I_Image)
   {
      auto RHIFormat = ERHIFormat::Undefined;
      switch (I_Image->GetPixelFormat())
      {
      case EPixelFormat::RGBA8_UNorm:
         if (I_Image->GetColorSpace() == EColorSpace::sRGB)
         {
            RHIFormat = ERHIFormat::R8G8B8A8_sRGB;
         }
         else
         {
            RHIFormat = ERHIFormat::R8G8B8A8_UNorm;
         }
         break;
      case EPixelFormat::RGBA16_Float:
         RHIFormat = ERHIFormat::R16G16B16A16_Float;
         break;
      default: LOG_FATAL("Unknown pixel format!");
      }

      auto TextureHandle = RHI->CreateTexture(
      {
         .Width      = I_Image->GetWidth(),
         .Height     = I_Image->GetHeight(),
         .Depth      = 1,
         .Format     = RHIFormat,
         .Type       = ERHIImageType::Image2D,
         .Usages     = ERHIImageUsage::ShaderResource | ERHIImageUsage::TransferDst | ERHIImageUsage::TransferSrc,
         .ViewType   = ERHIImageViewType::Image2D,
         .MipLevelRange   = {0, 0},
         .ArrayLayerRange = {0, 0},
         .SampleCount     = ERHISamplingRate::X1,
      });

      if (!Textures.Contains(TextureHandle))
      {
         auto SamplerHandle= RHI->CreateSampler(
         {
            .Type        = ERHISamplerType::Linear,
            .AddressMode = ERHISamplerAddressMode::Repeat,
         });

         auto Buffer = RHI->CreateBuffer({
             .Size   = I_Image->GetSizeInBytes(),
             .Usages = ERHIBufferUsage::TransferSrc
         }, I_Image->GetData(), I_Image->GetSizeInBytes());

         CommandList.CopyBufferToImage(Buffer, TextureHandle);
         CommandList.ConvertImageLayout(TextureHandle, ERHIImageLayout::ShaderReadOnly);

         PendingFreeBuffers.EmplaceBack(Buffer);

         Textures[TextureHandle] = MakeShared<FTexture2D>(TextureHandle, SamplerHandle);
      }

      return TextureHandle;
   }
}