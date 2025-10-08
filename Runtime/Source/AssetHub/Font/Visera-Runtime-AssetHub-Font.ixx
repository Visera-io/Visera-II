module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Font;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
import Visera.Runtime.AssetHub.Asset;
import Visera.Core.Log;

namespace Visera
{
   export class VISERA_RUNTIME_API FFont : public IAsset
   {
   public:
      [[nodiscard]] const FByte*
      GetData() const override { VISERA_UNIMPLEMENTED_API; return nullptr; }
      [[nodiscard]] UInt64
      GetSize() const override { VISERA_UNIMPLEMENTED_API; return 0; }
      
   private:
      

   public:
      FFont() = delete;
      FFont(const FName& I_Name, const FPath& I_Path);
   };

   FFont::
   FFont(const FName& I_Name, const FPath& I_Path)
   : IAsset(EType::Font, I_Name, I_Path)
   {

   }
}