module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Assets.Interface;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Core.Types.Name;
export import Visera.Core.Types.Path;

namespace Visera
{
   export VISERA_RUNTIME_API class IAsset
   {
   public:
      enum class EStatus
      {
         Unloaded,
         Loaded,
         Dirty,
      };

      enum class EType
      {
         Texture,
      };

      [[nodiscard]] virtual Bool
      Load()   = 0;
      [[nodiscard]] virtual Bool
      Unload() = 0;
      [[nodiscard]] virtual Bool
      Reload();

      void inline
      MarkAsDirty() const { Status = EStatus::Dirty; }
      [[nodiscard]] inline EType
      GetType() const { return Type; }
      [[nodiscard]] inline const FName&
      GetName() const { return Name; }
      [[nodiscard]] inline const FPath&
      GetPath() const { return Path; }
      [[nodiscard]] inline Bool
      IsLoaded() const { return Status != EStatus::Unloaded; }
      [[nodiscard]] inline Bool
      IsDirty() const { return Status == EStatus::Dirty; }

      IAsset() = delete;
      IAsset(EType I_Type, const FName& I_Name, const FPath& I_Path)
      : Type{I_Type}, Name{I_Name}, Path{I_Path} {}
      virtual ~IAsset() = default;

   private:
      const EType Type;
      const FName Name;
      const FPath Path;
      mutable EStatus Status { EStatus::Unloaded };
   };

   Bool IAsset::
   Reload()
   {
      if (IsLoaded())
      {
         if (Unload())
         {
            return Load();
         }
         return False;
      }
      return False;
   }
}
