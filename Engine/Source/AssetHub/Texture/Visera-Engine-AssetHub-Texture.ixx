module;
#include <Visera-Engine.hpp>
export module Visera.Engine.AssetHub.Texture;
#define VISERA_MODULE_NAME "Engine.AssetHub"
import Visera.Engine.AssetHub.Asset;
import Visera.Runtime.Media;
import Visera.Runtime.RHI;
import Visera.Core.Log;

namespace Visera
{
   export class VISERA_ENGINE_API FTexture : public IAsset
   {
   public:
      [[nodiscard]] const FByte*
      GetData() const override { VISERA_UNIMPLEMENTED_API; return nullptr; };
      [[nodiscard]] UInt64
      GetSize() const override { return Data->GetMemorySize(); };

   private:
      TSharedPtr<RHI::FImage> Data;

   public:
      FTexture() = delete;
      FTexture(const FName& I_Name, const FPath& I_Path);
      ~FTexture() override;
   };

   FTexture::
   FTexture(const FName& I_Name, const FPath& I_Path)
   : IAsset(EType::Texture, I_Name, I_Path)
   {
      auto Image = GMedia->CreateImage(I_Path);
      Data = GRHI->CreateTexture2D(Image);
   }

   FTexture::
   ~FTexture()
   {
      Data.reset();
   }
}