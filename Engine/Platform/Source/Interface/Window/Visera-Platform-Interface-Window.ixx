module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.Window;
#define VISERA_MODULE_NAME "Platform.Window"
export import Visera.Core.Types.String;
export import Visera.Core.Types.Pointer.Unique;
       import Visera.Core.Delegate.Unicast;

export namespace Visera
{
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
        /* void(Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods) */ TUnicastDelegate<void(Int32, Int32, Int32, Int32)>
        KeyboardCallback;
        /* void(Int32 I_Button, Int32 I_Action, Int32 I_Mods) */ TUnicastDelegate<void(Int32, Int32, Int32)>
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
        WaitEvents() const  = 0;
        virtual void
        PollEvents() const  = 0;
        virtual void
        SetSize(Int32 I_NewWidth, Int32 I_NewHeight) = 0;
        virtual void
        SetPosition(Int32 I_X, Int32 I_Y) const = 0;
        virtual void
        SetTitle(FStringView I_Title) = 0;
        virtual void
        SetIcon(const FIconSet& I_IconSet) = 0;
        [[nodiscard]] virtual Int32
        GetKeyboardKey(Int32 I_Key) const = 0;
        [[nodiscard]] virtual Int32
        GetMouseButton(Int32 I_Button) const = 0;
        [[nodiscard]] inline FStringView
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
        explicit IPlatformWindow(FStringView I_Title, UInt32 I_Width, UInt32 I_Height)
        : Title(I_Title), Width(I_Width), Height(I_Height) {}
        virtual ~IPlatformWindow() = default;

    protected:
        FString     Title;
        UInt32      Width      {0},     Height{0};
        Float       ScaleX     {1.0f},  ScaleY{1.0f};
        Bool        bMaximized {False};
    };
}
