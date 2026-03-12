module;
#include <Visera-Platform.hpp>
#include <GLFW/glfw3.h>
export module Visera.Platform.GLFW.Device.Keyboard;
#define VISERA_MODULE_NAME "Platform.GLFW"
import Visera.Platform.Interface.Device.Keyboard;

export namespace Visera
{
    /** Alias for Interface.Device type; GLFW uses same values. static_assert keeps them in sync. */
    using EGLFWKeyboardKey    = EPlatformKeyboardKey;
    using EGLFWKeyboardAction = EPlatformKeyboardKeyState;
    static_assert(static_cast<Int32>(EPlatformKeyboardKey::Space) == GLFW_KEY_SPACE);
    static_assert(static_cast<Int32>(EPlatformKeyboardKey::Menu) == GLFW_KEY_MENU);
    static_assert(static_cast<Int32>(EPlatformKeyboardKeyState::Release) == GLFW_RELEASE);
    static_assert(static_cast<Int32>(EPlatformKeyboardKeyState::Press) == GLFW_PRESS);

    /** Keyboard modifier flags (maps to GLFW_MOD_*). Use operator| to combine. */
    enum class EKeyboardModifier : UInt8
    {
        None     = 0,
        Shift    = GLFW_MOD_SHIFT,
        Control  = GLFW_MOD_CONTROL,
        Alt      = GLFW_MOD_ALT,
        Super    = GLFW_MOD_SUPER,
        CapsLock = GLFW_MOD_CAPS_LOCK,
        NumLock  = GLFW_MOD_NUM_LOCK,
    };

    constexpr EKeyboardModifier operator|(EKeyboardModifier I_A, EKeyboardModifier I_B)
    { return static_cast<EKeyboardModifier>(static_cast<UInt8>(I_A) | static_cast<UInt8>(I_B)); }
    constexpr EKeyboardModifier operator&(EKeyboardModifier I_A, EKeyboardModifier I_B)
    { return static_cast<EKeyboardModifier>(static_cast<UInt8>(I_A) & static_cast<UInt8>(I_B)); }

    inline constexpr EPlatformKeyboardKey GLFWKeyboardKeys[]
    {
        EPlatformKeyboardKey::Space, EPlatformKeyboardKey::Apostrophe, EPlatformKeyboardKey::Comma,
        EPlatformKeyboardKey::Minus, EPlatformKeyboardKey::Period, EPlatformKeyboardKey::Slash,
        EPlatformKeyboardKey::Num0, EPlatformKeyboardKey::Num1, EPlatformKeyboardKey::Num2,
        EPlatformKeyboardKey::Num3, EPlatformKeyboardKey::Num4, EPlatformKeyboardKey::Num5,
        EPlatformKeyboardKey::Num6, EPlatformKeyboardKey::Num7, EPlatformKeyboardKey::Num8,
        EPlatformKeyboardKey::Num9, EPlatformKeyboardKey::Semicolon, EPlatformKeyboardKey::Equal,
        EPlatformKeyboardKey::A, EPlatformKeyboardKey::B, EPlatformKeyboardKey::C,
        EPlatformKeyboardKey::D, EPlatformKeyboardKey::E, EPlatformKeyboardKey::F,
        EPlatformKeyboardKey::G, EPlatformKeyboardKey::H, EPlatformKeyboardKey::I,
        EPlatformKeyboardKey::J, EPlatformKeyboardKey::K, EPlatformKeyboardKey::L,
        EPlatformKeyboardKey::M, EPlatformKeyboardKey::N, EPlatformKeyboardKey::O,
        EPlatformKeyboardKey::P, EPlatformKeyboardKey::Q, EPlatformKeyboardKey::R,
        EPlatformKeyboardKey::S, EPlatformKeyboardKey::T, EPlatformKeyboardKey::U,
        EPlatformKeyboardKey::V, EPlatformKeyboardKey::W, EPlatformKeyboardKey::X,
        EPlatformKeyboardKey::Y, EPlatformKeyboardKey::Z,
        EPlatformKeyboardKey::LeftBracket, EPlatformKeyboardKey::Backslash,
        EPlatformKeyboardKey::RightBracket, EPlatformKeyboardKey::GraveAccent,
        EPlatformKeyboardKey::World1, EPlatformKeyboardKey::World2,
        EPlatformKeyboardKey::Escape, EPlatformKeyboardKey::Enter, EPlatformKeyboardKey::Tab,
        EPlatformKeyboardKey::Backspace, EPlatformKeyboardKey::Insert, EPlatformKeyboardKey::Delete,
        EPlatformKeyboardKey::Right, EPlatformKeyboardKey::Left, EPlatformKeyboardKey::Down,
        EPlatformKeyboardKey::Up, EPlatformKeyboardKey::PageUp, EPlatformKeyboardKey::PageDown,
        EPlatformKeyboardKey::Home, EPlatformKeyboardKey::End,
        EPlatformKeyboardKey::CapsLock, EPlatformKeyboardKey::ScrollLock, EPlatformKeyboardKey::NumLock,
        EPlatformKeyboardKey::PrintScreen, EPlatformKeyboardKey::Pause,
        EPlatformKeyboardKey::F1, EPlatformKeyboardKey::F2, EPlatformKeyboardKey::F3,
        EPlatformKeyboardKey::F4, EPlatformKeyboardKey::F5, EPlatformKeyboardKey::F6,
        EPlatformKeyboardKey::F7, EPlatformKeyboardKey::F8, EPlatformKeyboardKey::F9,
        EPlatformKeyboardKey::F10, EPlatformKeyboardKey::F11, EPlatformKeyboardKey::F12,
        EPlatformKeyboardKey::F13, EPlatformKeyboardKey::F14, EPlatformKeyboardKey::F15,
        EPlatformKeyboardKey::F16, EPlatformKeyboardKey::F17, EPlatformKeyboardKey::F18,
        EPlatformKeyboardKey::F19, EPlatformKeyboardKey::F20, EPlatformKeyboardKey::F21,
        EPlatformKeyboardKey::F22, EPlatformKeyboardKey::F23, EPlatformKeyboardKey::F24,
        EPlatformKeyboardKey::F25,
        EPlatformKeyboardKey::KP0, EPlatformKeyboardKey::KP1, EPlatformKeyboardKey::KP2,
        EPlatformKeyboardKey::KP3, EPlatformKeyboardKey::KP4, EPlatformKeyboardKey::KP5,
        EPlatformKeyboardKey::KP6, EPlatformKeyboardKey::KP7, EPlatformKeyboardKey::KP8,
        EPlatformKeyboardKey::KP9,
        EPlatformKeyboardKey::KPDecimal, EPlatformKeyboardKey::KPDivide,
        EPlatformKeyboardKey::KPMultiply, EPlatformKeyboardKey::KPSubtract,
        EPlatformKeyboardKey::KPAdd, EPlatformKeyboardKey::KPEnter, EPlatformKeyboardKey::KPEqual,
        EPlatformKeyboardKey::LeftShift, EPlatformKeyboardKey::LeftControl,
        EPlatformKeyboardKey::LeftAlt, EPlatformKeyboardKey::LeftSuper,
        EPlatformKeyboardKey::RightShift, EPlatformKeyboardKey::RightControl,
        EPlatformKeyboardKey::RightAlt, EPlatformKeyboardKey::RightSuper,
        EPlatformKeyboardKey::Menu,
    };

    inline constexpr Int32 GLFWKeyboardKeyLast = GLFW_KEY_MENU;
}
