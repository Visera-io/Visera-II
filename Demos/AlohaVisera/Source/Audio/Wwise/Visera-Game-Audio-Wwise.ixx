module;
#include <Visera-Game.hpp>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#if !defined(AK_OPTIMIZED)
#include <AK/Comm/AkCommunication.h>
#endif
export module Visera.Game.Audio.Wwise;
#define VISERA_MODULE_NAME "Game.Audio"
import Visera.Game.Audio.Interface;
import Visera.Game.Audio.Wwise.IO;
import Visera.Core.Types.Text;
import Visera.Runtime.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FWwiseAudioEngine : public IAudioEngine
    {
    public:
        void
        Tick(Float I_Seconds) override;

        [[nodiscard]] inline const auto
        GetStreamManager() const { return AK::IAkStreamMgr::Get(); }

    private:
        FWwiseIO   IO;
        AkDeviceID DeviceID {AK_INVALID_DEVICE_ID};

    public:
        explicit FWwiseAudioEngine() : IAudioEngine{EType::Wwise}
        {
            LOG_TRACE("Initializing Wwise Memory Manager.");
            {
                AkMemSettings MemorySettings{};
                AK::MemoryMgr::GetDefaultSettings(MemorySettings);

                auto Result = AK::MemoryMgr::Init(&MemorySettings);
                if (Result != AKRESULT::AK_Success)
                { LOG_FATAL("Failed to initialize Wwise Memory Manager (error:{})!", Int32(Result)); }
            }

            LOG_TRACE("Initializing Wwise Stream Manager.");
            {
                AkStreamMgrSettings StreamSettings{};
                AK::StreamMgr::GetDefaultSettings(StreamSettings);

                if (!AK::StreamMgr::Create(StreamSettings))
                { LOG_FATAL("Failed to initialize Wwise Stream Manager!"); }

                //if (AK::StreamMgr::SetCurrentLanguage(AKTEXT( "English(US)")) != AK_Success)
                //{ LOG_FATAL("Failed to set current language as English(US)!"); }
                //else
                //{ LOG_DEBUG("Wwise Stream Manager language is set as English(US)."); }

                AK::StreamMgr::SetFileLocationResolver(&IO);
            }

            LOG_TRACE("Creating Wwise Streaming Device.");
            {
                AkDeviceSettings DeviceSettings{};
                AK::StreamMgr::GetDefaultDeviceSettings(DeviceSettings);

                IO.Initialize(DeviceSettings);

                if (AK::StreamMgr::CreateDevice(DeviceSettings, &IO, DeviceID) != AK_Success)
                { LOG_FATAL("Failed to create the Wwise streaming device!"); }
            }

#if !defined(AK_OPTIMIZED)
            LOG_TRACE("Initializing Wwise Communication.");
            {
                AkCommSettings CommunicationSettings;
                AK::Comm::GetDefaultInitSettings(CommunicationSettings );
                AKPLATFORM::SafeStrCpy(
                    CommunicationSettings.szAppNetworkName,
                    "Visera",
                    AK_COMM_SETTINGS_MAX_STRING_SIZE);
                if (AK::Comm::Init(CommunicationSettings) != AK_Success)
                { LOG_FATAL("Failed to initialize music communication!"); }
            }
#endif // AK_OPTIMIZED

            LOG_TRACE("Initializing Wwise Sound Engine.");
            {
                auto InitSettings         = AkInitSettings{};
                AK::SoundEngine::GetDefaultInitSettings(InitSettings);

                auto PlatformInitSettings = AkPlatformInitSettings{};
                AK::SoundEngine::GetDefaultPlatformInitSettings(PlatformInitSettings);

                auto Result = AK::SoundEngine::Init(&InitSettings, &PlatformInitSettings);
                if (Result != AKRESULT::AK_Success)
                { LOG_FATAL("Failed to initialize Wwise Sound Engine (error:{})!", Int32(Result)); }
            }
            // Set Monitor (Looger)
            {
#if !defined(VISERA_RELEASE_MODE)
                auto ErrorLevel = AK::Monitor::ErrorLevel_All;
#else
                auto ErrorLevel = AK::Monitor::ErrorLevel_Error;
#endif
                AK::Monitor::SetLocalOutput(ErrorLevel,
                [](AK::Monitor::ErrorCode  I_ErrorCode,
                                const AkOSChar*             I_ErrorMessage,
                                AK::Monitor::ErrorLevel     I_ErrorLevel,
                                AkPlayingID                 I_PlayingID,
                                AkGameObjectID              I_GameObjectID)
                {
                    switch (I_ErrorLevel)
                    {
                    case AK::Monitor::ErrorLevel::ErrorLevel_Message:
                        LOG_DEBUG("Wwise: {}", FText::ToUTF8(I_ErrorMessage));
                        break;
                    case AK::Monitor::ErrorLevel::ErrorLevel_Error:
                        LOG_ERROR("Wwise: {} (error: {}).", FText::ToUTF8(I_ErrorMessage), Int32(I_ErrorCode));
                        break;
                    default: LOG_FATAL("Wwise:Unknown Message!");
                    }
                });
            }
        }

        ~FWwiseAudioEngine()
        {
            if (AK::SoundEngine::IsInitialized())
            {
                LOG_TRACE("Terminating Wwise Sound Engine.");
                if (AK::SoundEngine::ClearBanks() != AK_Success)
                { LOG_ERROR("Failed to clear all banks!"); }
                AK::SoundEngine::Term();
            }

#if !defined(AK_OPTIMIZED)
            LOG_TRACE("Terminating Wwise Communication.");
            AK::Comm::Term();
#endif // AK_OPTIMIZED

            if (DeviceID != AK_INVALID_DEVICE_ID)
            {
                LOG_TRACE("Destroying Wwise Streaming Device.");
                AK::StreamMgr::DestroyDevice(DeviceID);
                DeviceID = AK_INVALID_DEVICE_ID;
            }

            if (AK::IAkStreamMgr::Get())
            {
                LOG_TRACE("Terminating Wwise Stream Manager.");

                IO.Terminate();

                AK::IAkStreamMgr::Get()->Destroy();
            }

            if (AK::MemoryMgr::IsInitialized())
            {
                LOG_TRACE("Terminating Wwise Memory Manager.");
                AK::MemoryMgr::Term();
            }
        }
    };

    void FWwiseAudioEngine::
    Tick(Float I_Seconds)
    {
        static Float Gap = 1.0 / 60.0;
        Gap -= I_Seconds;
        if (Gap < 0.0f)
        {
            AK::SoundEngine::RenderAudio();
            Gap = 1.0 / 60.0;
        }
    }
}