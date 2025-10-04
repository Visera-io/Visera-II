module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Asset;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Core.Types.Name;
export import Visera.Core.Types.Path;
       import Visera.Core.Attribute;
       import Visera.Core.Log;

namespace Visera
{
   export VISERA_RUNTIME_API class IAsset
      : public Attribute::SharedOnly<IAsset>
   {
   public:
      enum class EType
      {
         Unknown,
         Shader,
         Image,
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
