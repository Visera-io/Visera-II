module;
#include <Visera-Window.hpp>
export module Visera.Runtime.Window;
#define VISERA_MODULE_NAME "Runtime.Window"
import Visera.Runtime.Global;
import Visera.Runtime.Input;
import Visera.Platform;
import Visera.Core.Types.String;
import Visera.Core.Types.Text;
import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    enum class EPresentMode : UInt8
    {
        VSync,
        Mailbox,
    };

    class VISERA_RUNTIME_API FWindow : public IRuntimeService
    {
    public:
        using FIconSet = FPlatformWindow::FIconSet;

        [[nodiscard]] Bool
        ShouldClose() const
        {
            if (!bHasInputService) { FPlatform::PollEvents(); }
            return PlatformWindow->ShouldClose();
        }
        [[nodiscard]] EPresentMode
        GetPresentMode() const { return PresentMode; }
        void
        SetPresentMode(EPresentMode I_Mode) { PresentMode = I_Mode; }
        [[nodiscard]] const FText&
        GetTitle()  const { return PlatformWindow->GetTitle(); }
        [[nodiscard]] UInt32
        GetWidth()  const { return PlatformWindow->GetWidth(); }
        [[nodiscard]] UInt32
        GetHeight() const { return PlatformWindow->GetHeight(); }
        /** True when window has zero width or height (e.g. minimized). Used to skip submit/present. */
        [[nodiscard]] Bool
        IsMinimized() const { return GetWidth() == 0 || GetHeight() == 0; }
        [[nodiscard]] Float
        GetScaleX()  const { return PlatformWindow->GetScaleX(); }
        [[nodiscard]] Float
        GetScaleY()  const { return PlatformWindow->GetScaleY(); }
        [[nodiscard]] const TUniqueRef<FPlatformWindow>
        GetPlatformWindow() const { return PlatformWindow; }

        void
        SetIcon(const FIconSet& I_IconSet) { PlatformWindow->SetIcon(I_IconSet); }
        void
        SetTitle(const FText& I_NewTitle) { PlatformWindow->SetTitle(I_NewTitle); }
        void
        Resize(UInt32 I_NewWidth, UInt32 I_NewHeight) { PlatformWindow->SetSize(I_NewWidth, I_NewHeight); }

    private:
        TUniquePtr<FPlatformWindow> PlatformWindow;
        EPresentMode                PresentMode      {EPresentMode::VSync};
        Bool                        bHasInputService {False};

    public:
        FWindow(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
                TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
            : IRuntimeService(I_Name, I_Registry, std::move(I_ConfigView), I_OnConfigChange, I_RuntimeName)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                FText Title(GetConfig().GetString(TJSONRoute<"Window.Title">(), "Visera"));
                UInt32  Width  = GetConfig().GetNumber(TJSONRoute<"Window.Width">(), 512);
                UInt32  Height = GetConfig().GetNumber(TJSONRoute<"Window.Height">(), 512);
                
                PlatformWindow = FPlatform::CreateWindow(Title, Width, Height);
                if (!PlatformWindow) { return False; }

                if (!PlatformWindow->WindowResizeCallback.TryBind([](Int32, Int32) { /* RHI detects resize via BeginFrame/Present and bDirty */ }))
                { LOG_FATAL("Failed to bind resize window event!"); }

                if (auto Input = GetService<FInput>(EService::Input).Lock())
                {
                    LOG_TRACE("Binding window callbacks to the Input service...");
                    Input->SetWindowForSync(PlatformWindow.Get());
                    bHasInputService = True;

                    if (!PlatformWindow->MouseButtonCallback.TryBind([Input](Int32 I_Button, Int32 I_Action, Int32 I_Mods)
                    { Input->NotifyMouseButton(static_cast<FMouse::EButton>(I_Button), static_cast<FMouse::EAction>(I_Action <= 2 ? I_Action : 0), static_cast<UInt8>(I_Mods)); }))
                    { LOG_ERROR("Failed to bind MouseButtonCallback event!"); }
                    if (!PlatformWindow->KeyboardCallback.TryBind([Input](Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods)
                    { Input->NotifyKeyboardKey(static_cast<FKeyboard::EKey>(I_Key), I_ScanCode, static_cast<FKeyboard::EAction>(I_Action <= 2 ? I_Action : 0), static_cast<UInt8>(I_Mods)); }))
                    { LOG_ERROR("Failed to bind KeyboardCallback event!"); }
                    if (!PlatformWindow->CursorMoveCallback.TryBind([Input](Double I_PosX, Double I_PosY)
                    { Input->NotifyCursorMove(static_cast<Float>(I_PosX), static_cast<Float>(I_PosY)); }))
                    { LOG_ERROR("Failed to bind CursorMoveCallback event!"); }
                    if (!PlatformWindow->ScrollCallback.TryBind([Input](Double I_OffsetX, Double I_OffsetY)
                    { Input->NotifyScroll(static_cast<Float>(I_OffsetX), static_cast<Float>(I_OffsetY)); }))
                    { LOG_ERROR("Failed to bind ScrollCallback event!"); }
                }

                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                if (bHasInputService)
                {
                    if (auto Input = GetService<FInput>(EService::Input).Lock())
                    { Input->SetWindowForSync(nullptr); }
                }
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }

            if (!OnConfigChange.TryBind([this](const FJSONRoute& I_NewConfig)
            {
                auto Route = I_NewConfig.GetRouteString();
                if (Route == FStringView("Window.Title").GetNative())
                {
                    PlatformWindow->SetTitle(FText(GetConfig().GetString(
                        TJSONRoute<"Window.Title">(),
                        PlatformWindow->GetTitle().GetString())));
                }
                else if (Route == FStringView("Window.Width").GetNative() ||
                         Route == FStringView("Window.Height").GetNative())
                {
                    UInt32 Width  = GetConfig().GetNumber(TJSONRoute<"Window.Width">(), 512);
                    UInt32 Height = GetConfig().GetNumber(TJSONRoute<"Window.Height">(), 512);
                    PlatformWindow->SetSize(Width, Height);
                }
            }))
            { LOG_FATAL("Failed to bind OnConfigChange function!"); }
        }
    };
}
