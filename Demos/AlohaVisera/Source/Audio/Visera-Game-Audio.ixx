module;
#include <Visera-Game.hpp>
#include <AK/SoundEngine/Common/AkConstants.h>
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/SoundEngine/Common/AkTypedefs.h>
export module Visera.Game.Audio;
#define VISERA_MODULE_NAME "Game.Audio"
import Visera.Game.Audio.Interface;
import Visera.Game.Audio.Null;
import Visera.Game.Audio.Wwise;
import Visera.Game.AssetHub.Sound;
import Visera.Core.Types.Map;
import Visera.Core.OS.Time;
import Visera.Global;

namespace Visera
{
    export using EAudioEngine = IAudioEngine::EType;

    export class VISERA_ENGINE_API FAudio : public IGlobalService<FAudio>
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
        [[nodiscard]] inline FToken
        Register(TSharedRef<FSound> I_Sound);
        [[nodiscard]] inline FEventID
        PostEvent(FStringView I_Event, FToken I_Token);

    private:
        TUniquePtr<IAudioEngine> Engine;
        TMap<FName, FToken>      Playlist;

    public:
        FAudio() : IGlobalService(FName{"Audio"})
        {
            Engine = MakeUnique<FNullAudioEngine>();
        }
    };

    export inline VISERA_ENGINE_API TUniquePtr<FAudio>
    GAudio = MakeUnique<FAudio>();

    /*void FAudio::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Audio.");

        Engine = MakeUnique<FWwiseAudioEngine>();

        switch (Engine->GetType())
        {
            case IAudioEngine::EType::Null : LOG_INFO("Audio Engine: Null.");  break;
            case IAudioEngine::EType::Wwise: LOG_INFO("Audio Engine: Wwise."); break;
            default: LOG_FATAL("Unknown Audio Engine!");  break;
        }
        // Set Default Listeners
        UInt64 MainID{0};
        if (AK_Success != AK::SoundEngine::RegisterGameObj(MainID, "Player"))
        {
            LOG_FATAL("Failed to register Main Listener");
        }
        AK::SoundEngine::SetDefaultListeners(&MainID, 1);

        Status = EStatus::Bootstrapped;
    }

    void FAudio::
    Terminate()
    {
        LOG_TRACE("Terminating Audio.");

        for (auto& [Name, PID] : Playlist)
        {
            if (AK::SoundEngine::UnregisterGameObj(PID) != AK_Success)
            { LOG_ERROR("Failed to unregister {} (id:{})!", Name, PID); }
        }
        Engine.reset();

        Status = EStatus::Terminated;
    }*/

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
    }
}
