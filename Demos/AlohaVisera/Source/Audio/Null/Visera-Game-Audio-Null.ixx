module;
#include <Visera-Game.hpp>
export module Visera.Game.Audio.Null;
#define VISERA_MODULE_NAME "Game.Audio"
import Visera.Game.Audio.Interface;
import Visera.Runtime.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FNullAudioEngine : public IAudioEngine
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
