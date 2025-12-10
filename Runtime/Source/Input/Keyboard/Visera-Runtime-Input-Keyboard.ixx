module;
#include <Visera-Runtime.hpp>
//#include <GLFW/glfw3.h>
export module Visera.Runtime.Input.Keyboard;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate.Unicast;
import Visera.Core.Delegate.Multicast;
import Visera.Core.Traits.Policy;

namespace Visera
{
    export class VISERA_RUNTIME_API FKeyboard // Onto Mapping to GLFW
    {
    public:
        enum class EAction : UInt8
        {
            Release = 0,	    // Released
            Press   = 1,	    // Just Pressed
            Hold    = 2,	    // Pressed and Holding
            Detach  = Hold + 1, // Just Released  (a special Release action)
        };

        enum class EKey : UInt16
        {
            A = 65, B = 66, C = 67, D = 68, E = 69, F = 70, G = 71,
            H = 72, I = 73, J = 74, K = 75, L = 76, M = 77, N = 78,
            O = 79, P = 80, Q = 81, R = 82, S = 83, T = 84, U = 85,
            V = 86, W = 87, X = 88, Y = 89, Z = 90,

            //Numbers
            Num0 = 48,  Num1 = 49, Num2 = 50, Num3 = 51, Num4 = 52,
            Num5 = 53,  Num6 = 54, Num7 = 55, Num8 = 56, Num9 = 57,

            //Fn
            F1 = 290,   F2 = 291,   F3 = 292,   F4 = 293,
            F5 = 294,   F6 = 295,   F7 = 296,   F8 = 297,
            F9 = 298,   F10= 299,   F11= 300,   F12= 301,

            //Others
            Space    = 32,
            LShift   = 340,
            RShift   = 344,
            ESC      = 256,
            Enter    = 257,
            Tab	     = 258,
            CapsLock = 280,
            LCtrl    = 341,
            RCtrl    = 345,
            LAlt     = 342,
            RAlt     = 346,
            Menu     = 348, // Max Key
        };
        static constexpr UInt16
        MaxKey = static_cast<UInt16>(EKey::Menu);

        [[nodiscard]] inline EAction
        GetKeyAction(EKey I_Key) const
        {
            const auto Key = static_cast<UInt32>(I_Key);
            EAction Status{ EAction::Release };
            OnGetKey.Invoke(I_Key, &Status);
            return Status;
        }
        [[nodiscard]] inline Bool
        IsPressed(EKey I_Key)  const {  return GetKeyAction(I_Key) == EAction::Press; }
        [[nodiscard]] inline Bool
        IsReleased(EKey I_Key) const {  return GetKeyAction(I_Key) == EAction::Release; }

        using FKeyEvent = TMulticastDelegate<EKey>;
        FKeyEvent OnPressed;
        FKeyEvent OnReleased;
        FKeyEvent OnHeld;
        FKeyEvent OnDetached;

        using FCheckKeyStatusEvent = TUnicastDelegate<void(EKey, EAction*), Policy::Exclusive>;
        FCheckKeyStatusEvent OnGetKey;

    private:
        EAction Keys[MaxKey + 1] {};

    public:
        FKeyboard()
        {

        }
    };
}