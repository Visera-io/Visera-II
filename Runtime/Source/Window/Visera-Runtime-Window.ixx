module;
#include <Visera-Runtime.hpp>

#include "GLFW/glfw3.h"
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Window.Interface;
import Visera.Runtime.Window.Null;
import Visera.Runtime.Window.GLFW;
import Visera.Core.Global;
import Visera.Core.Log;

namespace Visera
{
    export using EWindowType = IWindow::EType;

    export class VISERA_RUNTIME_API FWindow : public IGlobalSingleton<FWindow>
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
        FWindow() : IGlobalSingleton("Window") {}
        void Bootstrap() override
        {
            LOG_TRACE("Bootstrapping Window.");
#if !defined(VISERA_OFFSCREEN_MODE)
            Window = MakeUnique<FGLFWWindow>();
#else
            Window = MakeUnique<FNullWindow>();
#endif

            Status = EStatus::Bootstrapped;
        }
        void Terminate() override
        {
            LOG_TRACE("Terminating Window.");
            Window.reset();

            Status = EStatus::Terminated;
        }

    private:
        TUniquePtr<IWindow> Window;
    };

    export inline VISERA_RUNTIME_API TUniquePtr<FWindow>
    GWindow = MakeUnique<FWindow>();
}
