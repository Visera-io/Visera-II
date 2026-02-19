module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Cross.Null.Window;
#define VISERA_MODULE_NAME "Platform.Cross"
import Visera.Platform.Interface.Window;

namespace Visera
{
    export class VISERA_PLATFORM_API FNullWindow : public IPlatformWindow
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
        void inline
        SetTitle(FStringView I_Title) override { }
        void inline
        SetIcon(const FIconSet& I_IconSet) override { }

        FNullWindow() = delete;
        FNullWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height);
        ~FNullWindow() override;
    };

    FNullWindow::
    FNullWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height)
    : IPlatformWindow(I_Title, I_Width, I_Height)
    {

    }

    FNullWindow::
    ~FNullWindow()
    {

    }
}
