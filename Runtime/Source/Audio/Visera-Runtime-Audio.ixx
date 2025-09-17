module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Runtime.Audio.Wwise;
import Visera.Core.Log;

namespace Visera
{
    export using EAudioEngine = IAudioEngine::EType;

    export VISERA_RUNTIME_API class FAudio : IGlobalSingleton
    {
    public:
        FAudio() : IGlobalSingleton{"Audio"} {}
        void
        Bootstrap() override;
        void
        Terminate() override;

    private:
        TUniquePtr<IAudioEngine> Engine;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FAudio>
    GAudio = MakeUnique<FAudio>();

    void FAudio::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Audio.");

        Engine = MakeUnique<FWwiseAudioEngine>();

        Status = EStatues::Bootstrapped;
    }

    void FAudio::
    Terminate()
    {
        LOG_TRACE("Terminating Audio.");

        Engine.reset();

        Status = EStatues::Terminated;
    }

}
