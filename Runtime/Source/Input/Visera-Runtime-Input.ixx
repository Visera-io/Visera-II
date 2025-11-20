module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Input;
#define VISERA_MODULE_NAME "Runtime.Input"
export import Visera.Runtime.Input.Keyboard;
export import Visera.Runtime.Input.Mouse;
       import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FInput : public IGlobalSingleton<FInput>
    {
    public:
        [[nodiscard]] inline FKeyboard*
        GetKeyboard() { return &Keyboard; }
        [[nodiscard]] inline FMouse*
        GetMouse()    { return &Mouse; }

    private:
        FKeyboard Keyboard;
        FMouse    Mouse;

    public:
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Input.");

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_TRACE("Terminating Input.");

            Status = EStatus::Terminated;
        }

        FInput() : IGlobalSingleton("Input") {}
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FInput>
    GInput = MakeUnique<FInput>();
}