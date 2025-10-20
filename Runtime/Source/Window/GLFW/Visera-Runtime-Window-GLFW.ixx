module;
#include <Visera-Runtime.hpp>
#include <GLFW/glfw3.h>
export module Visera.Runtime.Window.GLFW;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Window.Interface;
import Visera.Core.Log;
#if defined VISERA_ON_WINDOWS_SYSTEM
import Visera.Runtime.AssetHub;
#endif

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
        [[nodiscard]] FStringView
        GetTitle() const override { return glfwGetWindowTitle(Handle); }
        void inline
        SetTitle(FStringView I_Title) override { glfwSetWindowTitle(Handle, I_Title.data()); }

        FGLFWWindow() : IWindow(EType::GLFW) {}

        void inline
        Bootstrap() override;
        void inline
        Terminate() override;

    private:
        GLFWwindow* Handle = nullptr;

    private:
        static void MouseButtonCallback(GLFWwindow* I_Handle, Int32 I_Button, Int32 I_Action, Int32 I_Mods);
        static void MouseCursorCallback(GLFWwindow* I_Handle, Double I_PosX, Double I_PosY);
        static void MouseScrollCallback(GLFWwindow* I_Handle, Double I_OffsetX,  Double I_OffsetY);
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

        //Create Window
        Handle = glfwCreateWindow(
            Width, Height, //[TODO] read from config (save the last scale).
#if !defined(VISERA_RELEASE_MODE)
            reinterpret_cast<const char*>(u8"Visera"),
#else
            reinterpret_cast<const char*>(VISERA_APP),
#endif
            nullptr,
            nullptr);
        if (!Handle)
        { LOG_FATAL("Failed to create the window!"); return; }

        if (bMaximized)
        {
            LOG_DEBUG("Maximizing the window (title: {})", GetTitle());
            glfwMaximizeWindow(Handle);
        }
        else
        {
            SetSize(static_cast<Float>(Width)  / ScaleX,
                    static_cast<Float>(Height) / ScaleY);
        }

#if defined(VISERA_ON_APPLE_SYSTEM)

#else
        auto IconImage = GAssetHub->LoadImage(PATH("Visera.png"));
        VISERA_ASSERT(IconImage->HasAlpha());

        GLFWimage Icon;
        Icon.width  = IconImage->GetWidth();
        Icon.height = IconImage->GetHeight();
        Icon.pixels = IconImage->Access();

        glfwSetWindowIcon(Handle, 1, &Icon);
#endif
        glfwGetWindowContentScale(Handle, &ScaleX, &ScaleY);

        glfwSetMouseButtonCallback(Handle, FGLFWWindow::MouseButtonCallback);
        glfwSetCursorPosCallback(Handle, FGLFWWindow::MouseCursorCallback);
        glfwSetScrollCallback(Handle, FGLFWWindow::MouseScrollCallback);

        LOG_DEBUG("Created a new window (title:{}, extent:[{},{}], scales:[{},{}])",
                  GetTitle(), Width, Height, ScaleX, ScaleY);

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
    void FGLFWWindow::
    MouseButtonCallback(GLFWwindow* I_Handle, Int32 I_Button, Int32 I_Action, Int32 I_Mods)
    {
        const char* actionStr =
            (I_Action == GLFW_PRESS)   ? "Pressed" :
            (I_Action == GLFW_RELEASE) ? "Released" : "Repeated";

        LOG_DEBUG("(WIP) Mouse Button {} {}", I_Button, actionStr);
    }

    void FGLFWWindow::
    MouseCursorCallback(GLFWwindow* I_Handle, Double I_PosX, Double I_PosY)
    {
        Mouse.Cursor.Offset.X = I_PosX - Mouse.Cursor.Position.X;
        Mouse.Cursor.Offset.Y = I_PosY - Mouse.Cursor.Position.Y;
        Mouse.Cursor.Position.X = I_PosX;
        Mouse.Cursor.Position.Y = I_PosY;
    }

    void FGLFWWindow::
    MouseScrollCallback(GLFWwindow* I_Handle, Double I_OffsetX,  Double I_OffsetY)
    {
        LOG_DEBUG("(WIP) Mouse scrolled ({}, {})", I_OffsetX, I_OffsetY);
    }
}
