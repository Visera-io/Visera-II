module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Asset.Texture;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Asset.Interface;

namespace Visera
{
   export VISERA_RUNTIME_API class FTextureAsset : public IAsset
   {
   public:
      [[nodiscard]] Bool
      Load()   override;
      [[nodiscard]] Bool
      Unload() override;

      FTextureAsset() = delete;
      FTextureAsset(EType I_Type, const FName& I_Name, const FPath& I_Path)
      : IAsset(I_Type, I_Name, I_Path) {}
   };

   Bool FTextureAsset::
   Load()
   {
      return False;
   }

   Bool FTextureAsset::
   Unload()
   {
      return False;
   }

}
