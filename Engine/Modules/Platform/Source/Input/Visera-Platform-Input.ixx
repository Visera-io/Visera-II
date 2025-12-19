module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Input;
#define VISERA_MODULE_NAME "Platform.Input"
export import Visera.Platform.Input.Keyboard;
export import Visera.Platform.Input.Mouse;
       import Visera.Core.Global;
       import Visera.Runtime.Log;

namespace Visera
{
    export class VISERA_PLATFORM_API FInput : public IGlobalSingleton<FInput>
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

    export inline VISERA_PLATFORM_API TUniquePtr<FInput>
    GInput = MakeUnique<FInput>();
}