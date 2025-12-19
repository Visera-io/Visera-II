module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Window.Null;
#define VISERA_MODULE_NAME "Platform.Window"
import Visera.Platform.Window.Interface;
import Visera.Runtime.Log;

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
        WaitEvents() const override {VISERA_UNIMPLEMENTED_API; }
        inline void
        PollEvents() const override { VISERA_UNIMPLEMENTED_API; }
        void inline
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) override { VISERA_UNIMPLEMENTED_API; }
        void inline
        SetPosition(Int32 I_X, Int32 I_Y) const override { VISERA_UNIMPLEMENTED_API; }
        [[nodiscard]] FStringView
        GetTitle() const override { VISERA_UNIMPLEMENTED_API; return ""; }
        void inline
        SetTitle(FStringView I_Title) override { VISERA_UNIMPLEMENTED_API; }

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
