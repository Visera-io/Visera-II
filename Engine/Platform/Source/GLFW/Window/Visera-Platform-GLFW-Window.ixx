module;
#include <Visera-Platform.hpp>
#define GLFW_INCLUDE_VULKAN
#if defined(VISERA_ON_WINDOWS_SYSTEM)
  #define GLFW_EXPOSE_NATIVE_WIN32
#endif
#if defined(__APPLE__)
  #define GLFW_EXPOSE_NATIVE_COCOA
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
export module Visera.Platform.GLFW.Window;
#define VISERA_MODULE_NAME "Platform.GLFW"
import Visera.Platform.Interface.Device;
import Visera.Platform.Interface.Window;
import Visera.Core.OS.Thread;
import Visera.Core.Log;
import Visera.Core.Containers.Array;

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
        SetTitle(FStringView I_Title) override { VISERA_ASSERT(FThread::IsMainThread()); Title = I_Title; glfwSetWindowTitle(Handle, Title.GetNative().c_str()); }
        void
        SetIcon(const FIconSet& I_IconSet) override;

        [[nodiscard]] EPlatformKeyboardKeyState
        QueryKeyboardKeyState(EPlatformKeyboardKey I_Key) const override;
        [[nodiscard]] EPlatformMouseButtonState
        QueryMouseButtonState(EPlatformMouseButton I_Button) const override;
        void
        QueryKeyboardState(TSpan<EPlatformKeyboardKeyState, kKeyboardStateTableSize> O_Out) const override;
        void
        QueryMouseButtonState(TSpan<EPlatformMouseButtonState, kMouseButtonStateTableSize> O_Out) const override;
        [[nodiscard]] void*
        GetNativeHandle() const override;
        [[nodiscard]] void*
        CreateVulkanSurface(void* I_Instance) const override;
        [[nodiscard]] static TArray<const char*>
        GetVulkanRequiredInstanceExtensions();

        FGLFWWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height);
        ~FGLFWWindow() override;

    private:
        static inline IPlatformWindow*   FocusedWindow{nullptr};
        static inline TAtomic<Int32>     ContextCount{0};
        GLFWwindow*                      Handle = nullptr;
    };

    FGLFWWindow::
    FGLFWWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height)
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
            Title.GetNative().c_str(), // The App name is used as the Editor's main window.
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

        glfwSetWindowContentScaleCallback (Handle, [](GLFWwindow* I_Window, Float I_ScaleX, Float I_ScaleY) {
            auto* W = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window));
            W->SetContentScale(I_ScaleX, I_ScaleY);
            W->WindowContentScaleCallback.Invoke(I_ScaleX, I_ScaleY);
        });
        // Cast GLFW ints to Interface.Device enums at the boundary; FInput receives only enums.
        glfwSetMouseButtonCallback        (Handle, [](GLFWwindow* I_Window, Int32 I_Button, Int32 I_Action, Int32 I_Mods) {
            auto* W = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window));
            W->MouseButtonCallback.Invoke(static_cast<EPlatformMouseButton>(I_Button), I_Action == GLFW_RELEASE ? EPlatformMouseButtonState::Release : EPlatformMouseButtonState::Press, static_cast<EPlatformKeyboardModifier>(I_Mods));
        });
        glfwSetCursorPosCallback          (Handle, [](GLFWwindow* I_Window, Double I_PosX, Double I_PosY) {
            auto* W = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window));
            W->CursorMoveCallback.Invoke(I_PosX * W->GetScaleX(),
                                         I_PosY * W->GetScaleY());
        });
        glfwSetScrollCallback             (Handle, [](GLFWwindow* I_Window, Double I_OffsetX, Double I_OffsetY) { static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window))->ScrollCallback.Invoke(I_OffsetX, I_OffsetY); });
        glfwSetKeyCallback                (Handle, [](GLFWwindow* I_Window, Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods) {
            auto* W = static_cast<FGLFWWindow*>(glfwGetWindowUserPointer(I_Window));
            EPlatformKeyboardKeyState Action = (I_Action == GLFW_RELEASE) ? EPlatformKeyboardKeyState::Release : EPlatformKeyboardKeyState::Press;
            W->KeyboardCallback.Invoke(static_cast<EPlatformKeyboardKey>(I_Key), I_ScanCode, Action, static_cast<EPlatformKeyboardModifier>(I_Mods));
        });
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

    EPlatformKeyboardKeyState FGLFWWindow::
    QueryKeyboardKeyState(EPlatformKeyboardKey I_Key) const
    {
        VISERA_ASSERT(FThread::IsMainThread());
        if (!Handle) { return EPlatformKeyboardKeyState::Release; }
        return static_cast<EPlatformKeyboardKeyState>(glfwGetKey(Handle, static_cast<int>(I_Key)));
    }

    EPlatformMouseButtonState FGLFWWindow::
    QueryMouseButtonState(EPlatformMouseButton I_Button) const
    {
        VISERA_ASSERT(FThread::IsMainThread());
        if (!Handle) { return EPlatformMouseButtonState::Release; }
        return static_cast<EPlatformMouseButtonState>(glfwGetMouseButton(Handle, static_cast<int>(I_Button)));
    }

    void FGLFWWindow::
    QueryKeyboardState(TSpan<EPlatformKeyboardKeyState, kKeyboardStateTableSize> O_Out) const
    {
        VISERA_ASSERT(FThread::IsMainThread());
        if (!Handle) { return; }
        for (size_t i = 0; i < kKeyboardStateTableSize; ++i)
        {
            if (i >= static_cast<size_t>(GLFW_KEY_SPACE) && i <= static_cast<size_t>(GLFW_KEY_LAST))
                O_Out[i] = static_cast<EPlatformKeyboardKeyState>(glfwGetKey(Handle, static_cast<int>(i)));
            else
                O_Out[i] = EPlatformKeyboardKeyState::Release;
        }
    }

    void FGLFWWindow::
    QueryMouseButtonState(TSpan<EPlatformMouseButtonState, kMouseButtonStateTableSize> O_Out) const
    {
        VISERA_ASSERT(FThread::IsMainThread());
        if (!Handle) { return; }
        for (size_t i = 0; i < kMouseButtonStateTableSize; ++i)
        { O_Out[i] = static_cast<EPlatformMouseButtonState>(glfwGetMouseButton(Handle, static_cast<int>(i))); }
    }

    void* FGLFWWindow::
    GetNativeHandle() const
    {
        if (!Handle) { return nullptr; }
#if defined(VISERA_ON_WINDOWS_SYSTEM)
        return static_cast<void*>(glfwGetWin32Window(Handle));
#elif defined(__APPLE__)
        return static_cast<void*>(glfwGetCocoaWindow(Handle));
#else
        return nullptr;
#endif
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
