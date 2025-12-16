module;
#include <Visera-Game.hpp>
export module Visera.Game.Event;
#define VISERA_MODULE_NAME "Game.Event"
import Visera.Core.Global;
import Visera.Core.Delegate.Multicast;
import Visera.Runtime.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FEvent : public IGlobalSingleton<FEvent>
    {
    public:
        TMulticastDelegate<> OnFrameBegin;
        TMulticastDelegate<> OnFrameEnd;

        TMulticastDelegate<> OnEngineBootstrap;
        TMulticastDelegate<> OnEngineTerminate;

    public:
        FEvent() : IGlobalSingleton("Event") {}
        void
        Bootstrap() override { LOG_TRACE("Bootstrapping Event."); Status = EStatus::Bootstrapped;}
        void
        Terminate() override { LOG_TRACE("Terminating Event.");   Status = EStatus::Terminated; }
    };

    export inline VISERA_ENGINE_API TUniquePtr<FEvent>
    GEvent = MakeUnique<FEvent>();
}
