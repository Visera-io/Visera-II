module;
#include <Visera-Input.hpp>
export module Visera.Runtime.Input.Device.Mouse;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate.Multicast;
import Visera.Core.Math.Algebra.Vector;
import Visera.Core.OS.Time;
import Visera.Platform;

export namespace Visera
{
    class VISERA_RUNTIME_API FMouse
    {
    public:
        using EButton = EPlatformMouseButton;

        static constexpr Int32 FirstButton = static_cast<Int32>(EButton::Left);
        static constexpr Int32 LastButton  = static_cast<Int32>(EButton::Button8);

        enum class EAction : Int32
        {
            Release = 0,	    // Released
            Press   = 1,	    // Just Pressed
            Hold    = 2,	    // Pressed and Holding
            Detach  = Hold + 1, // Just Released  (a special Release action)
        };

        struct FCursor
        {
            FVector2F Position {0,0};
            FVector2F Offset   {0,0};

            TMulticastDelegate<const FCursor&>
            OnMoved;
        };

        struct FButton
        {
            EAction           Action  {EAction::Release};
            /** Time point when button was pressed; used by FInput to compute HoldDuration. */
            FHighResTimePoint PressedAt {};
            /** Duration the current Action has been active, in seconds. */
            Float             HoldDuration {0};

            TMulticastDelegate<const FButton&>
            OnPressed;
            TMulticastDelegate<const FButton&>
            OnReleased;
            TMulticastDelegate<const FButton&>
            OnHeld;
        };

        struct FScroll
        {
            FVector2F Offset {0,0};

            TMulticastDelegate<const FScroll&>
            OnScrolled;
        };

        [[nodiscard]] FCursor& GetCursor() { return Cursor; }
        [[nodiscard]] const FCursor& GetCursor() const { return Cursor; }
        [[nodiscard]] FScroll& GetScroll() { return Scroll; }
        [[nodiscard]] const FScroll& GetScroll() const { return Scroll; }
        [[nodiscard]] FButton& GetButton(EButton I_Button)
        {
            const auto Button = static_cast<Int32>(I_Button);
            VISERA_ASSERT(Button >= FirstButton && Button <= LastButton);
            return Buttons[Button];
        }
        [[nodiscard]] const FButton& GetButton(EButton I_Button) const
        {
            const auto Button = static_cast<Int32>(I_Button);
            VISERA_ASSERT(Button >= FirstButton && Button <= LastButton);
            return Buttons[Button];
        }

    private:
        FCursor Cursor;
        FButton Buttons[LastButton + 1];
        FScroll Scroll;
    };
}