module;
#include <Visera-Runtime.hpp>
#include "AK/SoundEngine/Common/AkConstants.h"
#include "AK/SoundEngine/Common/AkSoundEngine.h"
#include "AK/SoundEngine/Common/AkTypedefs.h"
export module Visera.Runtime.Audio;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Runtime.Audio.Null;
import Visera.Runtime.Audio.Wwise;
import Visera.Runtime.AssetHub.Sound;
import Visera.Core.Types.Name;
import Visera.Core.Log;

namespace Visera
{
    export using EAudioEngine = IAudioEngine::EType;

    class VISERA_RUNTIME_API FAudio : IGlobalSingleton
    {
    public:
        using FToken   = AkGameObjectID;
        using FEventID = AkPlayingID;

        void inline
        Tick() const { Sleep(1.0 / 60.0); LOG_DEBUG("Tick"); Engine->Tick(1); }
        [[nodiscard]] inline FToken
        Register(TSharedPtr<FSound> I_Sound);
        [[nodiscard]] inline FEventID
        PostEvent(FStringView I_Event, FToken I_Token);

    private:
        TUniquePtr<IAudioEngine> Engine;
        TMap<FName, FToken>      Playlist;

    public:
        FAudio() : IGlobalSingleton{"Audio"} {}
        void
        Bootstrap() override;
        void
        Terminate() override;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FAudio>
    GAudio = MakeUnique<FAudio>();

    void FAudio::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Audio.");

        Engine = MakeUnique<FWwiseAudioEngine>();

        Status = EStatus::Bootstrapped;
    }

    void FAudio::
    Terminate()
    {
        LOG_TRACE("Terminating Audio.");

        Engine.reset();

        Status = EStatus::Terminated;
    }

    FAudio::FToken FAudio::
    Register(TSharedPtr<FSound> I_Sound)
    {
        static UInt64 UUID{0};

        auto& Token = Playlist[I_Sound->GetName()];

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
        LOG_DEBUG("Posting event {}", I_Event);

        auto EventID = AK::SoundEngine::PostEvent(I_Event.data(), I_Token);

        if (AK_INVALID_PLAYING_ID == EventID)
        { LOG_ERROR("Failed to post event {}!", I_Event); }

        return EventID;
    }
}
