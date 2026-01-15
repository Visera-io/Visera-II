module;
#include <Visera-Audio.hpp>
export module Visera.Audio.Null;
#define VISERA_MODULE_NAME "Audio.Null"
import Visera.Audio.Interface;
import Visera.Global.Log;

namespace Visera
{
    export class VISERA_AUDIO_API FNullAudioEngine : public IAudioEngine
    {
    public:
        void
        Tick(Float I_Seconds) override {};

    public:
        explicit FNullAudioEngine() : IAudioEngine{EType::Null}
        {

        }

        ~FNullAudioEngine() override
        {

        }
    };
}
