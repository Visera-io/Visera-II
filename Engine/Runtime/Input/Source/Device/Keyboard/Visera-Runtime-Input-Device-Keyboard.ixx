module;
#include <Visera-Input.hpp>
export module Visera.Runtime.Input.Device.Keyboard;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate;
import Visera.Platform.Cross.GLFW.Keyboard;

export namespace Visera
{
    class VISERA_RUNTIME_API FKeyboard
    {
    public:
        static constexpr auto& Keys = GLFWKeyboardKeys;
        using EKey = EGLFWKeyboardKey;
        enum class EAction : UInt8
        {
            Release = static_cast<UInt8>(EGLFWKeyboardAction::Release),
            Press   = static_cast<UInt8>(EGLFWKeyboardAction::Press),
            Hold,	// Pressed and Holding
        };
        static constexpr Int32 MaxKey = GLFWKeyboardKeyLast;

        enum class EKeyStatus : UInt8
        { Up, Down };

        /** Call each frame after PollEvents. I_GetKeyState(key) returns GLFW_PRESS (1) or GLFW_RELEASE (0). */
        template<typename F>
        void Sync(F&& I_GetKeyState)
        {
            for (const auto Key : Keys)
            {
                const auto Idx = static_cast<UInt32>(Key); VISERA_ASSERT(Idx <= MaxKey);

                const Bool PrevDown = (KeyStatus[Idx]     == EKeyStatus::Down);
                const Bool CurrDown = (I_GetKeyState(Key) == EAction::Press);

                if (!PrevDown && CurrDown)
                    KeyActions[Idx] = EAction::Press;
                else if (PrevDown && !CurrDown)
                    KeyActions[Idx] = EAction::Release;
                else if (PrevDown && CurrDown)
                    KeyActions[Idx] = EAction::Hold;
                else
                    KeyActions[Idx] = EAction::Release;
                KeyStatus[Idx] = CurrDown? EKeyStatus::Down : EKeyStatus::Up;
            }
        }

        [[nodiscard]] inline EAction
        GetKeyAction(EKey I_Key) const
        {
            const auto Key = static_cast<Int32>(I_Key);
            VISERA_ASSERT(Key >= 0 && Key <= MaxKey);
            return KeyActions[static_cast<UInt32>(Key)];
        }
        [[nodiscard]] inline Bool
        IsPressed(EKey I_Key)  const { return GetKeyAction(I_Key) == EAction::Press; }
        [[nodiscard]] inline Bool
        IsReleased(EKey I_Key) const { return GetKeyAction(I_Key) == EAction::Release; }

        using FKeyEvent = TMulticastDelegate<EKey>;
        FKeyEvent OnPressed;
        FKeyEvent OnReleased;
        FKeyEvent OnHeld;
        FKeyEvent OnDetached;

    private:
        EAction    KeyActions [MaxKey + 1]{};
        EKeyStatus KeyStatus  [MaxKey + 1]{};

    public:
        FKeyboard() = default;
    };
}