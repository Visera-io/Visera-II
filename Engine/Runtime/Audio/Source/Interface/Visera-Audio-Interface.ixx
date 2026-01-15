module;
#include <Visera-Audio.hpp>
export module Visera.Audio.Interface;
#define VISERA_MODULE_NAME "Audio.Interface"
import Visera.Global.Log;

namespace Visera
{
    export class VISERA_AUDIO_API IAudioEngine
    {
    public:
        enum class EType
        {
            Unknown,
            Null,
            Wwise,
        };
        [[nodiscard]] inline EType
        GetType() const { return Type; }
        virtual void
        Tick(Float I_Seconds) = 0;

    private:
        EType Type {EType::Unknown};

    public:
        explicit IAudioEngine() = delete;
        explicit IAudioEngine(EType I_Type) : Type{I_Type} {}
        virtual ~IAudioEngine() = default;
    };
}
