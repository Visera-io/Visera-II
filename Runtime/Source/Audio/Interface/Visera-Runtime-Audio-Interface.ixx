module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio.Interface;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Core.Log;

namespace Visera
{
    export VISERA_RUNTIME_API class IAudioEngine
    {
    public:
        enum class EType
        {
            Unknown,
            Null,
            Wwise,
        };

        virtual void
        Tick() const = 0;

    private:
        EType Type {EType::Unknown};

    public:
        explicit IAudioEngine() = delete;
        explicit IAudioEngine(EType I_Type) : Type{I_Type}
        {
            const char* AudioEngineName = "Unknown";
            switch (Type)
            {
            case EType::Null:  AudioEngineName = "Null";  break;
            case EType::Wwise: AudioEngineName = "Wwise"; break;
            default: LOG_FATAL("Unknown Audio Engine!");  break;
            }
            LOG_INFO("Audio Engine: {}", AudioEngineName);
        }
        virtual ~IAudioEngine() = default;
    };
}
