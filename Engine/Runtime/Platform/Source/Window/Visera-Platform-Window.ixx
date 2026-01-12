module;
#include <Visera-Platform.hpp>
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
        [[nodiscard]] inline void*
        GetHandle() const { return Window->GetHandle(); }
        [[nodiscard]] inline Bool
        ShouldClose() const { return Window->ShouldClose(); }
        inline void
        WaitEvents() const { Window->WaitEvents(); }
        inline void
        PollEvents() const  { Window->PollEvents(); }
        inline void
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) { Window->SetSize(I_NewWidth, I_NewHeight); };
        inline void
        SetPosition(Int32 I_X, Int32 I_Y) const { Window->SetPosition(I_X, I_Y); }
        [[nodiscard]] inline FStringView
        GetTitle() const { return Window->GetTitle(); }
        inline void
        SetTitle(FStringView I_Title) { Window->SetTitle(I_Title); }
        inline void
        SetIcon(TMutable<FByte> I_Data, Int32 I_Width, Int32 I_Height)
        {
#if defined(VISERA_ON_WINDOWS_SYSTEM)
            Window->SetIcon(I_Data, I_Width, I_Height);
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
                Window = MakeUnique<FGLFWWindow>();
#else
                Window = MakeUnique<FNullWindow>();
#endif
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                Window.reset();
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    private:
        TUniquePtr<IWindow> Window;
    };

    export inline VISERA_PLATFORM_API TUniquePtr<FWindow>
    GWindow = MakeUnique<FWindow>();
}
