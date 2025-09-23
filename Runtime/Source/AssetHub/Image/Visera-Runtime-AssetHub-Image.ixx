module;
#include <Visera-Runtime.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_RESIZE2_IMPLEMENTATION
#include <stb_image_resize2.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
export module Visera.Runtime.AssetHub.Image;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Asset;
import Visera.Runtime.AssetHub.Image.Wrapper;
import Visera.Runtime.AssetHub.Image.PNG;
import Visera.Core.Log;

namespace Visera
{
   export class VISERA_RUNTIME_API FImage : public IAsset
   {
   public:
      [[nodiscard]] const FByte*
      GetData() const override { return Data.data(); }
      [[nodiscard]] UInt64
      GetSize() const override { return Data.size(); }
      [[nodiscard]] inline UInt32
      GetWidth() const { return Wrapper->GetWidth(); }
      [[nodiscard]] inline UInt32
      GetHeight() const { return Wrapper->GetHeight(); }
      [[nodiscard]] inline UInt8
      GetBitDepth() const { return Wrapper->GetBitDepth(); }
      [[nodiscard]] EImageFormat
      GetFormat() const { return Format; }

      [[nodiscard]] Bool
      Resize(UInt32 I_Width, UInt32 I_Height);

      [[nodiscard]] inline Bool
      IsSRGB() const { return Wrapper->IsSRGB(); }
      [[nodiscard]] Bool
      IsRGBA() const { return Wrapper->IsRGBA(); }
      [[nodiscard]] Bool
      IsBGRA() const { return Wrapper->IsBGRA(); }
      [[nodiscard]] Bool
      HasAlpha() const { return Wrapper->HasAlpha(); }

      [[nodiscard]] FByte*
      Access() { return Data.data(); }

   private:
      TArray<FByte> Data;
      EImageFormat  Format { EImageFormat::Invalid };
      TUniquePtr<IImageWrapper> Wrapper;

   public:
      FImage() = delete;
      FImage(const FName& I_Name, const FPath& I_Path);
   };

   FImage::
   FImage(const FName& I_Name, const FPath& I_Path)
   : IAsset(EType::Image, I_Name, I_Path)
   {
      const FPath Extension = GetPath().GetExtension();

      if (Extension == PATH(".png"))
      {
         Wrapper = MakeUnique<FPNGImageWrapper>();
         Data = Wrapper->Import(GetPath());
         Format = EImageFormat::PNG;
      }
      else
      {
         LOG_ERROR("Failed to load the image \"{}\""
                   "-- unsupported extension {}!", GetPath(), Extension);
         return;
      }

      LOG_DEBUG("Loaded the image \"{}\" (extend:[{},{}], sRGB:{}).",
                GetPath(), Wrapper->GetWidth(), Wrapper->GetHeight(), Wrapper->IsSRGB());
   }

   Bool FImage::
   Resize(UInt32 I_Width, UInt32 I_Height)
   {
      TArray<FByte> Buffer(I_Width * I_Height * (HasAlpha()? 4U : 3U));

      Bool bSuccessed { True };

      if (IsSRGB())
      {
         bSuccessed = stbir_resize_uint8_srgb(
             Data.data(), GetWidth(), GetHeight(), 0,
             Buffer.data(), I_Width, I_Height, 0,
             static_cast<stbir_pixel_layout>(Format));
      }
      else
      {
         bSuccessed = stbir_resize_uint8_linear(
             Data.data(), GetWidth(), GetHeight(), 0,
             Buffer.data(), I_Width, I_Height, 0,
             static_cast<stbir_pixel_layout>(Format));
      }

      if (bSuccessed)
      {
         LOG_DEBUG("The image ({}) was resized from [{},{}] to [{},{}].",
                   GetPath(), GetWidth(), GetHeight(), I_Width, I_Height);

         Data = std::move(Buffer);
         Wrapper->SetWidth(I_Width);
         Wrapper->SetHeight(I_Height);
      }
      else { LOG_ERROR("Failed to resize the image \"{}\"!", GetPath());  }

      return bSuccessed;
   }
}