module;
#include <Visera-Window.hpp>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Global;
import Visera.Runtime.Input;
import Visera.Platform;
import Visera.Core.Types.String;
import Visera.Core.Types.Text;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Delegate.Multicast;

export namespace Visera
{
    class VISERA_RUNTIME_API FWindow : public IRuntimeService
    {
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
        [[nodiscard]] const FText&
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
        SetTitle(const FText& I_NewTitle) { PlatformWindow->SetTitle(I_NewTitle); }
        void
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight) { PlatformWindow->SetSize(I_NewWidth, I_NewHeight); }

    private:
        TUniquePtr<FPlatformWindow> PlatformWindow;
        FInput*                     Input            {nullptr};
        Bool                        bClosingNotified {False};

    public:
        FWindow(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
                TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
            : IRuntimeService(I_Name, I_Registry, std::move(I_ConfigView), I_OnConfigChange, I_RuntimeName)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                FText Title(GetConfig().GetString(TJSONRoute<"Window.Title">(), "Visera"));
                UInt32  Width  = GetConfig().GetNumber(TJSONRoute<"Window.Width">(), 512);
                UInt32  Height = GetConfig().GetNumber(TJSONRoute<"Window.Height">(), 512);

                PlatformWindow = FPlatform::CreateWindow(Title, Width, Height);
                if (!PlatformWindow) { return False; }

                Input = GetService<FInput>(EService::Input).Lock().Get();
                if (Input && !Input->RegisterWindow(PlatformWindow.Get()))
                { LOG_FATAL("Failed to register window with FInput!"); }

                if (!PlatformWindow->WindowResizeCallback.TryBind([](Int32, Int32) { /* RHI detects resize via BeginFrame/Present and bDirty */ }))
                { LOG_FATAL("Failed to bind resize window event!"); }

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                if (PlatformWindow && Input)
                { Input->UnregisterWindow(PlatformWindow.Get()); }
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }

            if (!OnConfigChange.TryBind([this](const FJSONRoute& I_NewConfig)
            {
                auto Route = I_NewConfig.GetRouteString();
                if (Route == FStringView("Window.Title").GetNative())
                {
                    PlatformWindow->SetTitle(FText(GetConfig().GetString(
                        TJSONRoute<"Window.Title">(),
                        PlatformWindow->GetTitle().GetString())));
                }
                else if (Route == FStringView("Window.Width").GetNative() ||
                         Route == FStringView("Window.Height").GetNative())
                {
                    UInt32 Width  = GetConfig().GetNumber(TJSONRoute<"Window.Width">(), 512);
                    UInt32 Height = GetConfig().GetNumber(TJSONRoute<"Window.Height">(), 512);
                    PlatformWindow->SetSize(Width, Height);
                }
            }))
            { LOG_FATAL("Failed to bind OnConfigChange function!"); }
        }
    };
}
