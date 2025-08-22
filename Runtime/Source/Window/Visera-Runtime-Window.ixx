module;
#include <Visera-Runtime.hpp>
#include <GLFW/glfw3.h>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Core.Types.Text;

export namespace Visera
{

    class VISERA_RUNTIME_API FWindow
    {
    public:
        [[nodiscard]] const FText&
        GetTitle() const { return Title;}

    private:
        FText       Title  = TEXT("Visera");
        GLFWwindow* Handle = nullptr;
    };

}
