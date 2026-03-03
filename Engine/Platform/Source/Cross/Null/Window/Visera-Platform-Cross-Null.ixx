module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Cross.Null.Window;
#define VISERA_MODULE_NAME "Platform.Cross"
import Visera.Platform.Interface.Window;
import Visera.Core.Types.Text;

namespace Visera
{
    export class VISERA_PLATFORM_API FNullWindow : public IPlatformWindow
    {
    public:
        [[nodiscard]] inline void*
        GetHandle() const override { return nullptr; }
        [[nodiscard]] inline Bool
        ShouldClose() const override { return True; }
        void inline
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) override { }
        void inline
        SetPosition(Int32 I_X, Int32 I_Y) const override { }
        void inline
        SetTitle(const FText& I_Title) override { Title = I_Title; }
        void inline
        SetIcon(const FIconSet& I_IconSet) override { }
        [[nodiscard]] void*
        CreateVulkanSurface(void* I_Instance) const override { (void)I_Instance; return nullptr; }

        FNullWindow() = delete;
        FNullWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height);
        ~FNullWindow() override;
    };

    FNullWindow::
    FNullWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height)
    : IPlatformWindow(I_Title, I_Width, I_Height)
    {

    }

    FNullWindow::
    ~FNullWindow()
    {

    }
}
