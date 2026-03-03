module;
#include <Visera-Input.hpp>
export module Visera.Runtime.Input.Mapping;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate.Multicast;
import Visera.Core.Types.Name;
import Visera.Core.Types.JSON;
import Visera.Core.Types.Optional;
import Visera.Core.Types.String;
import Visera.Core.Containers.Array;
import Visera.Core.Log;
export import Visera.Platform.Cross.GLFW.Keyboard;
       import Visera.Platform.Cross.GLFW.Mouse;

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

    namespace InputMappingDetail
    {
        inline TOptional<Int32> ParseKeyName(FStringView I_Name)
        {
            static constinit struct { const char* Name; Int32 Value; } const KeyTable[]{
                    {"Unknown", static_cast<Int32>(EGLFWKeyboardKey::Unknown)},
                    {"Space", static_cast<Int32>(EGLFWKeyboardKey::Space)},
                    {"Apostrophe", static_cast<Int32>(EGLFWKeyboardKey::Apostrophe)},
                    {"Comma", static_cast<Int32>(EGLFWKeyboardKey::Comma)},
                    {"Minus", static_cast<Int32>(EGLFWKeyboardKey::Minus)},
                    {"Period", static_cast<Int32>(EGLFWKeyboardKey::Period)},
                    {"Slash", static_cast<Int32>(EGLFWKeyboardKey::Slash)},
                    {"Num0", static_cast<Int32>(EGLFWKeyboardKey::Num0)},
                    {"Num1", static_cast<Int32>(EGLFWKeyboardKey::Num1)},
                    {"Num2", static_cast<Int32>(EGLFWKeyboardKey::Num2)},
                    {"Num3", static_cast<Int32>(EGLFWKeyboardKey::Num3)},
                    {"Num4", static_cast<Int32>(EGLFWKeyboardKey::Num4)},
                    {"Num5", static_cast<Int32>(EGLFWKeyboardKey::Num5)},
                    {"Num6", static_cast<Int32>(EGLFWKeyboardKey::Num6)},
                    {"Num7", static_cast<Int32>(EGLFWKeyboardKey::Num7)},
                    {"Num8", static_cast<Int32>(EGLFWKeyboardKey::Num8)},
                    {"Num9", static_cast<Int32>(EGLFWKeyboardKey::Num9)},
                    {"Semicolon", static_cast<Int32>(EGLFWKeyboardKey::Semicolon)},
                    {"Equal", static_cast<Int32>(EGLFWKeyboardKey::Equal)},
                    {"A", static_cast<Int32>(EGLFWKeyboardKey::A)},
                    {"B", static_cast<Int32>(EGLFWKeyboardKey::B)},
                    {"C", static_cast<Int32>(EGLFWKeyboardKey::C)},
                    {"D", static_cast<Int32>(EGLFWKeyboardKey::D)},
                    {"E", static_cast<Int32>(EGLFWKeyboardKey::E)},
                    {"F", static_cast<Int32>(EGLFWKeyboardKey::F)},
                    {"G", static_cast<Int32>(EGLFWKeyboardKey::G)},
                    {"H", static_cast<Int32>(EGLFWKeyboardKey::H)},
                    {"I", static_cast<Int32>(EGLFWKeyboardKey::I)},
                    {"J", static_cast<Int32>(EGLFWKeyboardKey::J)},
                    {"K", static_cast<Int32>(EGLFWKeyboardKey::K)},
                    {"L", static_cast<Int32>(EGLFWKeyboardKey::L)},
                    {"M", static_cast<Int32>(EGLFWKeyboardKey::M)},
                    {"N", static_cast<Int32>(EGLFWKeyboardKey::N)},
                    {"O", static_cast<Int32>(EGLFWKeyboardKey::O)},
                    {"P", static_cast<Int32>(EGLFWKeyboardKey::P)},
                    {"Q", static_cast<Int32>(EGLFWKeyboardKey::Q)},
                    {"R", static_cast<Int32>(EGLFWKeyboardKey::R)},
                    {"S", static_cast<Int32>(EGLFWKeyboardKey::S)},
                    {"T", static_cast<Int32>(EGLFWKeyboardKey::T)},
                    {"U", static_cast<Int32>(EGLFWKeyboardKey::U)},
                    {"V", static_cast<Int32>(EGLFWKeyboardKey::V)},
                    {"W", static_cast<Int32>(EGLFWKeyboardKey::W)},
                    {"X", static_cast<Int32>(EGLFWKeyboardKey::X)},
                    {"Y", static_cast<Int32>(EGLFWKeyboardKey::Y)},
                    {"Z", static_cast<Int32>(EGLFWKeyboardKey::Z)},
                    {"LeftBracket", static_cast<Int32>(EGLFWKeyboardKey::LeftBracket)},
                    {"Backslash", static_cast<Int32>(EGLFWKeyboardKey::Backslash)},
                    {"RightBracket", static_cast<Int32>(EGLFWKeyboardKey::RightBracket)},
                    {"GraveAccent", static_cast<Int32>(EGLFWKeyboardKey::GraveAccent)},
                    {"World1", static_cast<Int32>(EGLFWKeyboardKey::World1)},
                    {"World2", static_cast<Int32>(EGLFWKeyboardKey::World2)},
                    {"Escape", static_cast<Int32>(EGLFWKeyboardKey::Escape)},
                    {"Enter", static_cast<Int32>(EGLFWKeyboardKey::Enter)},
                    {"Tab", static_cast<Int32>(EGLFWKeyboardKey::Tab)},
                    {"Backspace", static_cast<Int32>(EGLFWKeyboardKey::Backspace)},
                    {"Insert", static_cast<Int32>(EGLFWKeyboardKey::Insert)},
                    {"Delete", static_cast<Int32>(EGLFWKeyboardKey::Delete)},
                    {"Right", static_cast<Int32>(EGLFWKeyboardKey::Right)},
                    {"Left", static_cast<Int32>(EGLFWKeyboardKey::Left)},
                    {"Down", static_cast<Int32>(EGLFWKeyboardKey::Down)},
                    {"Up", static_cast<Int32>(EGLFWKeyboardKey::Up)},
                    {"PageUp", static_cast<Int32>(EGLFWKeyboardKey::PageUp)},
                    {"PageDown", static_cast<Int32>(EGLFWKeyboardKey::PageDown)},
                    {"Home", static_cast<Int32>(EGLFWKeyboardKey::Home)},
                    {"End", static_cast<Int32>(EGLFWKeyboardKey::End)},
                    {"CapsLock", static_cast<Int32>(EGLFWKeyboardKey::CapsLock)},
                    {"ScrollLock", static_cast<Int32>(EGLFWKeyboardKey::ScrollLock)},
                    {"NumLock", static_cast<Int32>(EGLFWKeyboardKey::NumLock)},
                    {"PrintScreen", static_cast<Int32>(EGLFWKeyboardKey::PrintScreen)},
                    {"Pause", static_cast<Int32>(EGLFWKeyboardKey::Pause)},
                    {"F1", static_cast<Int32>(EGLFWKeyboardKey::F1)},
                    {"F2", static_cast<Int32>(EGLFWKeyboardKey::F2)},
                    {"F3", static_cast<Int32>(EGLFWKeyboardKey::F3)},
                    {"F4", static_cast<Int32>(EGLFWKeyboardKey::F4)},
                    {"F5", static_cast<Int32>(EGLFWKeyboardKey::F5)},
                    {"F6", static_cast<Int32>(EGLFWKeyboardKey::F6)},
                    {"F7", static_cast<Int32>(EGLFWKeyboardKey::F7)},
                    {"F8", static_cast<Int32>(EGLFWKeyboardKey::F8)},
                    {"F9", static_cast<Int32>(EGLFWKeyboardKey::F9)},
                    {"F10", static_cast<Int32>(EGLFWKeyboardKey::F10)},
                    {"F11", static_cast<Int32>(EGLFWKeyboardKey::F11)},
                    {"F12", static_cast<Int32>(EGLFWKeyboardKey::F12)},
                    {"F13", static_cast<Int32>(EGLFWKeyboardKey::F13)},
                    {"F14", static_cast<Int32>(EGLFWKeyboardKey::F14)},
                    {"F15", static_cast<Int32>(EGLFWKeyboardKey::F15)},
                    {"F16", static_cast<Int32>(EGLFWKeyboardKey::F16)},
                    {"F17", static_cast<Int32>(EGLFWKeyboardKey::F17)},
                    {"F18", static_cast<Int32>(EGLFWKeyboardKey::F18)},
                    {"F19", static_cast<Int32>(EGLFWKeyboardKey::F19)},
                    {"F20", static_cast<Int32>(EGLFWKeyboardKey::F20)},
                    {"F21", static_cast<Int32>(EGLFWKeyboardKey::F21)},
                    {"F22", static_cast<Int32>(EGLFWKeyboardKey::F22)},
                    {"F23", static_cast<Int32>(EGLFWKeyboardKey::F23)},
                    {"F24", static_cast<Int32>(EGLFWKeyboardKey::F24)},
                    {"F25", static_cast<Int32>(EGLFWKeyboardKey::F25)},
                    {"KP0", static_cast<Int32>(EGLFWKeyboardKey::KP0)},
                    {"KP1", static_cast<Int32>(EGLFWKeyboardKey::KP1)},
                    {"KP2", static_cast<Int32>(EGLFWKeyboardKey::KP2)},
                    {"KP3", static_cast<Int32>(EGLFWKeyboardKey::KP3)},
                    {"KP4", static_cast<Int32>(EGLFWKeyboardKey::KP4)},
                    {"KP5", static_cast<Int32>(EGLFWKeyboardKey::KP5)},
                    {"KP6", static_cast<Int32>(EGLFWKeyboardKey::KP6)},
                    {"KP7", static_cast<Int32>(EGLFWKeyboardKey::KP7)},
                    {"KP8", static_cast<Int32>(EGLFWKeyboardKey::KP8)},
                    {"KP9", static_cast<Int32>(EGLFWKeyboardKey::KP9)},
                    {"KPDecimal", static_cast<Int32>(EGLFWKeyboardKey::KPDecimal)},
                    {"KPDivide", static_cast<Int32>(EGLFWKeyboardKey::KPDivide)},
                    {"KPMultiply", static_cast<Int32>(EGLFWKeyboardKey::KPMultiply)},
                    {"KPSubtract", static_cast<Int32>(EGLFWKeyboardKey::KPSubtract)},
                    {"KPAdd", static_cast<Int32>(EGLFWKeyboardKey::KPAdd)},
                    {"KPEnter", static_cast<Int32>(EGLFWKeyboardKey::KPEnter)},
                    {"KPEqual", static_cast<Int32>(EGLFWKeyboardKey::KPEqual)},
                    {"LeftShift", static_cast<Int32>(EGLFWKeyboardKey::LeftShift)},
                    {"LeftControl", static_cast<Int32>(EGLFWKeyboardKey::LeftControl)},
                    {"LeftAlt", static_cast<Int32>(EGLFWKeyboardKey::LeftAlt)},
                    {"LeftSuper", static_cast<Int32>(EGLFWKeyboardKey::LeftSuper)},
                    {"RightShift", static_cast<Int32>(EGLFWKeyboardKey::RightShift)},
                    {"RightControl", static_cast<Int32>(EGLFWKeyboardKey::RightControl)},
                    {"RightAlt", static_cast<Int32>(EGLFWKeyboardKey::RightAlt)},
                    {"RightSuper", static_cast<Int32>(EGLFWKeyboardKey::RightSuper)},
                    {"Menu", static_cast<Int32>(EGLFWKeyboardKey::Menu)},
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
                    {"Left", static_cast<Int32>(EGLFWMouseButton::Left)},
                    {"Right", static_cast<Int32>(EGLFWMouseButton::Right)},
                    {"Middle", static_cast<Int32>(EGLFWMouseButton::Middle)},
                    {"Button4", static_cast<Int32>(EGLFWMouseButton::Button4)},
                    {"Button5", static_cast<Int32>(EGLFWMouseButton::Button5)},
                    {"Button6", static_cast<Int32>(EGLFWMouseButton::Button6)},
                    {"Button7", static_cast<Int32>(EGLFWMouseButton::Button7)},
                    {"Button8", static_cast<Int32>(EGLFWMouseButton::Button8)},
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
                auto Val = InputMappingDetail::ParseKeyName(FStringView(Key->GetNative()));
                if (!Val.HasValue()) { return NullOpt; }
                SourceValue = Val.GetValue();
            }
            else if (SourceSv == FStringView("MouseButton"))
            {
                SourceType = EInputSource::MouseButton;
                auto Btn = Item.TryGetString("Button");
                if (!Btn.HasValue() || Btn->IsEmpty()) { return NullOpt; }
                auto Val = InputMappingDetail::ParseButtonName(FStringView(Btn->GetNative()));
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

            EKeyboardModifier Mods = InputMappingDetail::ParseModifiers(Item);

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
