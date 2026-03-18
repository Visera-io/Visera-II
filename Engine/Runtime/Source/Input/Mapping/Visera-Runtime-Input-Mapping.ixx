module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Input.Mapping;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate.Multicast;
import Visera.Core.Types.Name;
import Visera.Core.Types.JSON;
import Visera.Core.Types.Optional;
import Visera.Core.Types.String;
import Visera.Core.Containers.Array;
import Visera.Core.Log;
import Visera.Platform;

export namespace Visera
{
    /** Physical input source for a mapping (UE5: key vs button). */
    enum class EInputSource : UInt8
    {
        KeyboardKey,
        MouseButton,
    };

    /** When the mapping triggers (UE5: Pressed, Released, Hold). */
    enum class EInputTrigger : UInt8
    {
        Press   = 1,
        Release = 0,
        Hold    = 2,
    };

    /**
     * Single mapping: physical input -> action (UE5 Enhanced Input style).
     * Maps a key or mouse button + trigger + modifiers to an abstract action name.
     */
    struct VISERA_RUNTIME_API FInputMapping
    {
        FName            ActionName;
        EInputSource     SourceType;
        Int32            SourceValue;  // FKeyboard::EKey or FMouse::EButton as Int32
        EInputTrigger    Trigger;
        EKeyboardModifier Modifiers{EKeyboardModifier::None};

        [[nodiscard]] Bool MatchesKey(Int32 I_Key, UInt8 I_Action, UInt8 I_Mods) const
        {
            if (SourceType != EInputSource::KeyboardKey) { return False; }
            if (SourceValue != I_Key) { return False; }
            if (static_cast<UInt8>(Trigger) != I_Action) { return False; }
            const auto ModMask = static_cast<UInt8>(Modifiers);
            return (ModMask == 0) || ((I_Mods & ModMask) == ModMask);
        }

        [[nodiscard]] Bool MatchesButton(Int32 I_Button, UInt8 I_Action, UInt8 I_Mods) const
        {
            if (SourceType != EInputSource::MouseButton) { return False; }
            if (SourceValue != I_Button) { return False; }
            if (static_cast<UInt8>(Trigger) != I_Action) { return False; }
            const auto ModMask = static_cast<UInt8>(Modifiers);
            return (ModMask == 0) || ((I_Mods & ModMask) == ModMask);
        }
    };

