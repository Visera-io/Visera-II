module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Window.Null;
#define VISERA_MODULE_NAME "Platform.Window"
import Visera.Platform.Window.Interface;
import Visera.Global.Log;

namespace Visera
{
    export class VISERA_PLATFORM_API FNullWindow : public IWindow
    {
    public:
        [[nodiscard]] inline void*
        GetHandle() const override { return nullptr; }
        [[nodiscard]] inline Bool
        ShouldClose() const override { return True; }
        inline void
        WaitEvents() const override { }
        inline void
        PollEvents() const override { }
        void inline
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) override { }
        void inline
        SetPosition(Int32 I_X, Int32 I_Y) const override { }
        [[nodiscard]] FStringView
        GetTitle() const override { return ""; }
        void inline
        SetTitle(FStringView I_Title) override { }
        void inline
        SetIcon(const FIconSet& I_IconSet) override { }

        FNullWindow();
        ~FNullWindow() override;
    };

    FNullWindow::
    FNullWindow() : IWindow(EType::Null)
    {
        LOG_TRACE("Bootstrapping Null Window.");
    }

    FNullWindow::
    ~FNullWindow()
    {
        LOG_TRACE("Terminating Null Window.");
    }
}
