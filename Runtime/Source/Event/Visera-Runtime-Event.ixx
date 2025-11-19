module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Event;
#define VISERA_MODULE_NAME "Runtime.Event"
import Visera.Core.Delegate;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FEvent : public IGlobalSingleton<FEvent>
    {
    public:
        TMulticastDelegate<> OnBeginFrame;
        TMulticastDelegate<> OnEndFrame;

    private:


    public:
        FEvent() : IGlobalSingleton("Event") {}
        void
        Bootstrap() override { LOG_TRACE("Bootstrapping Event."); Status = EStatus::Bootstrapped;}
        void
        Terminate() override { LOG_TRACE("Terminating Event.");   Status = EStatus::Terminated; }
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FEvent>
    GEvent = MakeUnique<FEvent>();
}
