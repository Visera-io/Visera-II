module;
#include <Visera-Runtime.hpp>
#include <GLFW/glfw3.h>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Window.Interface;
import Visera.Runtime.Window.GLFW;

namespace Visera
{
    export using EWindowType = IWindow::EType;

    export inline VISERA_RUNTIME_API TUniquePtr<IWindow>
    GWindow = MakeUnique<FGLFWWindow>();
}
