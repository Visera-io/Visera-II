module;
#include <Visera-Window.hpp>
export module Visera.Window;
#define VISERA_MODULE_NAME "Window"
import Visera.Global;
import Visera.Platform;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Delegate.Multicast;

export namespace Visera
{
    class VISERA_WINDOW_API FWindow : public IGlobalService
    {
    public:
        TMulticastDelegate<UInt32, UInt32>
        OnResized;

        [[nodiscard]] Bool
        ShouldClose() const { return PlatformWindow->ShouldClose(); }
        [[nodiscard]] UInt32
        GetWidth()  const { return PlatformWindow->GetWidth(); }
        [[nodiscard]] UInt32
        GetHeight() const { return PlatformWindow->GetHeight(); }
        [[nodiscard]] Float
        GetScaleX()  const { return PlatformWindow->GetScaleX(); }
        [[nodiscard]] Float
        GetScaleY()  const { return PlatformWindow->GetScaleY(); }
        [[nodiscard]] const TUniqueRef<FPlatformWindow>
        GetPlatformWindow() const { return PlatformWindow; }

        void
        PollEvents() const { return PlatformWindow->PollEvents(); }
        void
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight) const { PlatformWindow->SetSize(I_NewWidth, I_NewHeight); }

    private:
        TUniquePtr<FPlatformWindow> PlatformWindow;

    public:
        FWindow() : IGlobalService(EName::Window)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                PlatformWindow = FPlatform::CreateWindow("Visera", 1920, 1080);
                if (!FPlatformWindow::WindowResizeCallback.TryBind([this](UInt32 I_NewWidth, UInt32 I_NewHeight)
                {
                    OnResized.Broadcast(I_NewWidth, I_NewHeight);
                })) { LOG_FATAL("Failed to bind resize window event!"); }
                return PlatformWindow != nullptr;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };
}
