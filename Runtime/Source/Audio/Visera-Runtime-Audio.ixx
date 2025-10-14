module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Audio;
#define VISERA_MODULE_NAME "Runtime.Audio"
import Visera.Runtime.Audio.Interface;
import Visera.Runtime.Audio.Null;
import Visera.Runtime.Audio.Wwise;
import Visera.Core.Log;

namespace Visera
{
    export using EAudioEngine = IAudioEngine::EType;

    export VISERA_RUNTIME_API class FAudio : IGlobalSingleton
    {
    public:
        void inline
        Tick() const { Engine->Tick(); }

    private:
        TUniquePtr<IAudioEngine> Engine;

    public:
        FAudio() : IGlobalSingleton{"Audio"} {}
        void
        Bootstrap() override;
        void
        Terminate() override;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FAudio>
    GAudio = MakeUnique<FAudio>();

    void FAudio::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Audio.");

        Engine = MakeUnique<FWwiseAudioEngine>();

        Status = EStatus::Bootstrapped;
    }

    void FAudio::
    Terminate()
    {
        LOG_TRACE("Terminating Audio.");

        Engine.reset();

        Status = EStatus::Terminated;
    }

}
