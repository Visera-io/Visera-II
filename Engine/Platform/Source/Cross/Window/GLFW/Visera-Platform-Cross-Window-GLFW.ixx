module;
#include <Visera-Platform.hpp>
#if !defined(VISERA_OFFSCREEN_MODE)
#include <GLFW/glfw3.h>
#endif
export module Visera.Platform.Cross.Window.GLFW;
#define VISERA_MODULE_NAME "Platform.Cross"
import Visera.Platform.Interface.Window;

export namespace Visera
{
    class VISERA_PLATFORM_API FGLFWWindow : public IPlatformWindow
    {
    public:
        [[nodiscard]] static GLFWmonitor*
        GetPrimaryMonitor();

        [[nodiscard]] void*
        GetHandle() const override { return Handle; }
        [[nodiscard]] Bool
        ShouldClose() const override { return glfwWindowShouldClose(Handle); }
        void
        WaitEvents() const override { glfwWaitEvents(); }
        void
        PollEvents() const override { glfwPollEvents(); }
        void
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) override { glfwSetWindowSize(Handle, I_NewWidth, I_NewHeight); Width = I_NewWidth; Height = I_NewHeight; }
        void
        SetPosition(Int32 I_X, Int32 I_Y) const override { glfwSetWindowPos(Handle, I_X, I_Y); }
        void
        SetTitle(FStringView I_Title) override { glfwSetWindowTitle(Handle, I_Title.Data()); }
        void
        SetIcon(const FIconSet& I_IconSet) override;

        FGLFWWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height);
        ~FGLFWWindow() override;

    private:
        GLFWwindow* Handle = nullptr;
    };

    FGLFWWindow::
    FGLFWWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height)
    : IPlatformWindow(I_Title, I_Width, I_Height)
    {
        glfwSetErrorCallback([](Int32 I_Error, const char* I_Message)
        { ErrorCallback.Invoke(FString::Format("{} (error:{})", I_Message, I_Error)); });

        if (!glfwInit()) { ErrorCallback.Invoke("Failed to initialize GLFW!"); }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,	GLFW_TRUE);

        //Create Window
        Handle = glfwCreateWindow(
            Width, Height, //[TODO] read from config (save the last scale).
            Title.Data(), // The App name is used as the Editor's main window.
            nullptr,
            nullptr);
        if (!Handle)
        { ErrorCallback.Invoke("Failed to create the GLFW Window!"); return; }

        glfwGetWindowContentScale(Handle, &ScaleX, &ScaleY);

        if (bMaximized)
        {
            glfwMaximizeWindow(Handle);
        }
        else
        {
            glfwSetWindowSize(Handle, Width  / ScaleX, Height / ScaleY);
        }

        // Make static callbacks able to reach this instance.
        glfwSetWindowUserPointer          (Handle, this);

        glfwSetWindowContentScaleCallback (Handle, [](GLFWwindow*, Float I_ScaleX, Float I_ScaleY) { WindowContentScaleCallback.Invoke(I_ScaleX, I_ScaleY); });
        glfwSetMouseButtonCallback        (Handle, [](GLFWwindow*, Int32 I_Button, Int32 I_Action, Int32 I_Mods) { MouseButtonCallback.Invoke(I_Button, I_Action, I_Mods); });
        glfwSetCursorPosCallback          (Handle, [](GLFWwindow*, Double I_PosX, Double I_PosY) { CursorMoveCallback.Invoke(I_PosX, I_PosY); });
        glfwSetScrollCallback             (Handle, [](GLFWwindow*, Double I_OffsetX, Double I_OffsetY) { ScrollCallback.Invoke(I_OffsetX, I_OffsetY); });
        glfwSetKeyCallback                (Handle, [](GLFWwindow*, Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods) { KeyboardCallback.Invoke(I_Key, I_ScanCode, I_Action, I_Mods); });
        
        glfwSetFramebufferSizeCallback    (Handle, [](GLFWwindow* I_Window, Int32 I_Width, Int32 I_Height) {
            if (auto* Self = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window)))
            {
                Self->Width  = static_cast<UInt32>(I_Width);
                Self->Height = static_cast<UInt32>(I_Height);
            }
            WindowResizeCallback.Invoke(I_Width, I_Height);
        });
    }

    FGLFWWindow::
    ~FGLFWWindow()
    {
        if (Handle)
        {
            // Prevent callbacks from touching destroyed engine singletons during teardown.
            glfwSetMouseButtonCallback       (Handle, nullptr);
            glfwSetCursorPosCallback         (Handle, nullptr);
            glfwSetScrollCallback            (Handle, nullptr);
            glfwSetKeyCallback               (Handle, nullptr);
            glfwSetFramebufferSizeCallback   (Handle, nullptr);
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
            ErrorCallback.Invoke("Failed to get the primary monitor!");
            // Try to get any available monitor
            Int32 Count{0};
            GLFWmonitor** Monitors = glfwGetMonitors(&Count);
            if (Monitors && Count > 0)
            {
                for (Int32 i = 0; i < Count; i++)
                {
                    ErrorCallback.Invoke(FString::Format("Monitor[{}]: {}",
                        i, glfwGetMonitorName(Monitors[i])));
                }
            }
            else { ErrorCallback.Invoke("No monitors found!"); }
        }
        return PrimaryMonitor;
    }

    void FGLFWWindow::
    SetIcon(const FIconSet& I_IconSet)
    {
        GLFWimage Icons[6]
        {
            { .width = 16, .height = 16, .pixels = const_cast<unsigned char*>(I_IconSet.Icon16x16) },
            { .width = 32, .height = 32, .pixels = const_cast<unsigned char*>(I_IconSet.Icon32x32) },
            { .width = 48, .height = 48, .pixels = const_cast<unsigned char*>(I_IconSet.Icon48x48) },
            { .width = 64, .height = 64, .pixels = const_cast<unsigned char*>(I_IconSet.Icon64x64) },
            { .width = 128, .height = 128, .pixels = const_cast<unsigned char*>(I_IconSet.Icon128x128) },
            { .width = 256, .height = 256, .pixels = const_cast<unsigned char*>(I_IconSet.Icon256x256) },
        };
        glfwSetWindowIcon(Handle, 6, Icons);
    }
}
