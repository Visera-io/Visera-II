module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Window.Interface;
import Visera.Runtime.Window.Null;
import Visera.Runtime.Window.GLFW;

namespace Visera
{
    export using EWindowType = IWindow::EType;
    export inline VISERA_RUNTIME_API TUniquePtr<IWindow>
#if !defined(VISERA_OFFSCREEN_MODE)
    GWindow = MakeUnique<FGLFWWindow>();
#else
    GWindow = MakeUnique<FNullWindow>();
#endif
}
