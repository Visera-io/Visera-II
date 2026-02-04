module;
#include <Visera-Platform.hpp>
#if defined (VISERA_ON_WINDOWS_SYSTEM)
#include <VISERA_ICONS.inl>
#endif
export module Visera.Platform.Window;
#define VISERA_MODULE_NAME "Platform.Window"
import Visera.Platform.Window.Interface;
import Visera.Platform.Window.Null;
import Visera.Platform.Window.GLFW;
import Visera.Global.Service;
import Visera.Global.Log;
import Visera.Core.Delegate.Multicast;

namespace Visera
{
    export using EWindowType = IWindow::EType;

    export class VISERA_PLATFORM_API FWindow : public IGlobalService
    {
    public:
        TMulticastDelegate<UInt32, UInt32>
        OnResizeWindow;

        using FIconSet = IWindow::FIconSet;
        [[nodiscard]] inline void*
        GetHandle() const { return Window->GetHandle(); }
        [[nodiscard]] inline Bool
        ShouldClose() const { return Window->ShouldClose(); }
        inline void
        WaitEvents() const { Window->WaitEvents(); }
        inline void
        PollEvents() const  { Window->PollEvents(); }
        inline void
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) { Window->SetSize(I_NewWidth, I_NewHeight); OnResizeWindow.Broadcast(I_NewWidth, I_NewHeight); }
        inline void
        SetPosition(Int32 I_X, Int32 I_Y) const { Window->SetPosition(I_X, I_Y); }
        [[nodiscard]] inline FStringView
        GetTitle() const { return Window->GetTitle(); }
        inline void
        SetTitle(FStringView I_Title) { Window->SetTitle(I_Title); }
        inline void
        SetIcon(const FIconSet& I_IconSet)
        {
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            VISERA_ASSERT(I_IconSet.Icon16x16);
            VISERA_ASSERT(I_IconSet.Icon32x32);
            VISERA_ASSERT(I_IconSet.Icon48x48);
            VISERA_ASSERT(I_IconSet.Icon64x64);
            VISERA_ASSERT(I_IconSet.Icon128x128);
            VISERA_ASSERT(I_IconSet.Icon256x256);
            Window->SetIcon(I_IconSet);
#endif
        }

        [[nodiscard]] inline UInt32
        GetWidth() const  { return Window->GetWidth(); }
        [[nodiscard]] inline UInt32
        GetHeight() const { return Window->GetHeight(); }
        [[nodiscard]] inline Float
        GetScaleX() const  { return Window->GetScaleX(); }
        [[nodiscard]] inline Float
        GetScaleY() const { return Window->GetScaleY(); }
        [[nodiscard]] EWindowType
        GetType() const { return Window->GetType(); }
        [[nodiscard]] inline Bool
        IsMaximized() const { return Window->IsMaximized(); }
        
    public:
        FWindow() : IGlobalService(EName::Window)
        {
            Dependencies =
            {
                EName::Input,
            };
            if (!OnBootstrap.TryBind([this]
            {
#if !defined(VISERA_OFFSCREEN_MODE)
                Window =new FGLFWWindow();
#else
                Window =new FNullWindow();
#endif
#if defined (VISERA_ON_WINDOWS_SYSTEM)
                SetIcon(FIconSet{
                    .Icon16x16   = ::ViseraIcons[EViseraIcon::X16],
                    .Icon32x32   = ::ViseraIcons[EViseraIcon::X32],
                    .Icon48x48   = ::ViseraIcons[EViseraIcon::X48],
                    .Icon64x64   = ::ViseraIcons[EViseraIcon::X64],
                    .Icon128x128 = ::ViseraIcons[EViseraIcon::X128],
                    .Icon256x256 = ::ViseraIcons[EViseraIcon::X256],
                });
#endif
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                delete Window;
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    private:
        IWindow* Window;
    };
}
