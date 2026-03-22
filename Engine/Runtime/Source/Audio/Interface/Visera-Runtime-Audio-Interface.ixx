module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio.Interface;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Core.Log;
import Visera.Core.Types.Path;
import Visera.Core.Types.String;

namespace Visera
{
    export class VISERA_RUNTIME_API IAudioEngine
    {
    public:
        using FObjectID  = UInt64;
        using FEventID   = UInt32;
        using FRTPCID    = UInt32;
        using FPlayingID = UInt32;

        static constexpr FObjectID  InvalidObjectID  = 0;
        static constexpr FEventID   InvalidEventID   = 0;
        static constexpr FRTPCID    InvalidRTPCID    = 0;
        static constexpr FPlayingID InvalidPlayingID = 0;

        enum class EType
        {
            Unknown,
            Null,
            Wwise,
        };
        [[nodiscard]] inline EType
        GetType() const { return Type; }
        virtual void
        RenderAudio() = 0;
        virtual Bool
        RegisterGameObject(FObjectID I_GameObjectID, FStringView I_DebugName) = 0;
        virtual Bool
        UnregisterGameObject(FObjectID I_GameObjectID) = 0;
        virtual Bool
        UnregisterAllGameObjects() = 0;
        [[nodiscard]] virtual FPlayingID
        PostEvent(FEventID I_EventID, FObjectID I_GameObjectID) = 0;
        virtual Bool
        SetRTPC(FRTPCID I_RTPCID, FObjectID I_GameObjectID, Float I_Value) = 0;
        virtual Bool
        SetPosition(FObjectID I_GameObjectID, Float I_X, Float I_Y, Float I_Z) = 0;
        virtual Bool
        SetDefaultListeners(FObjectID I_ListenerID) = 0;
        /** Add Wwise file resolver base path once (resolved OS path). */
        virtual Bool
        MountSoundBankBase(const FPath& I_ResolvedBasePath) = 0;
        /** Load one .bnk under the mounted base (relative file name, e.g. init.bnk). */
        virtual Bool
        LoadSoundBankFile(FStringView I_RelativeFileName) = 0;
        [[nodiscard]] virtual FEventID
        GetEventID(FStringView I_EventName) = 0;
        [[nodiscard]] virtual FRTPCID
        GetRTPCID(FStringView I_RTPCName) = 0;

    private:
        EType Type {EType::Unknown};

    public:
        explicit IAudioEngine() = delete;
        explicit IAudioEngine(EType I_Type) : Type{I_Type} {}
        virtual ~IAudioEngine() = default;
    };
}
