module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio.Null;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FNullAudioEngine : public IAudioEngine
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