    namespace Detail
    {
        inline TOptional<Int32> ParseKeyName(FStringView I_Name)
        {
            static constinit struct { const char* Name; Int32 Value; } const KeyTable[]{
                    {"Unknown", static_cast<Int32>(EPlatformKeyboardKey::Unknown)},
                    {"Space", static_cast<Int32>(EPlatformKeyboardKey::Space)},
                    {"Apostrophe", static_cast<Int32>(EPlatformKeyboardKey::Apostrophe)},
                    {"Comma", static_cast<Int32>(EPlatformKeyboardKey::Comma)},
                    {"Minus", static_cast<Int32>(EPlatformKeyboardKey::Minus)},
                    {"Period", static_cast<Int32>(EPlatformKeyboardKey::Period)},
                    {"Slash", static_cast<Int32>(EPlatformKeyboardKey::Slash)},
                    {"Num0", static_cast<Int32>(EPlatformKeyboardKey::Num0)},
                    {"Num1", static_cast<Int32>(EPlatformKeyboardKey::Num1)},
                    {"Num2", static_cast<Int32>(EPlatformKeyboardKey::Num2)},
                    {"Num3", static_cast<Int32>(EPlatformKeyboardKey::Num3)},
                    {"Num4", static_cast<Int32>(EPlatformKeyboardKey::Num4)},
                    {"Num5", static_cast<Int32>(EPlatformKeyboardKey::Num5)},
                    {"Num6", static_cast<Int32>(EPlatformKeyboardKey::Num6)},
                    {"Num7", static_cast<Int32>(EPlatformKeyboardKey::Num7)},
                    {"Num8", static_cast<Int32>(EPlatformKeyboardKey::Num8)},
                    {"Num9", static_cast<Int32>(EPlatformKeyboardKey::Num9)},
                    {"Semicolon", static_cast<Int32>(EPlatformKeyboardKey::Semicolon)},
                    {"Equal", static_cast<Int32>(EPlatformKeyboardKey::Equal)},
                    {"A", static_cast<Int32>(EPlatformKeyboardKey::A)},
                    {"B", static_cast<Int32>(EPlatformKeyboardKey::B)},
                    {"C", static_cast<Int32>(EPlatformKeyboardKey::C)},
                    {"D", static_cast<Int32>(EPlatformKeyboardKey::D)},
                    {"E", static_cast<Int32>(EPlatformKeyboardKey::E)},
                    {"F", static_cast<Int32>(EPlatformKeyboardKey::F)},
                    {"G", static_cast<Int32>(EPlatformKeyboardKey::G)},
                    {"H", static_cast<Int32>(EPlatformKeyboardKey::H)},
                    {"I", static_cast<Int32>(EPlatformKeyboardKey::I)},
                    {"J", static_cast<Int32>(EPlatformKeyboardKey::J)},
                    {"K", static_cast<Int32>(EPlatformKeyboardKey::K)},
                    {"L", static_cast<Int32>(EPlatformKeyboardKey::L)},
                    {"M", static_cast<Int32>(EPlatformKeyboardKey::M)},
                    {"N", static_cast<Int32>(EPlatformKeyboardKey::N)},
                    {"O", static_cast<Int32>(EPlatformKeyboardKey::O)},
                    {"P", static_cast<Int32>(EPlatformKeyboardKey::P)},
                    {"Q", static_cast<Int32>(EPlatformKeyboardKey::Q)},
                    {"R", static_cast<Int32>(EPlatformKeyboardKey::R)},
                    {"S", static_cast<Int32>(EPlatformKeyboardKey::S)},
                    {"T", static_cast<Int32>(EPlatformKeyboardKey::T)},
                    {"U", static_cast<Int32>(EPlatformKeyboardKey::U)},
                    {"V", static_cast<Int32>(EPlatformKeyboardKey::V)},
                    {"W", static_cast<Int32>(EPlatformKeyboardKey::W)},
                    {"X", static_cast<Int32>(EPlatformKeyboardKey::X)},
                    {"Y", static_cast<Int32>(EPlatformKeyboardKey::Y)},
                    {"Z", static_cast<Int32>(EPlatformKeyboardKey::Z)},
                    {"LeftBracket", static_cast<Int32>(EPlatformKeyboardKey::LeftBracket)},
                    {"Backslash", static_cast<Int32>(EPlatformKeyboardKey::Backslash)},
                    {"RightBracket", static_cast<Int32>(EPlatformKeyboardKey::RightBracket)},
                    {"GraveAccent", static_cast<Int32>(EPlatformKeyboardKey::GraveAccent)},
                    {"World1", static_cast<Int32>(EPlatformKeyboardKey::World1)},
                    {"World2", static_cast<Int32>(EPlatformKeyboardKey::World2)},
                    {"Escape", static_cast<Int32>(EPlatformKeyboardKey::Escape)},
                    {"Enter", static_cast<Int32>(EPlatformKeyboardKey::Enter)},
                    {"Tab", static_cast<Int32>(EPlatformKeyboardKey::Tab)},
                    {"Backspace", static_cast<Int32>(EPlatformKeyboardKey::Backspace)},
                    {"Insert", static_cast<Int32>(EPlatformKeyboardKey::Insert)},
                    {"Delete", static_cast<Int32>(EPlatformKeyboardKey::Delete)},
                    {"Right", static_cast<Int32>(EPlatformKeyboardKey::Right)},
                    {"Left", static_cast<Int32>(EPlatformKeyboardKey::Left)},
                    {"Down", static_cast<Int32>(EPlatformKeyboardKey::Down)},
                    {"Up", static_cast<Int32>(EPlatformKeyboardKey::Up)},
                    {"PageUp", static_cast<Int32>(EPlatformKeyboardKey::PageUp)},
                    {"PageDown", static_cast<Int32>(EPlatformKeyboardKey::PageDown)},
                    {"Home", static_cast<Int32>(EPlatformKeyboardKey::Home)},
                    {"End", static_cast<Int32>(EPlatformKeyboardKey::End)},
                    {"CapsLock", static_cast<Int32>(EPlatformKeyboardKey::CapsLock)},
                    {"ScrollLock", static_cast<Int32>(EPlatformKeyboardKey::ScrollLock)},
                    {"NumLock", static_cast<Int32>(EPlatformKeyboardKey::NumLock)},
                    {"PrintScreen", static_cast<Int32>(EPlatformKeyboardKey::PrintScreen)},
                    {"Pause", static_cast<Int32>(EPlatformKeyboardKey::Pause)},
                    {"F1", static_cast<Int32>(EPlatformKeyboardKey::F1)},
                    {"F2", static_cast<Int32>(EPlatformKeyboardKey::F2)},
                    {"F3", static_cast<Int32>(EPlatformKeyboardKey::F3)},
                    {"F4", static_cast<Int32>(EPlatformKeyboardKey::F4)},
                    {"F5", static_cast<Int32>(EPlatformKeyboardKey::F5)},
                    {"F6", static_cast<Int32>(EPlatformKeyboardKey::F6)},
                    {"F7", static_cast<Int32>(EPlatformKeyboardKey::F7)},
                    {"F8", static_cast<Int32>(EPlatformKeyboardKey::F8)},
                    {"F9", static_cast<Int32>(EPlatformKeyboardKey::F9)},
                    {"F10", static_cast<Int32>(EPlatformKeyboardKey::F10)},
                    {"F11", static_cast<Int32>(EPlatformKeyboardKey::F11)},
                    {"F12", static_cast<Int32>(EPlatformKeyboardKey::F12)},
                    {"F13", static_cast<Int32>(EPlatformKeyboardKey::F13)},
                    {"F14", static_cast<Int32>(EPlatformKeyboardKey::F14)},
                    {"F15", static_cast<Int32>(EPlatformKeyboardKey::F15)},
                    {"F16", static_cast<Int32>(EPlatformKeyboardKey::F16)},
                    {"F17", static_cast<Int32>(EPlatformKeyboardKey::F17)},
                    {"F18", static_cast<Int32>(EPlatformKeyboardKey::F18)},
                    {"F19", static_cast<Int32>(EPlatformKeyboardKey::F19)},
                    {"F20", static_cast<Int32>(EPlatformKeyboardKey::F20)},
                    {"F21", static_cast<Int32>(EPlatformKeyboardKey::F21)},
                    {"F22", static_cast<Int32>(EPlatformKeyboardKey::F22)},
                    {"F23", static_cast<Int32>(EPlatformKeyboardKey::F23)},
                    {"F24", static_cast<Int32>(EPlatformKeyboardKey::F24)},
                    {"F25", static_cast<Int32>(EPlatformKeyboardKey::F25)},
                    {"KP0", static_cast<Int32>(EPlatformKeyboardKey::KP0)},
                    {"KP1", static_cast<Int32>(EPlatformKeyboardKey::KP1)},
                    {"KP2", static_cast<Int32>(EPlatformKeyboardKey::KP2)},
                    {"KP3", static_cast<Int32>(EPlatformKeyboardKey::KP3)},
                    {"KP4", static_cast<Int32>(EPlatformKeyboardKey::KP4)},
                    {"KP5", static_cast<Int32>(EPlatformKeyboardKey::KP5)},
                    {"KP6", static_cast<Int32>(EPlatformKeyboardKey::KP6)},
                    {"KP7", static_cast<Int32>(EPlatformKeyboardKey::KP7)},
                    {"KP8", static_cast<Int32>(EPlatformKeyboardKey::KP8)},
                    {"KP9", static_cast<Int32>(EPlatformKeyboardKey::KP9)},
                    {"KPDecimal", static_cast<Int32>(EPlatformKeyboardKey::KPDecimal)},
                    {"KPDivide", static_cast<Int32>(EPlatformKeyboardKey::KPDivide)},
                    {"KPMultiply", static_cast<Int32>(EPlatformKeyboardKey::KPMultiply)},
                    {"KPSubtract", static_cast<Int32>(EPlatformKeyboardKey::KPSubtract)},
                    {"KPAdd", static_cast<Int32>(EPlatformKeyboardKey::KPAdd)},
                    {"KPEnter", static_cast<Int32>(EPlatformKeyboardKey::KPEnter)},
                    {"KPEqual", static_cast<Int32>(EPlatformKeyboardKey::KPEqual)},
                    {"LeftShift", static_cast<Int32>(EPlatformKeyboardKey::LeftShift)},
                    {"LeftControl", static_cast<Int32>(EPlatformKeyboardKey::LeftControl)},
                    {"LeftAlt", static_cast<Int32>(EPlatformKeyboardKey::LeftAlt)},
                    {"LeftSuper", static_cast<Int32>(EPlatformKeyboardKey::LeftSuper)},
                    {"RightShift", static_cast<Int32>(EPlatformKeyboardKey::RightShift)},
                    {"RightControl", static_cast<Int32>(EPlatformKeyboardKey::RightControl)},
                    {"RightAlt", static_cast<Int32>(EPlatformKeyboardKey::RightAlt)},
                    {"RightSuper", static_cast<Int32>(EPlatformKeyboardKey::RightSuper)},
                    {"Menu", static_cast<Int32>(EPlatformKeyboardKey::Menu)},
                };
                const FStringView Sv = I_Name;
                for (const auto& E : KeyTable)
                {
                    if (Sv == FStringView(E.Name)) { return TOptional<Int32>(E.Value); }
                }
                return NullOpt;
        }

