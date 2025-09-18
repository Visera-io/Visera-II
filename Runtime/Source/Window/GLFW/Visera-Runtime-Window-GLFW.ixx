module;
#include <Visera-Runtime.hpp>
#include <GLFW/glfw3.h>
export module Visera.Runtime.Window.GLFW;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Window.Interface;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FGLFWWindow : public IWindow
    {
    public:
    //     void inline
    //     Initialize() { if (!glfwInit()) { LOG_FATAL("Failed to initialize GLFW!"); return; }; bInitialized = True; };

        [[nodiscard]] static GLFWmonitor*
        GetPrimaryMonitor();

        [[nodiscard]] inline void*
        GetHandle() const override { return Handle; }
        [[nodiscard]] inline Bool
        ShouldClose() const override { return glfwWindowShouldClose(Handle); }
        inline void
        WaitEvents() const override { glfwWaitEvents(); }
        inline void
        PollEvents() const override { glfwPollEvents(); }
        void inline
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) override { glfwSetWindowSize(Handle, I_NewWidth, I_NewHeight); Width = I_NewWidth; Height = I_NewHeight; }
        void inline
        SetPosition(Int32 I_X, Int32 I_Y) const override { glfwSetWindowPos(Handle, I_X, I_Y); }

        FGLFWWindow() : IWindow(EType::GLFW) {}

        void inline
        Bootstrap() override;
        void inline
        Terminate() override;

    private:
        GLFWwindow* Handle = nullptr;
    };

    void FGLFWWindow::
    Bootstrap()
    {
        LOG_TRACE("Bootstrapping Window.");

        if (!glfwInit())
        { LOG_FATAL("Failed to initialize GLFW!"); }

        glfwSetErrorCallback([](Int32 I_Error, const char* I_Description)
        { LOG_ERROR("GLFW Error {}: {}", I_Error, I_Description); });

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
        { LOG_FATAL("Failed to create the window!"); return; }

#if defined(VISERA_ON_APPLE_SYSTEM)
        //SetPosition(400, 200);
#else

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

        Status = EStatus::Bootstrapped;
    }

    void FGLFWWindow::
    Terminate()
    {
        LOG_TRACE("Terminating Window.");
        glfwDestroyWindow(Handle);
        glfwTerminate();
        Handle = nullptr;

        Status = EStatus::Terminated;
    }

    GLFWmonitor* FGLFWWindow::
    GetPrimaryMonitor()
    {
        auto PrimaryMonitor = glfwGetPrimaryMonitor();
        if (!PrimaryMonitor)
        {
            LOG_ERROR("Failed to get the primary monitor!");
            // Try to get any available monitor
            Int32 Count{0};
            GLFWmonitor** Monitors = glfwGetMonitors(&Count);
            if (Monitors && Count > 0)
            {
                for (Int32 i = 0; i < Count; i++)
                {
                    LOG_INFO("Monitor[{}]: {}", i, glfwGetMonitorName(Monitors[i]));
                }
            }
            else { LOG_ERROR("No monitors found!"); }
        }
        return PrimaryMonitor;
    }

}
