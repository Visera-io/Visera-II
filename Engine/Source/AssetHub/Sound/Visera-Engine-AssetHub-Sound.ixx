module;
#include <Visera-Engine.hpp>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
export module Visera.Engine.AssetHub.Sound;
#define VISERA_MODULE_NAME "Engine.AssetHub"
import Visera.Engine.AssetHub.Asset;
import Visera.Core.Log;

namespace Visera
{
   export class VISERA_ENGINE_API FSound : public IAsset
   {
   public:
      [[nodiscard]] const FByte*
      GetData() const override { VISERA_UNIMPLEMENTED_API; return nullptr; };
      [[nodiscard]] UInt64
      GetSize() const override { VISERA_UNIMPLEMENTED_API; return 0; };

      [[nodiscard]] inline AkBankID
      GetID() const { return ID; }
      [[nodiscard]] inline Bool
      IsInvalid() const { return ID == AK_INVALID_BANK_ID; }

   private:
      AkBankID ID {AK_INVALID_BANK_ID};

   public:
      FSound() = delete;
      FSound(const FName& I_Name, const FPath& I_Path);
      ~FSound();
   };

   FSound::
   FSound(const FName& I_Name, const FPath& I_Path)
   : IAsset(EType::Sound, I_Name, I_Path)
   {
      const FPath Extension = GetPath().GetExtension();

      if (Extension == PATH(".bnk"))
      {
         VISERA_ASSERT(AK::SoundEngine::IsInitialized());
         //auto WidePath = I_Path.GetNativePath().wstring();
         auto Result = AK::SoundEngine::LoadBank(I_Path.GetNativePath().c_str(), ID);
         switch (Result)
         {
         case AK_Success:            break;
         case AK_BankAlreadyLoaded:  LOG_WARN("Bank \"{}\" already loaded!", I_Path); break;
         case AK_WrongBankVersion:   LOG_ERROR("Wrong bank version!"); break;
         default:                    LOG_FATAL("Unknown Error ({})!", Int32(Result)); break;
         }
      }
      else
      {
         LOG_ERROR("Failed to load the sound \"{}\""
                   "-- unsupported extension {}!", GetPath(), Extension);
         return;
      }
   }

   FSound::
   ~FSound()
   {
      if (AK::SoundEngine::UnloadBank(GetPath().GetNativePath().c_str(), nullptr) != AK_Success)
      { LOG_ERROR("Failed to unload the sound \"{}\"!", GetPath()); }
   }
}