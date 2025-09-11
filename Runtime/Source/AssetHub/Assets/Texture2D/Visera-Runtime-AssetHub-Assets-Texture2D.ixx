module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Assets.Texture2D;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Assets.Interface;

namespace Visera
{
   export VISERA_RUNTIME_API class FTexture2D : public IAsset
   {
   public:
      [[nodiscard]] Bool
      Load()   override;
      [[nodiscard]] Bool
      Unload() override;

      FTexture2D() = delete;
      FTexture2D(EType I_Type, const FName& I_Name, const FPath& I_Path)
      : IAsset(I_Type, I_Name, I_Path) {}
   };

   Bool FTexture2D::
   Load()
   {
      return False;
   }

   Bool FTexture2D::
   Unload()
   {
      return False;
   }

}
