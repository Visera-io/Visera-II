module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Input;
#define VISERA_MODULE_NAME "Platform.Input"
export import Visera.Platform.Input.Keyboard;
export import Visera.Platform.Input.Mouse;
       import Visera.Global.Service;
       import Visera.Global.Log;

namespace Visera
{
    export class VISERA_PLATFORM_API FInput : public IGlobalService<FInput>
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
        FInput() : IGlobalService(FName{"Input"}) {}
    };

    export inline VISERA_PLATFORM_API TUniquePtr<FInput>
    GInput = MakeUnique<FInput>();
}