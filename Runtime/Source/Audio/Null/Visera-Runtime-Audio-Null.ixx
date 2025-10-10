module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio.Null;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Core.Log;

namespace Visera
{
    export VISERA_RUNTIME_API class FNullAudioEngine : public IAudioEngine
    {
    public:
        explicit FNullAudioEngine() : IAudioEngine{EType::Wwise}
        {

        }

        ~FNullAudioEngine() override
        {

        }
    };
}
