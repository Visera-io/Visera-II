module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Input;
#define VISERA_MODULE_NAME "Platform.Input"
export import Visera.Platform.Input.Keyboard;
export import Visera.Platform.Input.Mouse;
       import Visera.Global;

namespace Visera
{
    export class VISERA_PLATFORM_API FInput : public IGlobalService
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
        FInput() : IGlobalService(EName::Input) {}
    };

    export inline VISERA_PLATFORM_API TUniquePtr<FInput>
    GInput = MakeUnique<FInput>();
}