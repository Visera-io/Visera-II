module;
#include <Visera-Input.hpp>
export module Visera.Runtime.Input;
#define VISERA_MODULE_NAME "Runtime.Inpu"
export import Visera.Runtime.Input.Keyboard;
export import Visera.Runtime.Input.Mouse;
       import Visera.Runtime.Global;
       import Visera.Platform;

export namespace Visera
{
    class VISERA_RUNTIME_API FInput : public IGlobalService
    {
    public:
        [[nodiscard]] inline FKeyboard*
        GetKeyboard() { return &Keyboard; }
        [[nodiscard]] inline FMouse*
        GetMouse()    { return &Mouse; }

    private:
        FKeyboard Keyboard;
        FMouse    Mouse;

    public:
        FInput(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {
#if !defined(VISERA_OFFSCREEN_MODE)
                EName::Window,
#endif
            };

            if (!OnBootstrap.TryBind([this]
            {
                if (!FPlatformWindow::MouseButtonCallback.TryBind(
                [this](Int32 I_Button, Int32 I_Action, Int32 I_Mods)
                {
                    const auto Button = static_cast<FMouse::EButton>(I_Button);
                    switch (static_cast<FMouse::EAction>(I_Action))
                    {
                    case FMouse::EAction::Release : return Mouse.OnReleased.Broadcast(Button);
                    case FMouse::EAction::Press   : return Mouse.OnPressed.Broadcast(Button);
                    case FMouse::EAction::Hold    : return Mouse.OnHeld.Broadcast(Button);
                    default: LOG_ERROR("Unhandled button action ({})!", I_Action);
                    }
                }))
                { LOG_FATAL("Failed to bind CursorMoveCallback event!"); }

                if (!FPlatformWindow::KeyboardCallback.TryBind(
                [this](Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods)
                {
                    const auto Key = static_cast<FKeyboard::EKey>(I_Key);
                    switch (static_cast<FKeyboard::EAction>(I_Action))
                    {
                    case FKeyboard::EAction::Release : return Keyboard.OnReleased.Broadcast(Key);
                    case FKeyboard::EAction::Press   : return Keyboard.OnPressed.Broadcast(Key);
                    case FKeyboard::EAction::Hold    : return Keyboard.OnHeld.Broadcast(Key);
                    default: LOG_ERROR("Unhandled key action ({})!", I_Action);
                    }
                }))
                { LOG_FATAL("Failed to bind KeyboardCallback event!"); }

                if (!FPlatformWindow::CursorMoveCallback.TryBind(
                [this](Double I_PosX, Double I_PosY)
                {
                    Mouse.OnCursorMoved.Broadcast(I_PosX, I_PosY);
                }))
                { LOG_FATAL("Failed to bind CursorMoveCallback event!"); }

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