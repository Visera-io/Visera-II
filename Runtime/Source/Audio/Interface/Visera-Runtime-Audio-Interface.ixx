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
            Wwise,
        };
        explicit IAudioEngine() = delete;
        explicit IAudioEngine(EType I_Type) : Type{I_Type}
        { LOG_INFO("Audio Engine: {}", Type == EType::Unknown? "Unknown" : "Wwise"); }
        virtual ~IAudioEngine() = default;

    private:
        EType Type {EType::Unknown};
    };
}
