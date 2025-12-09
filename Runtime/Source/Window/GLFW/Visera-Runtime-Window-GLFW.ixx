module;
#include <Visera-Runtime.hpp>
#if !defined(VISERA_OFFSCREEN_MODE)
#include <GLFW/glfw3.h>
#endif
export module Visera.Runtime.Window.GLFW;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Window.Interface;
import Visera.Runtime.Input;
import Visera.Runtime.Platform;
import Visera.Core.Log;
#if defined(VISERA_ON_WINDOWS_SYSTEM)
import Visera.Core.Types.Path;
import Visera.Runtime.Media.Image;
#endif

namespace Visera
{
#if !defined(VISERA_OFFSCREEN_MODE)
    export class VISERA_RUNTIME_API FGLFWWindow : public IWindow
    {
    public:
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

        FGLFWWindow();
        ~FGLFWWindow() override;

    private:
        GLFWwindow* Handle = nullptr;

    private:
        static void WindowContentScaleCallback(GLFWwindow* I_Handle, Float I_ScaleX, Float I_ScaleY);
        static void KeyCallback(GLFWwindow* I_Handle, Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods);
        static void MouseButtonCallback(GLFWwindow* I_Handle, Int32 I_Button, Int32 I_Action, Int32 I_Mods);
        static void CursorMoveCallback(GLFWwindow* I_Handle, Double I_PosX, Double I_PosY);
        static void ScrollCallback(GLFWwindow* I_Handle, Double I_OffsetX,  Double I_OffsetY);
    };

    FGLFWWindow::
    FGLFWWindow() : IWindow(EType::GLFW)
    {
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
            "Visera", // The App name is used as the Editor's main window.
#else
            VISERA_APP,
#endif
            nullptr,
            nullptr);
        if (!Handle)
        { LOG_FATAL("Failed to create the GLFW Window!"); return; }

        glfwGetWindowContentScale(Handle, &ScaleX, &ScaleY);

        if (bMaximized)
        {
            LOG_DEBUG("Maximizing the GLFW Window (title: {})", GetTitle());
            glfwMaximizeWindow(Handle);
        }
        else
        {
            SetSize(static_cast<Float>(Width)  / ScaleX,
                    static_cast<Float>(Height) / ScaleY);
        }

#if defined(VISERA_ON_WINDOWS_SYSTEM)
        FImage IconImage{ GPlatform->GetExecutableDirectory() / FPath{"Assets/Image/Visera.png"} };
        VISERA_ASSERT(IconImage.HasAlpha());

        GLFWimage Icon;
        Icon.width  = IconImage.GetWidth();
        Icon.height = IconImage.GetHeight();
        Icon.pixels = IconImage.Access();

        glfwSetWindowIcon(Handle, 1, &Icon);
#endif
        // Make static callbacks able to reach this instance.
        glfwSetWindowUserPointer          (Handle, this);

        glfwSetWindowContentScaleCallback (Handle, FGLFWWindow::WindowContentScaleCallback);
        glfwSetMouseButtonCallback        (Handle, FGLFWWindow::MouseButtonCallback);
        glfwSetCursorPosCallback          (Handle, FGLFWWindow::CursorMoveCallback);
        glfwSetScrollCallback             (Handle, FGLFWWindow::ScrollCallback);
        glfwSetKeyCallback                (Handle, FGLFWWindow::KeyCallback);

        if (!GInput->GetKeyboard()->OnGetKey.TryBind(
        [this](FKeyboard::EKey I_Key, FKeyboard::EAction* O_Status)
        {
            *O_Status = static_cast<FKeyboard::EAction>
                        (glfwGetKey(Handle, static_cast<Int32>(I_Key)));
        }))
        { LOG_FATAL("Failed to delegate OnGetKey()!"); }

        LOG_DEBUG("Created a new GLFW Window (title:{}, extent:[{},{}], scales:[{},{}])",
                  GetTitle(), Width, Height, ScaleX, ScaleY);
    }

    FGLFWWindow::
    ~FGLFWWindow()
    {
        LOG_TRACE("Terminating GLFW Window.");
        if (Handle)
        {
            // Prevent callbacks from touching destroyed engine singletons during teardown.
            glfwSetMouseButtonCallback       (Handle, nullptr);
            glfwSetCursorPosCallback         (Handle, nullptr);
            glfwSetScrollCallback            (Handle, nullptr);
            glfwSetKeyCallback               (Handle, nullptr);
            glfwSetWindowContentScaleCallback(Handle, nullptr);
            glfwSetWindowUserPointer         (Handle, nullptr);

            glfwDestroyWindow(Handle);
            Handle = nullptr;
        }
        glfwTerminate();
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
    WindowContentScaleCallback(GLFWwindow* I_Handle, Float I_ScaleX, Float I_ScaleY)
    {
        if (auto* Self = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Handle)))
        {
            Self->ScaleX = I_ScaleX;
            Self->ScaleY = I_ScaleY;
        }
    }

    void FGLFWWindow::
    KeyCallback(GLFWwindow* I_Handle, Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods)
    {
        const auto Key = static_cast<FKeyboard::EKey>(I_Key);
        switch (I_Action)
        {
        case GLFW_RELEASE: return GInput->GetKeyboard()->OnReleased.Broadcast(Key);
        case GLFW_PRESS:   return GInput->GetKeyboard()->OnPressed.Broadcast(Key);
        case GLFW_REPEAT:  return GInput->GetKeyboard()->OnHeld.Broadcast(Key);
        default: LOG_ERROR("Unhandled key action ({})!", I_Action);
        }
    }

    void FGLFWWindow::
    MouseButtonCallback(GLFWwindow* I_Handle, Int32 I_Button, Int32 I_Action, Int32 I_Mods)
    {
        const auto Button = static_cast<FMouse::EButton>(I_Button);
        switch (I_Action)
        {
        case GLFW_RELEASE: return GInput->GetMouse()->OnReleased.Broadcast(Button);
        case GLFW_PRESS:   return GInput->GetMouse()->OnPressed.Broadcast(Button);
        case GLFW_REPEAT:  return GInput->GetMouse()->OnHeld.Broadcast(Button);
        default: LOG_ERROR("Unhandled mouse action ({})!", I_Action);
        }
    }

    void FGLFWWindow::
    CursorMoveCallback(GLFWwindow* I_Handle, Double I_PosX, Double I_PosY)
    {
        if (auto* Self = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Handle)))
        {
            GInput->GetMouse()->OnCursorMoved.Broadcast(I_PosX * Self->ScaleX, I_PosY * Self->ScaleY);
        }
    }

    void FGLFWWindow::
    ScrollCallback(GLFWwindow* I_Handle, Double I_OffsetX,  Double I_OffsetY)
    {
        GInput->GetMouse()->OnScrolled.Broadcast(I_OffsetX, I_OffsetY);
    }
#endif
}
