module;
#include <Visera-Engine.hpp>
#include <GLFW/glfw3.h>
export module Visera.Engine.Platform.Window;
#define VISERA_MODULE_NAME "Engine.Platform"
import Visera.Engine.Object;
import Visera.Core.Types.Text;

export namespace Visera
{
    class VISERA_ENGINE_API VWindow : public VObject
    {
    public:
        [[nodiscard]] const FText&
        GetTitle() const { return Title;}

        VWindow() : VObject() {}

    private:
        FText       Title  = TEXT("Visera");
        GLFWwindow* Handle = nullptr;
    };


}
