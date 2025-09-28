module;
#include <Visera-Runtime.hpp>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#if !defined(AK_OPTIMIZED)
#include <AK/Comm/AkCommunication.h>	// Communication between Wwise and the game (excluded in release build)
#endif
export module Visera.Runtime.Audio.Wwise;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Core.Log;

namespace Visera
{
    export VISERA_RUNTIME_API class FWwiseAudioEngine : public IAudioEngine
    {
    public:
        [[nodiscard]] inline const auto
        GetStreamManager() const { return AK::IAkStreamMgr::Get(); }

    public:
        explicit FWwiseAudioEngine() : IAudioEngine{EType::Wwise}
        {
            Int32 Result {AKRESULT::AK_Success};

            LOG_TRACE("Initializing Wwise Memory Manager.");
            {
                AkMemSettings MemorySettings;
                AK::MemoryMgr::GetDefaultSettings(MemorySettings);
                Result = AK::MemoryMgr::Init(&MemorySettings);
                if (Result != AKRESULT::AK_Success)
                { LOG_FATAL("Failed to initialize Wwise Memory Manager (error:{})!", Result); }
            }

            LOG_TRACE("Initializing Wwise Stream Manager.");
            {
                AkStreamMgrSettings StreamSettings;
                AK::StreamMgr::GetDefaultSettings(StreamSettings);
                if (!AK::StreamMgr::Create(StreamSettings))
                { LOG_FATAL("Failed to initialize Wwise Stream Manager!"); }
            }

            LOG_TRACE("Initializing Wwise Sound Engine.");
            {
                AkInitSettings InitSettings;
                AkPlatformInitSettings PlatformInitSettings;
                AK::SoundEngine::GetDefaultInitSettings(InitSettings);
                AK::SoundEngine::GetDefaultPlatformInitSettings(PlatformInitSettings);
                Result = AK::SoundEngine::Init(&InitSettings, &PlatformInitSettings);
                if (Result != AKRESULT::AK_Success)
                { LOG_FATAL("Failed to initialize Wwise Sound Engine (error:{})!", Result); }
            }
        }

        ~FWwiseAudioEngine() override
        {
            if (AK::SoundEngine::IsInitialized())
            {
                LOG_TRACE("Terminating Wwise Sound Engine.");
                AK::SoundEngine::Term();
            }

            if (AK::IAkStreamMgr::Get())
            {
                LOG_TRACE("Terminating Wwise Steam Manager.");
                AK::IAkStreamMgr::Get()->Destroy();
            }

            if (AK::MemoryMgr::IsInitialized())
            {
                LOG_TRACE("Terminating Wwise Memory Manager.");
                AK::MemoryMgr::Term();
            }
        }
    };
}
