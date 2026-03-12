module;
#include <Visera-Platform.hpp>
export module Visera.Platform.MacOS.Window;
#define VISERA_MODULE_NAME "Platform.MacOS"
import Visera.Platform.GLFW.Window;
import Visera.Core.Types.Text;

export namespace Visera
{
    class VISERA_PLATFORM_API FMacOSWindow : public FGLFWWindow
    {
    public:
        void
        SetIcon(const FIconSet& I_IconSet) override
        { /* MacOS regular windows do not have icons */ }

        FMacOSWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height)
        : FGLFWWindow(I_Title, I_Width, I_Height)
        {

        }
    };
}