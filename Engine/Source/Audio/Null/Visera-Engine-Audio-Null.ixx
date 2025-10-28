module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Audio.Null;
#define VISERA_MODULE_NAME "Engine.Audio"
import Visera.Engine.Audio.Interface;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FNullAudioEngine : public IAudioEngine
    {
    public:
        void
        Tick(Float I_Seconds) const override {};

    public:
        explicit FNullAudioEngine() : IAudioEngine{EType::Null}
        {

        }

        ~FNullAudioEngine() override
        {

        }
    };
}
