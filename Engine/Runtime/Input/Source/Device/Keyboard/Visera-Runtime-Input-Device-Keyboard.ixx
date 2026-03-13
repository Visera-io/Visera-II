module;
#include <Visera-Input.hpp>
export module Visera.Runtime.Input.Device.Keyboard;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate;
import Visera.Core.OS.Time;
import Visera.Platform;

export namespace Visera
{
    class VISERA_RUNTIME_API FKeyboard
    {
    public:
        using EKey = EPlatformKeyboardKey;
        static constexpr auto FirstKey = static_cast<Int32>(EKey::Space);
        static constexpr auto LastKey  = static_cast<Int32>(EKey::Menu);
        static constexpr auto KeyCount = static_cast<UInt32>(LastKey - FirstKey + 1);

        enum class EAction : UInt8
        {
            Release = static_cast<UInt8>(EPlatformKeyboardAction::Release),
            Press   = static_cast<UInt8>(EPlatformKeyboardAction::Press),
            Hold,	// Pressed and Holding
        };

        struct FKey
        {
            EAction           Action  {EAction::Release};
            /** Time point when key was pressed; used by FInput to compute HoldDuration. */
            FHighResTimePoint PressedAt {};
            /** Duration the current Action has been active, in seconds. */
            Float             HoldDuration {0};

            TMulticastDelegate<const FKey&>
            OnPressed;
            TMulticastDelegate<const FKey&>
            OnReleased;
            TMulticastDelegate<const FKey&>
            OnHeld;
        };

        [[nodiscard]] FKey& GetKey(EKey I_Key)
        {
            const auto Key = static_cast<Int32>(I_Key);
            VISERA_ASSERT(Key >= FirstKey && Key <= LastKey);
            return Keys[static_cast<UInt32>(Key - FirstKey)];
        }
        [[nodiscard]] const FKey& GetKey(EKey I_Key) const
        {
            const auto Key = static_cast<Int32>(I_Key);
            VISERA_ASSERT(Key >= FirstKey && Key <= LastKey);
            return Keys[static_cast<UInt32>(Key - FirstKey)];
        }

    private:
        FKey Keys[KeyCount];

    public:
        FKeyboard() = default;
    };
}