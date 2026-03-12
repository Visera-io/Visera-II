module;
#include <Visera-Platform.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
export module Visera.Platform.GLFW.Window;
#define VISERA_MODULE_NAME "Platform.GLFW"
import Visera.Platform.Interface.Window;
import Visera.Core.OS.Thread;
import Visera.Core.Log;
import Visera.Core.Containers.Array;
import Visera.Core.Types.Text;

export namespace Visera
{
    class VISERA_PLATFORM_API FGLFWWindow : public IPlatformWindow
    {
    public:
        /** Currently focused GLFW window (for input). Set by focus callback; cleared when window loses focus or is destroyed. */
        [[nodiscard]] static IPlatformWindow*
        GetFocusedPlatformWindow();

        [[nodiscard]] void*
        GetHandle() const override { return Handle; }
        [[nodiscard]] Bool
        ShouldClose() const override { VISERA_ASSERT(FThread::IsMainThread()); return glfwWindowShouldClose(Handle); }
        void
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) override { VISERA_ASSERT(FThread::IsMainThread()); glfwSetWindowSize(Handle, I_NewWidth, I_NewHeight); Width = I_NewWidth; Height = I_NewHeight; }
        void
        SetPosition(Int32 I_X, Int32 I_Y) const override { VISERA_ASSERT(FThread::IsMainThread()); glfwSetWindowPos(Handle, I_X, I_Y); }
        void
        SetTitle(const FText& I_Title) override { VISERA_ASSERT(FThread::IsMainThread()); Title = I_Title; glfwSetWindowTitle(Handle, I_Title.GetData()); }
        void
        SetIcon(const FIconSet& I_IconSet) override;

        [[nodiscard]] Int32
        GetKeyboardKey(Int32 I_Key) const override;
        [[nodiscard]] Int32
        GetMouseButton(Int32 I_Button) const override;
        [[nodiscard]] void*
        CreateVulkanSurface(void* I_Instance) const override;
        [[nodiscard]] static TArray<const char*>
        GetVulkanRequiredInstanceExtensions();

        FGLFWWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height);
        ~FGLFWWindow() override;

    private:
        static inline IPlatformWindow*   FocusedWindow{nullptr};
        static inline TAtomic<Int32>     ContextCount{0};
        GLFWwindow*                      Handle = nullptr;
    };

    FGLFWWindow::
    FGLFWWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height)
    : IPlatformWindow(I_Title, I_Width, I_Height)
    {
        (void)ContextCount.FetchAdd(1, EMemoryOrder::AcqRel);
        if (!glfwInit())
        { LOG_ERROR("Failed to initialize GLFW!"); return; }

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE,	GLFW_TRUE);

        //Create Window
        Handle = glfwCreateWindow(
            Width, Height, //[TODO] read from config (save the last scale).
            Title.GetData(), // The App name is used as the Editor's main window.
            nullptr,
            nullptr);
        if (!Handle)
        { LOG_ERROR("Failed to create the GLFW Window!"); return; }

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

        glfwSetWindowContentScaleCallback (Handle, [](GLFWwindow* I_Window, Float I_ScaleX, Float I_ScaleY) { static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window))->WindowContentScaleCallback.Invoke(I_ScaleX, I_ScaleY); });
        glfwSetMouseButtonCallback        (Handle, [](GLFWwindow* I_Window, Int32 I_Button, Int32 I_Action, Int32 I_Mods) { static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window))->MouseButtonCallback.Invoke(I_Button, I_Action, I_Mods); });
        glfwSetCursorPosCallback          (Handle, [](GLFWwindow* I_Window, Double I_PosX, Double I_PosY) { static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window))->CursorMoveCallback.Invoke(I_PosX, I_PosY); });
        glfwSetScrollCallback             (Handle, [](GLFWwindow* I_Window, Double I_OffsetX, Double I_OffsetY) { static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window))->ScrollCallback.Invoke(I_OffsetX, I_OffsetY); });
        glfwSetKeyCallback                (Handle, [](GLFWwindow* I_Window, Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods) { static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window))->KeyboardCallback.Invoke(I_Key, I_ScanCode, I_Action, I_Mods); });
        glfwSetWindowFocusCallback        (Handle, [](GLFWwindow* I_Window, Int32 I_Focused) {
            auto* P = static_cast<IPlatformWindow*>(glfwGetWindowUserPointer(I_Window));
            if (I_Focused) { FGLFWWindow::FocusedWindow = P; }
            else if (FGLFWWindow::FocusedWindow == P) { FGLFWWindow::FocusedWindow = nullptr; }
        });
        // Newly created window is the focused one until the OS sends focus events (e.g. first message pump).
        FocusedWindow = this;

        glfwSetFramebufferSizeCallback    (Handle, [](GLFWwindow* I_Window, Int32 I_Width, Int32 I_Height) {
            if (auto* Self = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window)))
            {
                Self->Width  = static_cast<UInt32>(I_Width);
                Self->Height = static_cast<UInt32>(I_Height);
            }
            static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window))
            ->WindowResizeCallback.Invoke(I_Width, I_Height);
        });
    }

    IPlatformWindow*
    FGLFWWindow::GetFocusedPlatformWindow()
    {
        VISERA_ASSERT(FThread::IsMainThread());
        return FocusedWindow;
    }

    FGLFWWindow::
    ~FGLFWWindow()
    {
        VISERA_ASSERT(FThread::IsMainThread());
        if (Handle)
        {
            if (FocusedWindow == this) { FocusedWindow = nullptr; }
            // Prevent callbacks from touching destroyed engine singletons during teardown.
            glfwSetMouseButtonCallback       (Handle, nullptr);
            glfwSetCursorPosCallback         (Handle, nullptr);
            glfwSetScrollCallback            (Handle, nullptr);
            glfwSetKeyCallback               (Handle, nullptr);
            glfwSetFramebufferSizeCallback   (Handle, nullptr);
            glfwSetWindowContentScaleCallback(Handle, nullptr);
            glfwSetWindowFocusCallback       (Handle, nullptr);
            glfwSetWindowUserPointer         (Handle, nullptr);

            glfwDestroyWindow(Handle);
            Handle = nullptr;
        }

        // [NOTE]: GLFW is used globally, so we don't need to terminate it here (see FGLFWEventLoop).
        // if (ContextCount.fetch_sub(1, std::memory_order_relaxed) == 1) { glfwTerminate(); }
    }

    Int32 FGLFWWindow::
    GetKeyboardKey(Int32 I_Key) const
    {
        VISERA_ASSERT(FThread::IsMainThread());
        return Handle ? glfwGetKey(Handle, I_Key) : GLFW_RELEASE;
    }

    Int32 FGLFWWindow::
    GetMouseButton(Int32 I_Button) const
    {
        VISERA_ASSERT(FThread::IsMainThread());
        return Handle ? glfwGetMouseButton(Handle, I_Button) : GLFW_RELEASE;
    }

    void FGLFWWindow::
    SetIcon(const FIconSet& I_IconSet)
    {
        VISERA_ASSERT(FThread::IsMainThread());
        GLFWimage Icons[6]
        {
            { .width = 16, .height = 16,   .pixels = const_cast<unsigned char*>(I_IconSet.Icon16x16) },
            { .width = 32, .height = 32,   .pixels = const_cast<unsigned char*>(I_IconSet.Icon32x32) },
            { .width = 48, .height = 48,   .pixels = const_cast<unsigned char*>(I_IconSet.Icon48x48) },
            { .width = 64, .height = 64,   .pixels = const_cast<unsigned char*>(I_IconSet.Icon64x64) },
            { .width = 128, .height = 128, .pixels = const_cast<unsigned char*>(I_IconSet.Icon128x128) },
            { .width = 256, .height = 256, .pixels = const_cast<unsigned char*>(I_IconSet.Icon256x256) },
        };
        glfwSetWindowIcon(Handle, 6, Icons);
    }

    TArray<const char*> FGLFWWindow::
    GetVulkanRequiredInstanceExtensions()
    {
        VISERA_ASSERT(FThread::IsMainThread());
        if (!glfwInit())
        { LOG_FATAL("Failed to GetVulkanRequiredInstanceExtensions -- failed to initialize GLFW!"); }

        TArray<const char*> Out;
        UInt32 Count = 0;
        const char** Names = glfwGetRequiredInstanceExtensions(&Count);
        if (!Names) { return Out; }
        Out.Reserve(Count);
        for (UInt32 I = 0; I < Count; ++I)
        { Out.PushBack(Names[I]); }
        return Out;
    }

    void* FGLFWWindow::
    CreateVulkanSurface(void* I_Instance) const
    {
        VISERA_ASSERT(FThread::IsMainThread());
        if (!Handle || !I_Instance) { return nullptr; }
        VkSurfaceKHR Surface = VK_NULL_HANDLE;
        VkResult Result = glfwCreateWindowSurface(
            static_cast<VkInstance>(I_Instance),
            Handle,
            nullptr,
            &Surface);
        if (Result != VK_SUCCESS) { return nullptr; }
        return static_cast<void*>(Surface);
    }
}
