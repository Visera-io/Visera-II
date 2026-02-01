module;
#include <Visera-Audio.hpp>
#include <AK/SoundEngine/Common/AkConstants.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkTypedefs.h>
export module Visera.Audio;
#define VISERA_MODULE_NAME "Audio"
import Visera.Audio.Interface;
import Visera.Audio.Null;
import Visera.Audio.Wwise;
//import Visera.Game.AssetHub.Sound;
import Visera.Core.Types.Map;
import Visera.Core.OS.Time;
import Visera.Global;

export namespace Visera
{
    using EAudioEngine = IAudioEngine::EType;

    class VISERA_AUDIO_API FAudio : public IGlobalService
    {
    public:
        using FToken   = AkGameObjectID;
        using FEventID = AkPlayingID;

        void inline
        Tick()
        {
            static FSystemClock Timer{};
            static UInt64 Time{0};
            Time = Timer.Elapsed().Milliseconds();
            if (Time >= 16)
            {
                Engine->Tick(1);
                Timer.Reset();
            }
        }
        //[[nodiscard]] inline FToken
        //Register(TSharedRef<FSound> I_Sound);
        //[[nodiscard]] inline FEventID
        //PostEvent(FStringView I_Event, FToken I_Token);

    private:
        TUniquePtr<IAudioEngine> Engine;
        TMap<FName, FToken>      Playlist;

    public:
        FAudio() : IGlobalService(EName::Audio)
        {
            Dependencies =
            {
                EName::Platform
            };

            if (!OnBootstrap.TryBind([this]
            {
                Engine = MakeUnique<FWwiseAudioEngine>();

                DEBUG_ONLY_FIELD
                (
                switch (Engine->GetType())
                {
                    case IAudioEngine::EType::Null : LOG_TRACE("Audio Engine: Null.");  break;
                    case IAudioEngine::EType::Wwise: LOG_TRACE("Audio Engine: Wwise."); break;
                    default: LOG_FATAL("Unknown Audio Engine!");  break;
                }
                );
                // Set Default Listeners
                UInt64 MainID{0};
                if (AK_Success != AK::SoundEngine::RegisterGameObj(MainID, "Player"))
                {
                    LOG_FATAL("Failed to register Main Listener");
                }
                AK::SoundEngine::SetDefaultListeners(&MainID, 1);

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                for (auto& [Name, PID] : Playlist)
                {
                    if (AK::SoundEngine::UnregisterGameObj(PID) != AK_Success)
                    { LOG_ERROR("Failed to unregister {} (id:{})!", Name, PID); }
                }
                Engine.reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };
    /*
    FAudio::FToken FAudio::
    Register(TSharedRef<FSound> I_Sound)
    {
        auto& Token = Playlist[I_Sound->GetName()];

        static UInt64 UUID{1};
        if (AK_Success == AK::SoundEngine::RegisterGameObj(UUID, FName::FetchNameString(I_Sound->GetName()).data()))
        {
            LOG_DEBUG("Registered sound {} (token:{}).", I_Sound->GetPath(), UUID);
            Token = UUID++;
            return Token;
        }
        LOG_ERROR("Failed to register sound {}!", I_Sound->GetPath());
        return AK_INVALID_GAME_OBJECT;
    }

    FAudio::FEventID FAudio::
    PostEvent(FStringView I_Event, FToken I_Token)
    {
        LOG_TRACE("Posting event {}", I_Event);

        auto EventID = AK::SoundEngine::PostEvent(I_Event.data(), I_Token);

        if (AK_INVALID_PLAYING_ID == EventID)
        { LOG_ERROR("Failed to post event {}!", I_Event); }

        return EventID;
    }*/
}
