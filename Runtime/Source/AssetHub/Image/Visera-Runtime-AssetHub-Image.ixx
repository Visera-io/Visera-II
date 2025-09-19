module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Image;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Asset;
import Visera.Runtime.AssetHub.Image.Importer;
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
      GetWidth() const { return Importer->GetWidth(); }
      [[nodiscard]] inline UInt32
      GetHeight() const { return Importer->GetHeight(); }
      [[nodiscard]] inline UInt8
      GetBitDepth() const { return Importer->GetBitDepth(); }
      [[nodiscard]] EImageFormat
      GetFormat() const { return Format; }

      [[nodiscard]] inline Bool
      IsSRGB() const { return Importer->IsSRGB(); }
      [[nodiscard]] Bool
      IsRGBA() const { return Importer->GetColorFormat() == EColorFormat::RGBA; }
      [[nodiscard]] Bool
      IsBGRA() const { return Importer->GetColorFormat() == EColorFormat::BGRA; }
      [[nodiscard]] Bool
      HasAlpha() const { return IsRGBA() || IsBGRA(); }

      [[nodiscard]] FByte*
      Access() { return Data.data(); }

   private:
      TArray<FByte> Data;
      EImageFormat  Format { EImageFormat::Invalid };
      TUniquePtr<IImageImporter> Importer;

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
         Importer = MakeUnique<FPNGImageImporter>();
         Data = Importer->Import(GetPath());
         Format = EImageFormat::PNG;
      }
      else
      {
         LOG_ERROR("Failed to load the image \"{}\""
                   "-- unsupported extension {}!", GetPath(), Extension);
         return;
      }

      LOG_DEBUG("Loaded the image \"{}\" (extend:[{},{}], sRGB:{}).",
                GetPath(), Importer->GetWidth(), Importer->GetHeight(), Importer->IsSRGB());
   }
}