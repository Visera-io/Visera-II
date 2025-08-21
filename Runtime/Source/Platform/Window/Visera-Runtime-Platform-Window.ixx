module;
#include <Visera-Core.hpp>
#include <Visera-Runtime.hpp>
#include <GLFW/glfw3.h>
export module Visera.Runtime.Platform.Window;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Core.Log;
import Visera.Core.Types.Text;

namespace Visera { class GPlatform;}

export namespace Visera
{
    class GWindow
    {
        friend class GPlatform;
    public:


    private:
        FText       Title  = TEXT("Visera");
        GLFWwindow* Handle = nullptr;

    private:
        static void
        Bootstrap();
        static void
        Terminate();
    };

    void GWindow::
    Bootstrap()
    {
        LOG_DEBUG("Bootstrapping Window...");
    }

    void GWindow::
    Terminate()
    {
        LOG_DEBUG("Terminating Window...");
    }

}
