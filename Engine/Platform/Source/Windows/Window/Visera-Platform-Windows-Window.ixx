module;
#include <Visera-Platform.hpp>
#if defined(VISERA_ON_WINDOWS_SYSTEM)
#include <windows.h>
#endif
export module Visera.Platform.Windows.Window;
#define VISERA_MODULE_NAME "Platform.Windows"
export import Visera.Platform.GLFW.Window;
import Visera.Platform.Interface.Device;
import Visera.Platform.Interface.Window;
import Visera.Core.Types.Text;

export namespace Visera
{
    /**
     * Windows platform window: inherits FGLFWWindow (GLFW creates the window, GetNativeHandle returns HWND).
     * Overrides QueryKeyboardState / QueryMouseButtonState to fill the state tables inside this module
     * using GetKeyboardState + VK→EPlatformKeyboardKey mapping and GetAsyncKeyState for mouse,
     * so that GLFW stays free of Windows-specific code.
     */
    class VISERA_PLATFORM_API FWindowsWindow : public FGLFWWindow
    {
    public:
        void
        QueryKeyboardState(TSpan<EPlatformKeyboardKeyState, kKeyboardStateTableSize> O_Out) const override;
        void
        QueryMouseButtonState(TSpan<EPlatformMouseButtonState, kMouseButtonStateTableSize> O_Out) const override;

        FWindowsWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height)
        : FGLFWWindow(I_Title, I_Width, I_Height)
        {}
    };
}

namespace Visera
{
    /** VK → EPlatformKeyboardKey (GLFW-aligned). Built once; unmapped VKs stay Unknown. */
    const EPlatformKeyboardKey* GetKeyboardKeyMapping()
    {
        static EPlatformKeyboardKey Table[256];
        static Bool Once = []
        {
            for (int i = 0; i < 256; ++i)
            { Table[i] = EPlatformKeyboardKey::Unknown; }
            Table[VK_BACK]      = EPlatformKeyboardKey::Backspace;
            Table[VK_TAB]       = EPlatformKeyboardKey::Tab;
            Table[VK_RETURN]    = EPlatformKeyboardKey::Enter;
            Table[VK_ESCAPE]    = EPlatformKeyboardKey::Escape;
            Table[VK_SPACE]     = EPlatformKeyboardKey::Space;
            Table[VK_PRIOR]     = EPlatformKeyboardKey::PageUp;
            Table[VK_NEXT]      = EPlatformKeyboardKey::PageDown;
            Table[VK_END]       = EPlatformKeyboardKey::End;
            Table[VK_HOME]      = EPlatformKeyboardKey::Home;
            Table[VK_LEFT]      = EPlatformKeyboardKey::Left;
            Table[VK_UP]        = EPlatformKeyboardKey::Up;
            Table[VK_RIGHT]     = EPlatformKeyboardKey::Right;
            Table[VK_DOWN]      = EPlatformKeyboardKey::Down;
            Table[VK_INSERT]    = EPlatformKeyboardKey::Insert;
            Table[VK_DELETE]    = EPlatformKeyboardKey::Delete;
            for (int i = 0; i <= 9; ++i)
            { Table['0' + i]    = static_cast<EPlatformKeyboardKey>(static_cast<Int32>(EPlatformKeyboardKey::Num0) + i); }
            for (int i = 0; i < 26; ++i)
            { Table['A' + i]    = static_cast<EPlatformKeyboardKey>(static_cast<Int32>(EPlatformKeyboardKey::A) + i); }
            Table[VK_OEM_1]     = EPlatformKeyboardKey::Semicolon;
            Table[VK_OEM_PLUS]  = EPlatformKeyboardKey::Equal;
            Table[VK_OEM_COMMA] = EPlatformKeyboardKey::Comma;
            Table[VK_OEM_MINUS] = EPlatformKeyboardKey::Minus;
            Table[VK_OEM_PERIOD]= EPlatformKeyboardKey::Period;
            Table[VK_OEM_2]     = EPlatformKeyboardKey::Slash;
            Table[VK_OEM_3]     = EPlatformKeyboardKey::GraveAccent;
            Table[VK_OEM_4]     = EPlatformKeyboardKey::LeftBracket;
            Table[VK_OEM_5]     = EPlatformKeyboardKey::Backslash;
            Table[VK_OEM_6]     = EPlatformKeyboardKey::RightBracket;
            Table[VK_OEM_7]     = EPlatformKeyboardKey::Apostrophe;
            Table[VK_NUMPAD0]   = EPlatformKeyboardKey::KP0;
            Table[VK_NUMPAD1]   = EPlatformKeyboardKey::KP1;
            Table[VK_NUMPAD2]   = EPlatformKeyboardKey::KP2;
            Table[VK_NUMPAD3]   = EPlatformKeyboardKey::KP3;
            Table[VK_NUMPAD4]   = EPlatformKeyboardKey::KP4;
            Table[VK_NUMPAD5]   = EPlatformKeyboardKey::KP5;
            Table[VK_NUMPAD6]   = EPlatformKeyboardKey::KP6;
            Table[VK_NUMPAD7]   = EPlatformKeyboardKey::KP7;
            Table[VK_NUMPAD8]   = EPlatformKeyboardKey::KP8;
            Table[VK_NUMPAD9]   = EPlatformKeyboardKey::KP9;
            Table[VK_DECIMAL]   = EPlatformKeyboardKey::KPDecimal;
            Table[VK_DIVIDE]    = EPlatformKeyboardKey::KPDivide;
            Table[VK_MULTIPLY]  = EPlatformKeyboardKey::KPMultiply;
            Table[VK_SUBTRACT]  = EPlatformKeyboardKey::KPSubtract;
            Table[VK_ADD]       = EPlatformKeyboardKey::KPAdd;
            for (int i = 0; i <= 24; ++i)
            { Table[VK_F1 + i]  = static_cast<EPlatformKeyboardKey>(static_cast<Int32>(EPlatformKeyboardKey::F1) + i); }
            Table[VK_LSHIFT]    = EPlatformKeyboardKey::LeftShift;
            Table[VK_RSHIFT]    = EPlatformKeyboardKey::RightShift;
            Table[VK_LCONTROL]  = EPlatformKeyboardKey::LeftControl;
            Table[VK_RCONTROL]  = EPlatformKeyboardKey::RightControl;
            Table[VK_LMENU]     = EPlatformKeyboardKey::LeftAlt;
            Table[VK_RMENU]     = EPlatformKeyboardKey::RightAlt;
            Table[VK_LWIN]      = EPlatformKeyboardKey::LeftSuper;
            Table[VK_RWIN]      = EPlatformKeyboardKey::RightSuper;
            Table[VK_APPS]      = EPlatformKeyboardKey::Menu;
            Table[VK_CAPITAL]   = EPlatformKeyboardKey::CapsLock;
            Table[VK_SCROLL]    = EPlatformKeyboardKey::ScrollLock;
            Table[VK_NUMLOCK]   = EPlatformKeyboardKey::NumLock;
            Table[VK_SNAPSHOT]  = EPlatformKeyboardKey::PrintScreen;
            Table[VK_PAUSE]     = EPlatformKeyboardKey::Pause;
            return True;
        }();
        return Table;
    }