        inline TOptional<Int32> ParseButtonName(FStringView I_Name)
            {
                static constinit struct { const char* Name; Int32 Value; } const BtnTable[]{
                    {"Left", static_cast<Int32>(EPlatformMouseButton::Left)},
                    {"Right", static_cast<Int32>(EPlatformMouseButton::Right)},
                    {"Middle", static_cast<Int32>(EPlatformMouseButton::Middle)},
                    {"Button4", static_cast<Int32>(EPlatformMouseButton::Button4)},
                    {"Button5", static_cast<Int32>(EPlatformMouseButton::Button5)},
                    {"Button6", static_cast<Int32>(EPlatformMouseButton::Button6)},
                    {"Button7", static_cast<Int32>(EPlatformMouseButton::Button7)},
                    {"Button8", static_cast<Int32>(EPlatformMouseButton::Button8)},
                };
                const FStringView Sv = I_Name;
                for (const auto& E : BtnTable)
                {
                    if (Sv == FStringView(E.Name)) { return TOptional<Int32>(E.Value); }
                }
                return NullOpt;
        }

        inline EKeyboardModifier ParseModifiers(const FJSON& I_Obj)
        {
            EKeyboardModifier Out = EKeyboardModifier::None;
            auto ArrOpt = I_Obj.TryGetArray<FString>("Modifiers");
            if (!ArrOpt.HasValue()) { return Out; }
            for (const auto& S : ArrOpt.GetValue())
            {
                const FStringView Sv(S);
                if (Sv == FStringView("Shift")) { Out = Out | EKeyboardModifier::Shift; }
                else if (Sv == FStringView("Control")) { Out = Out | EKeyboardModifier::Control; }
                else if (Sv == FStringView("Alt")) { Out = Out | EKeyboardModifier::Alt; }
                else if (Sv == FStringView("Super")) { Out = Out | EKeyboardModifier::Super; }
                else if (Sv == FStringView("CapsLock")) { Out = Out | EKeyboardModifier::CapsLock; }
                else if (Sv == FStringView("NumLock")) { Out = Out | EKeyboardModifier::NumLock; }
            }
            return Out;
        }
    }

