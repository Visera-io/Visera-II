module;
#include <Visera-Audio.hpp>
#include <AK/SoundEngine/Common/AkTypes.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkMemoryMgrModule.h>
#include <AK/SoundEngine/Common/AkStreamMgrModule.h>
#include <AK/Tools/Common/AkPlatformFuncs.h>
#include <string>
#if !defined(AK_OPTIMIZED)
#include <AK/Comm/AkCommunication.h>
#endif
export module Visera.Runtime.Audio.Wwise;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Runtime.Audio.Wwise.IO;
import Visera.Core.Containers.Map;
import Visera.Core.Types.Text;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Log;
import Visera.Platform;

namespace Visera
{
    export class VISERA_RUNTIME_API FWwiseAudioEngine : public IAudioEngine
    {
    public:
        void
        RenderAudio() override;
        Bool
        RegisterGameObject(FObjectID I_GameObjectID, FStringView I_DebugName) override;
        Bool
        UnregisterGameObject(FObjectID I_GameObjectID) override;
        Bool
        UnregisterAllGameObjects() override;
        [[nodiscard]] FPlayingID
        PostEvent(FEventID I_EventID, FObjectID I_GameObjectID) override;
        Bool
        SetRTPC(FRTPCID I_RTPCID, FObjectID I_GameObjectID, Float I_Value) override;
        Bool
        SetPosition(FObjectID I_GameObjectID, Float I_X, Float I_Y, Float I_Z) override;
        Bool
        SetDefaultListeners(FObjectID I_ListenerID) override;
        Bool
        InitializeBanks(const FPath& I_BasePath, FStringView I_InitBankName, FStringView I_MainBankName) override;
        [[nodiscard]] FEventID
        GetEventID(FStringView I_EventName) override;
        [[nodiscard]] FRTPCID
        GetRTPCID(FStringView I_RTPCName) override;
        [[nodiscard]] inline const auto
        GetStreamManager() const { return AK::IAkStreamMgr::Get(); }

    private:
        FWwiseIO   IO;
        AkDeviceID DeviceID {AK_INVALID_DEVICE_ID};
        UInt32     CommandQueueSize {1024 * 1024};
        TMap<FString, FEventID> EventIDCache {};
        TMap<FString, FRTPCID>  RTPCIDCache {};

    public:
        explicit FWwiseAudioEngine(UInt32 I_CommandQueueSize = 1024 * 1024)
            : IAudioEngine{EType::Wwise}
            , CommandQueueSize{I_CommandQueueSize}
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
                InitSettings.uCommandQueueSize = CommandQueueSize;

                auto PlatformInitSettings = AkPlatformInitSettings{};
                AK::SoundEngine::GetDefaultPlatformInitSettings(PlatformInitSettings);

                auto Result = AK::SoundEngine::Init(&InitSettings, &PlatformInitSettings);
                if (Result != AKRESULT::AK_Success)
                { LOG_FATAL("Failed to initialize Wwise Sound Engine (error:{})!", Int32(Result)); }
                LOG_DEBUG("Wwise command queue size: {} bytes.", InitSettings.uCommandQueueSize);
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
                        LOG_DEBUG("Wwise: {}", FPlatformPath(I_ErrorMessage).ToPath());
                        break;
                    case AK::Monitor::ErrorLevel::ErrorLevel_Error:
                        LOG_ERROR("Wwise: {} (error: {}).", FPlatformPath(I_ErrorMessage).ToPath(), Int32(I_ErrorCode));
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
    RenderAudio()
    {
        AK::SoundEngine::RenderAudio();
    }

    Bool FWwiseAudioEngine::
    RegisterGameObject(FObjectID I_GameObjectID, FStringView I_DebugName)
    {
        return AK::SoundEngine::RegisterGameObj(static_cast<AkGameObjectID>(I_GameObjectID), I_DebugName.Data()) == AK_Success;
    }

    Bool FWwiseAudioEngine::
    UnregisterGameObject(FObjectID I_GameObjectID)
    {
        return AK::SoundEngine::UnregisterGameObj(static_cast<AkGameObjectID>(I_GameObjectID)) == AK_Success;
    }

    Bool FWwiseAudioEngine::
    UnregisterAllGameObjects()
    {
        return AK::SoundEngine::UnregisterAllGameObj() == AK_Success;
    }

