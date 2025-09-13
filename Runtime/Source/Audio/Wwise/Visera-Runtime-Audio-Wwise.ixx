module;
#include <Visera-Runtime.hpp>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
export module Visera.Runtime.Audio.Wwise;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Core.Log;

namespace Visera
{
    export VISERA_RUNTIME_API class FWwiseAudioEngine : public IAudioEngine
    {
    public:
        explicit FWwiseAudioEngine() : IAudioEngine{EType::Wwise}
        {
            // Memory Manager
            // AkMemSettings MemorySettings;
            // AK::MemoryMgr::GetDefaultSettings(MemorySettings);
            // if (AK::MemoryMgr::Init(&MemorySettings))
            // {
            //     LOG_ERROR("Failed to initialize Wwise Memory Manager!");
            //     return;
            // }
            //
            // // Streaming Manager
            // AkStreamMgrSettings StreamSettings;
            // AK::StreamMgr::GetDefaultSettings(StreamSettings);
            // if (!AK::StreamMgr::Create(StreamSettings))
            // {
            //     LOG_ERROR("Failed to initialize Wwise Stream Manager!");
            //     return;
            // }
            //
            // // Sound Engine
            // AkInitSettings InitSettings;
            // AkPlatformInitSettings PlatformInitSettings;
            // AK::SoundEngine::GetDefaultInitSettings(InitSettings);
            // AK::SoundEngine::GetDefaultPlatformInitSettings(PlatformInitSettings);
            // if (!AK::SoundEngine::Init(&InitSettings, &PlatformInitSettings))
            // {
            //     LOG_ERROR("Failed to initialize Wwise Sound Engine!");
            //     return;
            // }
        }
    };
}
