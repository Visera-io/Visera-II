module;
#include <Visera-Game.hpp>
export module Visera.Game.AssetHub.Texture;
#define VISERA_MODULE_NAME "Game.AssetHub"
import Visera.Game.AssetHub.Asset;
import Visera.Assets;
import Visera.Global.Log;
//import Visera.RHI;

namespace Visera
{
   // export class VISERA_ENGINE_API FTexture : public IAsset
   // {
   // public:
   //    [[nodiscard]] const FByte*
   //    GetData() const override { VISERA_UNIMPLEMENTED_API; return nullptr; };
   //    [[nodiscard]] UInt64
   //    GetSize() const override { VISERA_UNIMPLEMENTED_API; return 0; };

   // private:

   // public:
   //    FTexture() = delete;
   //    FTexture(const FName& I_Name, const FPath& I_Path);
   //    ~FTexture() override;
   // };

   // FTexture::
   // FTexture(const FName& I_Name, const FPath& I_Path)
   // : IAsset(EType::Texture, I_Name, I_Path)
   // {
   //    auto Image = GAssets->CreateImage(I_Path);
   //    VISERA_UNIMPLEMENTED_API;
   // }

   // FTexture::
   // ~FTexture()
   // {

   // }
}