    void FWindowsWindow::
    QueryKeyboardState(TSpan<EPlatformKeyboardKeyState, kKeyboardStateTableSize> O_Out) const
    {
        for (size_t i = 0; i < kKeyboardStateTableSize; ++i)
        { O_Out[i] = EPlatformKeyboardKeyState::Release; }
        UInt8 state[256];
        if (!GetKeyboardState(state))
        { return; }
        const EPlatformKeyboardKey* Map = GetKeyboardKeyMapping();
        for (int vk = 0; vk < 256; ++vk)
        {
            EPlatformKeyboardKey key = Map[vk];
            if (key == EPlatformKeyboardKey::Unknown)
            { continue; }
            auto idx = static_cast<size_t>(static_cast<Int32>(key));
            if (idx < kKeyboardStateTableSize)
            { O_Out[idx] = (state[vk] & 0x80) ? EPlatformKeyboardKeyState::Press : EPlatformKeyboardKeyState::Release; }
        }
    }

    void FWindowsWindow::
    QueryMouseButtonState(TSpan<EPlatformMouseButtonState, kMouseButtonStateTableSize> O_Out) const
    {
        static const int VKButtons[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON, VK_XBUTTON1, VK_XBUTTON2 };
        constexpr size_t N = sizeof(VKButtons) / sizeof(VKButtons[0]);
        for (size_t i = 0; i < N && i < kMouseButtonStateTableSize; ++i)
        { O_Out[i] = (GetAsyncKeyState(VKButtons[i]) & 0x8000) ? EPlatformMouseButtonState::Press : EPlatformMouseButtonState::Release; }
        for (size_t i = N; i < kMouseButtonStateTableSize; ++i)
        { O_Out[i] = EPlatformMouseButtonState::Release; }
    }
}