    /**
     * Parse .vinputmap JSON into FInputMapping array.
     * Returns NullOpt on parse error (invalid format, unknown key/button, missing fields).
     * See Engine/Schemas/InputMap.schema.json for format.
     */
    [[nodiscard]] inline TOptional<TArray<FInputMapping>>
    ParseInputMap(const FJSON& I_JSON)
    {
        if (!I_JSON.Contains("Mappings")) { return NullOpt; }
        TArray<FInputMapping> Out;
        for (UInt64 i = 0; ; ++i)
        {
            const FString Route = FString::Format("Mappings[{}]", i);
            auto ItemOpt = I_JSON.TryGetObject(FJSONRoute(Route.GetNative()));
            if (!ItemOpt.HasValue() || ItemOpt.GetValue().IsNull()) { break; }
            const FJSON& Item = ItemOpt.GetValue();

            auto Action = Item.TryGetString("Action");
            if (!Action.HasValue() || Action->IsEmpty()) { return NullOpt; }
            auto Source = Item.TryGetString("Source");
            if (!Source.HasValue()) { return NullOpt; }
            auto Trigger = Item.TryGetString("Trigger");
            if (!Trigger.HasValue()) { return NullOpt; }

            EInputSource SourceType;
            Int32 SourceValue;
            const FStringView SourceSv(Source->GetNative());
            if (SourceSv == FStringView("KeyboardKey"))
            {
                SourceType = EInputSource::KeyboardKey;
                auto Key = Item.TryGetString("Key");
                if (!Key.HasValue() || Key->IsEmpty()) { return NullOpt; }
                auto Val = Detail::ParseKeyName(FStringView(Key->GetNative()));
                if (!Val.HasValue()) { return NullOpt; }
                SourceValue = Val.GetValue();
            }
            else if (SourceSv == FStringView("MouseButton"))
            {
                SourceType = EInputSource::MouseButton;
                auto Btn = Item.TryGetString("Button");
                if (!Btn.HasValue() || Btn->IsEmpty()) { return NullOpt; }
                auto Val = Detail::ParseButtonName(FStringView(Btn->GetNative()));
                if (!Val.HasValue()) { return NullOpt; }
                SourceValue = Val.GetValue();
            }
            else { return NullOpt; }

            EInputTrigger TriggerType;
            const FStringView TriggerSv(Trigger->GetNative());
            if (TriggerSv == FStringView("Press")) { TriggerType = EInputTrigger::Press; }
            else if (TriggerSv == FStringView("Release")) { TriggerType = EInputTrigger::Release; }
            else if (TriggerSv == FStringView("Hold")) { TriggerType = EInputTrigger::Hold; }
            else { return NullOpt; }

            EKeyboardModifier Mods = Detail::ParseModifiers(Item);

            Out.PushBack(FInputMapping{
                .ActionName = FName(Action->GetNative()),
                .SourceType = SourceType,
                .SourceValue = SourceValue,
                .Trigger = TriggerType,
                .Modifiers = Mods,
            });
#if defined(VISERA_DEBUG_MODE)
            const auto& An = Out.Back().ActionName;
            LOG_DEBUG("ParseInputMap: Action='{}' -> Handle={}, GetNameString='{}'",
                      Action->GetNative(), An.GetHandle(), An.GetNameString());
#endif
        }
        return TOptional<TArray<FInputMapping>>(std::move(Out));
    }
}
