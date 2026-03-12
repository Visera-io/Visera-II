module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.Window;
#define VISERA_MODULE_NAME "Platform.Interface"
export import Visera.Platform.Interface.Device;
export import Visera.Core.Types.String;
export import Visera.Core.Types.Pointer.Unique;
       import Visera.Core.Delegate.Unicast;
       import Visera.Core.Types.Text;

export namespace Visera
{
    /** Platform window abstraction. Input query and callbacks use Interface.Device enums so
        Runtime.Input stays platform-agnostic; Platform layer performs any cast at the boundary. */
    class VISERA_PLATFORM_API IPlatformWindow
    {
    public:
        struct FIconSet
        {
            const FByte* Icon16x16    = nullptr;
            const FByte* Icon32x32    = nullptr;
            const FByte* Icon48x48    = nullptr;
            const FByte* Icon64x64    = nullptr;
            const FByte* Icon128x128  = nullptr;
            const FByte* Icon256x256  = nullptr;
        };
        /* void(Float I_ScaleX, Float I_ScaleY) */ TUnicastDelegate<void(Float, Float)>
        WindowContentScaleCallback;
        /** Key, platform scancode, action (Press/Release), modifier flags. Types from Interface.Device. */
        TUnicastDelegate<void(EPlatformKeyboardKey I_Key, Int32 I_ScanCode, EPlatformKeyboardKeyState I_Action, EPlatformKeyboardModifier I_Mods)>
        KeyboardCallback;
        /** Button, action (Press/Release), modifier flags. Types from Interface.Device. */
        TUnicastDelegate<void(EPlatformMouseButton I_Button, EPlatformMouseButtonState I_Action, EPlatformKeyboardModifier I_Mods)>
        MouseButtonCallback;
        /* void(Double I_PosX, Double I_PosY) */ TUnicastDelegate<void(Double, Double)>
        CursorMoveCallback;
        /* void(Double I_OffsetX,  Double I_OffsetY) */ TUnicastDelegate<void(Double, Double)>
        ScrollCallback;
        /* void(Int32 I_Width, Int32 I_Height) */ TUnicastDelegate<void(Int32, Int32)>
        WindowResizeCallback;

    public:
        [[nodiscard]] virtual void*
        GetHandle() const = 0;
        [[nodiscard]] virtual Bool
        ShouldClose() const = 0;
        virtual void
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) = 0;
        virtual void
        SetPosition(Int32 I_X, Int32 I_Y) const = 0;
        virtual void
        SetTitle(const FText& I_Title) = 0;
        virtual void
        SetIcon(const FIconSet& I_IconSet) = 0;
        /** Current key state for this window. Platform-agnostic enum; cast to platform int only in Platform layer. */
        [[nodiscard]] virtual EPlatformKeyboardKeyState
        QueryKeyboardKeyState(EPlatformKeyboardKey I_Key) const = 0;
        /** Current mouse button state for this window. */
        [[nodiscard]] virtual EPlatformMouseButtonState
        QueryMouseButtonState(EPlatformMouseButton I_Button) const = 0;
        /** Fill caller-owned keyboard state table. Index = EPlatformKeyboardKey value; table length kKeyboardStateTableSize. */
        virtual void
        QueryKeyboardState(TSpan<EPlatformKeyboardKeyState, kKeyboardStateTableSize> O_Out) const = 0;
        /** Fill caller-owned mouse button state table. Index = button 0..7; table length kMouseButtonStateTableSize. */
        virtual void
        QueryMouseButtonState(TSpan<EPlatformMouseButtonState, kMouseButtonStateTableSize> O_Out) const = 0;
        /** Native window handle (e.g. HWND on Windows, NSWindow* on MacOS). nullptr if not available. */
        [[nodiscard]] virtual void*
        GetNativeHandle() const = 0;
        [[nodiscard]] virtual void*
        CreateVulkanSurface(void* I_Instance) const = 0;
        [[nodiscard]] inline const FText&
        GetTitle() const { return Title; }
        [[nodiscard]] inline UInt32
        GetWidth() const  { return Width; }
        [[nodiscard]] inline UInt32
        GetHeight() const { return Height; }
        [[nodiscard]] inline Float
        GetScaleX() const  { return ScaleX; }
        [[nodiscard]] inline Float
        GetScaleY() const { return ScaleY; }
        [[nodiscard]] inline Bool
        IsInitialized() const { return Width && Height; }
        [[nodiscard]] inline Bool
        IsMaximized() const { return bMaximized; };

        explicit IPlatformWindow() = delete;
        explicit IPlatformWindow(const FText& I_Title, UInt32 I_Width, UInt32 I_Height)
        : Title(I_Title), Width(I_Width), Height(I_Height) {}
        virtual ~IPlatformWindow() = default;

    protected:
        FText       Title;
        UInt32      Width      {0},     Height{0};
        Float       ScaleX     {1.0f},  ScaleY{1.0f};
        Bool        bMaximized {False};
    };
}
