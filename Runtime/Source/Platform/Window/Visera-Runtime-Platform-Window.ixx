module;
#include <Visera-Runtime.hpp>
#include <GLFW/glfw3.h>
export module Visera.Runtime.Platform.Window;
#define VISERA_MODULE_NAME "Runtime.Platform"
import Visera.Runtime.Object;
import Visera.Core.Types.Text;

export namespace Visera
{
    class VWindow : public VObject
    {
    public:
        VWindow() : VObject() {}

    private:
        FText       Title  = TEXT("Visera");
        GLFWwindow* Handle = nullptr;
    };


}
