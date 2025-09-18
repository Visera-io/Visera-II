module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Image;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Interface;
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

      [[nodiscard]] EImageFormat
      GetFormat() const { return Format; }

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