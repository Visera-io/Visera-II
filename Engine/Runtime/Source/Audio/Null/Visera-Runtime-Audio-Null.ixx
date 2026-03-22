module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio.Null;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FNullAudioEngine : public IAudioEngine
    {
    public:
        void
        RenderAudio() override {}
        Bool
        RegisterGameObject(FObjectID I_GameObjectID, FStringView I_DebugName) override
        { return True; }
        Bool
        UnregisterGameObject(FObjectID I_GameObjectID) override
        { return True; }
        Bool
        UnregisterAllGameObjects() override
        { return True; }
        [[nodiscard]] FPlayingID
        PostEvent(FEventID I_EventID, FObjectID I_GameObjectID) override
        { return InvalidPlayingID; }
        Bool
        SetRTPC(FRTPCID I_RTPCID, FObjectID I_GameObjectID, Float I_Value) override
        { return True; }
        Bool
        SetPosition(FObjectID I_GameObjectID, Float I_X, Float I_Y, Float I_Z) override
        { return True; }
        Bool
        SetDefaultListeners(FObjectID I_ListenerID) override
        { return True; }
        Bool
        MountSoundBankBase(const FPath& I_ResolvedBasePath) override
        { return True; }
        Bool
        LoadSoundBankFile(FStringView I_RelativeFileName) override
        { return True; }
        [[nodiscard]] FEventID
        GetEventID(FStringView I_EventName) override
        { return InvalidEventID; }
        [[nodiscard]] FRTPCID
        GetRTPCID(FStringView I_RTPCName) override
        { return InvalidRTPCID; }

    public:
        explicit FNullAudioEngine() : IAudioEngine{EType::Null}
        {

        }

        ~FNullAudioEngine() override
        {

        }
    };
}
