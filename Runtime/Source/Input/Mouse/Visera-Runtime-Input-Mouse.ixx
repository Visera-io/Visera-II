module;
#include <Visera-Runtime.hpp>
//#include <GLFW/glfw3.h>
export module Visera.Runtime.Input.Mouse;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate.Multicast;
import Visera.Core.Math.Algebra.Vector;

namespace Visera
{
    export class VISERA_RUNTIME_API FMouse // Onto Mapping to GLFW
    {
    public:
        enum class EAction : Int32
        {
            Release = 0,	    // Released
            Press   = 1,	    // Just Pressed
            Hold    = 2,	    // Pressed and Holding
            Detach  = Hold + 1, // Just Released  (a special Release action)
        };

        enum class EButton : Int32
        {
            Left    = 0,
            Middle  = 1,
            Right   = 2,
        };

        using FButtonEvent = TMulticastDelegate<EButton>;
        FButtonEvent     OnPressed;
        FButtonEvent     OnReleased;
        FButtonEvent     OnHeld;

        using FScrollEvent = TMulticastDelegate<Float, Float>;
        FScrollEvent     OnScrolled;

        using FCursorMoveEvent = TMulticastDelegate<Float, Float>;
        FCursorMoveEvent OnCursorMoved;

        union FPosition
        {
            struct { Float X, Y; };
            FVector2F Vector{0, 0};
        };
        [[nodiscard]] inline const FPosition&
        GetPosition() const { return Position; }
        [[nodiscard]] inline const FPosition&
        GetOffset()   const { return Offset; }

    private:
        FPosition Position{0,0};
        FPosition Offset  {0,0};

    public:
        FMouse()
        {
            OnCursorMoved.Subscribe([this](Float I_X, Float I_Y)
            {
                Offset.X = I_X - Position.X;
                Offset.Y = I_Y - Position.Y;
                Position.X = I_X;
                Position.Y = I_Y;
            });
        }
    };
}