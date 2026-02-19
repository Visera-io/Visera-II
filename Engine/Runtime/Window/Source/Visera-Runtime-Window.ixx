module;
#include <Visera-Window.hpp>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Global;
import Visera.Platform;
import Visera.Core.Types.String;
import Visera.Core.Types.Pointer.Unique;
import Visera.Core.Delegate.Multicast;

export namespace Visera
{
    class VISERA_RUNTIME_API FWindow : public IGlobalService
    {
    public:
        using FIconSet = FPlatformWindow::FIconSet;
        TMulticastDelegate<FWindow*>
        OnResized;

        [[nodiscard]] Bool
        ShouldClose() const { return PlatformWindow->ShouldClose(); }
        [[nodiscard]] FStringView
        GetTitle()  const { return PlatformWindow->GetTitle(); }
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
        SetIcon(const FIconSet& I_IconSet) { PlatformWindow->SetIcon(I_IconSet); }
        void
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight) { PlatformWindow->SetSize(I_NewWidth, I_NewHeight); }

    private:
        TUniquePtr<FPlatformWindow> PlatformWindow;

    public:
        FWindow(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                FString Title  = GetConfig().GetString(TJSONRoute<"Window.Title">(), "Visera");
                UInt32  Width  = GetConfig().GetNumber(TJSONRoute<"Window.Width">(), 512);
                UInt32  Height = GetConfig().GetNumber(TJSONRoute<"Window.Height">(), 512);
                
                PlatformWindow = FPlatform::CreateWindow(Title, Width, Height);
                if (!PlatformWindow) { return False; }

                if (!PlatformWindow->WindowResizeCallback.TryBind([this](Int32 /*I_NewWidth*/, Int32 /*I_NewHeight*/)
                {
                    OnResized.Broadcast(this);
                })) { LOG_FATAL("Failed to bind resize window event!"); }

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }

            if (!OnConfigChange.TryBind([this](const FJSONRoute& I_NewConfig)
            {
                if (I_NewConfig.GetRouteString() == FStringView("Window.Title").GetNative())
                {
                    PlatformWindow->SetTitle(GetConfig().GetString(
                        TJSONRoute<"Window.Title">(),
                        PlatformWindow->GetTitle()));
                }
            }))
            { LOG_FATAL("Failed to bind OnConfigChange function!"); }
        }
    };
}
