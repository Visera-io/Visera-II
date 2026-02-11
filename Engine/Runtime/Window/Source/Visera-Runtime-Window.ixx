module;
#include <Visera-Window.hpp>
#include <VISERA_ICONS.inl>
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
        PollEvents() const { return PlatformWindow->PollEvents(); }
        void
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight) const { PlatformWindow->SetSize(I_NewWidth, I_NewHeight); }

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
                FJSON ServiceConfig = Config.GetObject("Window");
                FString Title = ServiceConfig.GetString("Title", "Visera");
                UInt32  Width = static_cast<UInt32>(ServiceConfig.GetNumber("Width", 512));
                UInt32  Height = static_cast<UInt32>(ServiceConfig.GetNumber("Height", 512));
                
                PlatformWindow = FPlatform::CreateWindow(Title, Width, Height);
                if (!PlatformWindow) { return False; }

                if (!PlatformWindow->WindowResizeCallback.TryBind([this](Int32 /*I_NewWidth*/, Int32 /*I_NewHeight*/)
                {
                    OnResized.Broadcast(this);
                })) { LOG_FATAL("Failed to bind resize window event!"); }

                PlatformWindow->SetIcon(
                {
                    .Icon16x16   = ViseraIcons[X16],
                    .Icon32x32   = ViseraIcons[X32],
                    .Icon48x48   = ViseraIcons[X48],
                    .Icon64x64   = ViseraIcons[X64],
                    .Icon128x128 = ViseraIcons[X128],
                    .Icon256x256 = ViseraIcons[X256],
                });
                return True;
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