    IAudioEngine::FPlayingID FWwiseAudioEngine::
    PostEvent(FEventID I_EventID, FObjectID I_GameObjectID)
    {
        return static_cast<FPlayingID>(AK::SoundEngine::PostEvent(static_cast<AkUniqueID>(I_EventID), static_cast<AkGameObjectID>(I_GameObjectID)));
    }

    Bool FWwiseAudioEngine::
    SetRTPC(FRTPCID I_RTPCID, FObjectID I_GameObjectID, Float I_Value)
    {
        return AK::SoundEngine::SetRTPCValue(static_cast<AkRtpcID>(I_RTPCID), I_Value, static_cast<AkGameObjectID>(I_GameObjectID)) == AK_Success;
    }

    Bool FWwiseAudioEngine::
    SetPosition(FObjectID I_GameObjectID, Float I_X, Float I_Y, Float I_Z)
    {
        AkSoundPosition Position{};
        Position.SetPosition(I_X, I_Y, I_Z);
        Position.SetOrientation(0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f);
        return AK::SoundEngine::SetPosition(static_cast<AkGameObjectID>(I_GameObjectID), Position) == AK_Success;
    }

    Bool FWwiseAudioEngine::
    SetDefaultListeners(FObjectID I_ListenerID)
    {
        AkGameObjectID ListenerID = static_cast<AkGameObjectID>(I_ListenerID);
        return AK::SoundEngine::SetDefaultListeners(&ListenerID, 1) == AK_Success;
    }

    Bool FWwiseAudioEngine::
    InitializeBanks(const FPath& I_BasePath, FStringView I_InitBankName, FStringView I_MainBankName)
    {
        const FPlatformPath BasePlatformPath = FPlatform::MakePlatformPath(I_BasePath);
        const std::wstring_view BaseWsv{BasePlatformPath};
        if (!BaseWsv.empty() && IO.AddBasePath(reinterpret_cast<const AkOSChar*>(BaseWsv.data())) != AK_Success)
        { LOG_WARN("Failed to add bank base path: {}", I_BasePath); }

        AkBankID InitBankID = AK_INVALID_BANK_ID;
        AkBankID MainBankID = AK_INVALID_BANK_ID;

        const FPlatformPath InitBankPlatformPath = FPlatform::MakePlatformPath(FPath{FString{I_InitBankName}});
        const std::wstring_view InitWsv{InitBankPlatformPath};
        if (InitWsv.empty() || AK::SoundEngine::LoadBank(reinterpret_cast<const AkOSChar*>(InitWsv.data()), InitBankID) != AK_Success)
        { LOG_WARN("Failed to load Init bank ({}). Events may fail.", I_InitBankName); return False; }

        const FPlatformPath MainBankPlatformPath = FPlatform::MakePlatformPath(FPath{FString{I_MainBankName}});
        const std::wstring_view MainWsv{MainBankPlatformPath};
        if (MainWsv.empty() || AK::SoundEngine::LoadBank(reinterpret_cast<const AkOSChar*>(MainWsv.data()), MainBankID) != AK_Success)
        { LOG_ERROR("Failed to load Main bank ({}). BGM and SFX will not play.", I_MainBankName); return False; }

        return True;
    }

    IAudioEngine::FEventID FWwiseAudioEngine::
    GetEventID(FStringView I_EventName)
    {
        const FString Key{I_EventName};
        const auto Iter = EventIDCache.Find(Key);
        if (Iter != EventIDCache.end()) { return Iter->second; }
        const FString Name{I_EventName};
        const auto ID = static_cast<FEventID>(AK::SoundEngine::GetIDFromString(Name.Data()));
        EventIDCache.InsertOrAssign(Key, ID);
        return ID;
    }

    IAudioEngine::FRTPCID FWwiseAudioEngine::
    GetRTPCID(FStringView I_RTPCName)
    {
        const FString Key{I_RTPCName};
        const auto Iter = RTPCIDCache.Find(Key);
        if (Iter != RTPCIDCache.end()) { return Iter->second; }
        const FString Name{I_RTPCName};
        const auto ID = static_cast<FRTPCID>(AK::SoundEngine::GetIDFromString(Name.Data()));
        RTPCIDCache.InsertOrAssign(Key, ID);
        return ID;
    }
}