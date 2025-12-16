module;
#include <Visera-Game.hpp>
export module Visera.Game.AssetHub.Asset;
#define VISERA_MODULE_NAME "Game.AssetHub"
export import Visera.Runtime.Name;
export import Visera.Core.Types.Path;
       import Visera.Runtime.Log;

namespace Visera
{
   export class VISERA_ENGINE_API IAsset
   {
   public:
      enum class EType
      {
         Unknown,
         Shader,
         Sound,
         Texture,
      };
      [[nodiscard]] virtual Bool
      Save() const { VISERA_UNIMPLEMENTED_API; return False; }

      [[nodiscard]] virtual const FByte*
      GetData() const = 0;
      [[nodiscard]] virtual UInt64
      GetSize() const = 0;

      [[nodiscard]] inline EType
      GetType() const { return Type; }
      [[nodiscard]] inline const FName&
      GetName() const { return Name; }
      [[nodiscard]] inline const FPath&
      GetPath() const { return Path; }

      [[nodiscard]] inline Bool
      IsEmpty() const { return GetSize() == 0; }

   private:
      const EType Type {EType::Unknown};
      const FName Name;
      const FPath Path;

   public:
      IAsset() = delete;
      IAsset(EType I_Type, const FName& I_Name, const FPath& I_Path)
      : Type{I_Type}, Name{I_Name}, Path{I_Path} {}
      virtual ~IAsset() = default;
   };
}
