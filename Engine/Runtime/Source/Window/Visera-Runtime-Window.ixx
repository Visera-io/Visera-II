module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Input;
import Visera.Platform;
import Visera.Core.Log;
import Visera.Core.Types.String;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Delegate.Multicast;

export namespace Visera
{
    /** CreateInfo for FWindow. Presence in FEngineCreateInfo enables the window service. */
    struct VISERA_RUNTIME_API FWindowCreateInfo
    {
        FString Title  = "Visera";
        UInt32  Width  = 512;
        UInt32  Height = 512;
        Bool    Resizable = True;
        /** If true, position the window on the primary monitor work area after creation. */
        Bool    Center   = False;
        /** Exclusive fullscreen on the primary monitor (GLFW: current video mode resolution). */
        Bool    Fullscreen = False;
    };

    class VISERA_RUNTIME_API FWindow
    {
    public:
        /** I_Input may be nullptr; if non-null, the window registers with it for input callbacks. */
        explicit FWindow(const FWindowCreateInfo& I_CreateInfo, FInput* I_Input = nullptr);
        ~FWindow();

    public:
        using FIconSet = FPlatformWindow::FIconSet;

        /** True if the platform window should close. Call FPlatform::PollEvents() once per frame before checking. On first true, broadcasts OnWindowClosing(this) once. */
        [[nodiscard]] Bool
        ShouldClose()
        {
            Bool bShouldClose = PlatformWindow->ShouldClose();
            if (bShouldClose && !bClosingNotified)
            {
                bClosingNotified = True;
                OnWindowClosing.Broadcast(const_cast<FWindow*>(this));
            }
            return bShouldClose;
        }
        [[nodiscard]] FStringView
        GetTitle()  const { return PlatformWindow->GetTitle(); }
        [[nodiscard]] UInt32
        GetWidth()  const { return PlatformWindow->GetWidth(); }
        [[nodiscard]] UInt32
        GetHeight() const { return PlatformWindow->GetHeight(); }
        /** True when window has zero width or height (e.g. minimized). Used to skip submit/present. */
        [[nodiscard]] Bool
        IsMinimized() const { return GetWidth() == 0 || GetHeight() == 0; }
        [[nodiscard]] Float
        GetScaleX()  const { return PlatformWindow->GetScaleX(); }
        [[nodiscard]] Float
        GetScaleY()  const { return PlatformWindow->GetScaleY(); }
        [[nodiscard]] const TUniqueRef<FPlatformWindow>
        GetPlatformWindow() const { return PlatformWindow; }

        /** Fired once when ShouldClose() first returns true. Argument is this window (for multi-window). */
        TMulticastDelegate<FWindow*>                   OnWindowClosing;

        void
        SetIcon(const FIconSet& I_IconSet) { PlatformWindow->SetIcon(I_IconSet); }
        void
        SetTitle(FStringView I_NewTitle) { PlatformWindow->SetTitle(I_NewTitle); }
        void
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight) { PlatformWindow->SetSize(I_NewWidth, I_NewHeight); }

    private:
        TUniquePtr<FPlatformWindow> PlatformWindow;
        FInput*                     Input            {nullptr};
        Bool                        bClosingNotified {False};
    };

    FWindow::FWindow(const FWindowCreateInfo& I_CreateInfo, FInput* I_Input)
        : Input(I_Input)
    {
        PlatformWindow = FPlatform::CreateWindow(I_CreateInfo.Title, I_CreateInfo.Width, I_CreateInfo.Height, I_CreateInfo.Resizable, I_CreateInfo.Center, I_CreateInfo.Fullscreen);
        if (!PlatformWindow)
        { LOG_FATAL("Failed to create platform window!"); return; }

        if (Input && !Input->RegisterWindow(PlatformWindow.Get()))
        { LOG_FATAL("Failed to register window with FInput!"); }

        if (!PlatformWindow->WindowResizeCallback.TryBind([](Int32, Int32) { /* RHI detects resize via BeginFrame/Present and bDirty */ }))
        { LOG_FATAL("Failed to bind resize window event!"); }
    }

    FWindow::~FWindow()
    {
        if (PlatformWindow && Input)
        { Input->UnregisterWindow(PlatformWindow.Get()); }
    }
}
