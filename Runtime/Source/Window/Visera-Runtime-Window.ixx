module;
#include <Visera-Runtime.hpp>
#include <GLFW/glfw3.h>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Core.Types.Text;
import Visera.Core.Log;

namespace Visera
{

    class VISERA_RUNTIME_API FWindow
    {
    public:
        [[nodiscard]] static GLFWmonitor*
        GetPrimaryMonitor() { return glfwGetPrimaryMonitor(); }
        [[nodiscard]] static const GLFWvidmode*
        GetPrimaryMonitorVideoMode() { return glfwGetVideoMode(GetPrimaryMonitor()); }

        [[nodiscard]] inline Bool
        ShouldClose() const { return glfwWindowShouldClose(Handle); }
        inline void
        PollEvents() const { glfwPollEvents(); }

        [[nodiscard]] const FText&
        GetTitle() const { return Title; }

        void inline
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) { glfwSetWindowSize(Handle, I_NewWidth, I_NewHeight); Width = I_NewWidth; Height = I_NewHeight; }
        void inline
        SetPosition(Int32 I_X, Int32 I_Y) const { glfwSetWindowPos(Handle, I_X, I_Y); }

        void inline
        Bootstrap();
        void inline
        Terminate();

    private:
        FText       Title  = TEXT("Visera");
        GLFWwindow* Handle = nullptr;
        Int32       Width{0},     Height{0};
        Float       ScaleX{1.0f}, ScaleY{1.0f};
        Bool        bMaximized{False};
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FWindow>
    GWindow = MakeUnique<FWindow>();

    void FWindow::
    Bootstrap()
    {
        //Init GLFW
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,	GLFW_FALSE);

        Width = 1920;
        Height = 1080;

        //Create Window
        Handle = glfwCreateWindow(
            Width, Height,
            static_cast<const char*>(Title),
            nullptr,
            nullptr);
        if (!Handle)
        { LOG_FATAL("Failed to create the window!"); }

        if (!glfwVulkanSupported())
        { LOG_WARN("Vulkan is NOT Supported!"); }

        // Set Window Position
        const GLFWvidmode* VidMode = GetPrimaryMonitorVideoMode();
#if defined(VISERA_ON_APPLE_SYSTEM)
        //SetPosition(400, 200);
#else
        SetPosition(
            (VidMode->width    -   Width ) >> 1,	// Mid
            (VidMode->height   -   Height) >> 1);	// Mid
#endif

        glfwGetWindowContentScale(Handle, &ScaleX, &ScaleY);

        if (bMaximized)
        {
            LOG_DEBUG("Maximizing the window (title: {})", Title);
            glfwMaximizeWindow(Handle);
        }
        else
        {
            SetSize(static_cast<Float>(Width)  / ScaleX,
                    static_cast<Float>(Height) / ScaleY);
        }

        LOG_DEBUG("Created a new window (title:{}, extent:[{},{}], scales:[{},{}])",
            Title, Width, Height, ScaleX, ScaleY);
    }

    void FWindow::
    Terminate()
    {
        glfwDestroyWindow(Handle);
        glfwTerminate();
    }

}
