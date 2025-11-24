module;
#include <Visera-Engine.hpp>
export module Visera.Engine.Event;
#define VISERA_MODULE_NAME "Engine.Event"
import Visera.Core.Global;
import Visera.Core.Delegate;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_ENGINE_API FEvent : public IGlobalSingleton<FEvent>
    {
    public:
        TMulticastDelegate<> OnFrameBegin;
        TMulticastDelegate<> OnFrameEnd;

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
