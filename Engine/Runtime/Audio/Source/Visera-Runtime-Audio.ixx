module;
#include <Visera-Audio.hpp>
#include <AK/SoundEngine/Common/AkConstants.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkTypedefs.h>
export module Visera.Runtime.Audio;
#define VISERA_MODULE_NAME "Runtime.Audi"
import Visera.Runtime.Audio.Interface;
import Visera.Runtime.Audio.Null;
import Visera.Runtime.Audio.Wwise;
//import Visera.Runtime.Game.AssetHub.Sound;
import Visera.Core.Containers.Map;
import Visera.Core.Types.String;
import Visera.Core.OS.Time;
import Visera.Runtime.Global;

export namespace Visera
{
    using EAudioEngine = IAudioEngine::EType;

    class VISERA_RUNTIME_API FAudio : public IGlobalService
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
        IAudioEngine*       Engine   {nullptr};
        TMap<FName, FToken> Playlist;

    public:
        FAudio(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                Engine = new FWwiseAudioEngine();

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
                delete Engine;
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
        if (AK_Success == AK::SoundEngine::RegisterGameObj(UUID, FName::FetchNameString(I_Sound->GetName()).Data()))
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

        auto EventID = AK::SoundEngine::PostEvent(I_Event.Data(), I_Token);

        if (AK_INVALID_PLAYING_ID == EventID)
        { LOG_ERROR("Failed to post event {}!", I_Event); }

        return EventID;
    }*/
}
