module;
#include <Visera-Runtime.hpp>
#include <cctype>
export module Visera.Runtime.Input.Mapping;
#define VISERA_MODULE_NAME "Runtime.Input"
import Visera.Core.Delegate.Multicast;
import Visera.Core.Types.Name;
import Visera.Core.Types.JSON;
import Visera.Core.Types.Optional;
import Visera.Core.Types.String;
import Visera.Core.Containers.Array;
import Visera.Core.Containers.Map;
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
        [[nodiscard]] inline Bool
        AsciiEqualsIgnoreCase(FStringView I_A, FStringView I_B)
        {
            if (I_A.GetSize() != I_B.GetSize())
            { return False; }
            const char* Pa = I_A.Data();
            const char* Pb = I_B.Data();
            for (FString::SizeType i = 0; i < I_A.GetSize(); ++i)
            {
                const unsigned char Ca = static_cast<unsigned char>(Pa[i]);
                const unsigned char Cb = static_cast<unsigned char>(Pb[i]);
                if (std::tolower(Ca) != std::tolower(Cb))
                { return False; }
            }
            return True;
        }

        inline TOptional<Int32> ParseKeyName(FStringView I_Name)
        {
            // FName normalizes ASCII identifiers; lookup is case-insensitive for those names.
            static const TMap<FName, Int32> KeyNameToScanCode{
                {FName("Unknown"), static_cast<Int32>(EPlatformKeyboardKey::Unknown)},
                {FName("Space"), static_cast<Int32>(EPlatformKeyboardKey::Space)},
                {FName("Apostrophe"), static_cast<Int32>(EPlatformKeyboardKey::Apostrophe)},
                {FName("Comma"), static_cast<Int32>(EPlatformKeyboardKey::Comma)},
                {FName("Minus"), static_cast<Int32>(EPlatformKeyboardKey::Minus)},
                {FName("Period"), static_cast<Int32>(EPlatformKeyboardKey::Period)},
                {FName("Slash"), static_cast<Int32>(EPlatformKeyboardKey::Slash)},
                {FName("Num0"), static_cast<Int32>(EPlatformKeyboardKey::Num0)},
                {FName("Num1"), static_cast<Int32>(EPlatformKeyboardKey::Num1)},
                {FName("Num2"), static_cast<Int32>(EPlatformKeyboardKey::Num2)},
                {FName("Num3"), static_cast<Int32>(EPlatformKeyboardKey::Num3)},
                {FName("Num4"), static_cast<Int32>(EPlatformKeyboardKey::Num4)},
                {FName("Num5"), static_cast<Int32>(EPlatformKeyboardKey::Num5)},
                {FName("Num6"), static_cast<Int32>(EPlatformKeyboardKey::Num6)},
                {FName("Num7"), static_cast<Int32>(EPlatformKeyboardKey::Num7)},
                {FName("Num8"), static_cast<Int32>(EPlatformKeyboardKey::Num8)},
                {FName("Num9"), static_cast<Int32>(EPlatformKeyboardKey::Num9)},
                {FName("Semicolon"), static_cast<Int32>(EPlatformKeyboardKey::Semicolon)},
                {FName("Equal"), static_cast<Int32>(EPlatformKeyboardKey::Equal)},
                {FName("A"), static_cast<Int32>(EPlatformKeyboardKey::A)},
                {FName("B"), static_cast<Int32>(EPlatformKeyboardKey::B)},
                {FName("C"), static_cast<Int32>(EPlatformKeyboardKey::C)},
                {FName("D"), static_cast<Int32>(EPlatformKeyboardKey::D)},
                {FName("E"), static_cast<Int32>(EPlatformKeyboardKey::E)},
                {FName("F"), static_cast<Int32>(EPlatformKeyboardKey::F)},
                {FName("G"), static_cast<Int32>(EPlatformKeyboardKey::G)},
                {FName("H"), static_cast<Int32>(EPlatformKeyboardKey::H)},
                {FName("I"), static_cast<Int32>(EPlatformKeyboardKey::I)},
                {FName("J"), static_cast<Int32>(EPlatformKeyboardKey::J)},
                {FName("K"), static_cast<Int32>(EPlatformKeyboardKey::K)},
                {FName("L"), static_cast<Int32>(EPlatformKeyboardKey::L)},
                {FName("M"), static_cast<Int32>(EPlatformKeyboardKey::M)},
                {FName("N"), static_cast<Int32>(EPlatformKeyboardKey::N)},
                {FName("O"), static_cast<Int32>(EPlatformKeyboardKey::O)},
                {FName("P"), static_cast<Int32>(EPlatformKeyboardKey::P)},
                {FName("Q"), static_cast<Int32>(EPlatformKeyboardKey::Q)},
                {FName("R"), static_cast<Int32>(EPlatformKeyboardKey::R)},
                {FName("S"), static_cast<Int32>(EPlatformKeyboardKey::S)},
                {FName("T"), static_cast<Int32>(EPlatformKeyboardKey::T)},
                {FName("U"), static_cast<Int32>(EPlatformKeyboardKey::U)},
                {FName("V"), static_cast<Int32>(EPlatformKeyboardKey::V)},
                {FName("W"), static_cast<Int32>(EPlatformKeyboardKey::W)},
                {FName("X"), static_cast<Int32>(EPlatformKeyboardKey::X)},
                {FName("Y"), static_cast<Int32>(EPlatformKeyboardKey::Y)},
                {FName("Z"), static_cast<Int32>(EPlatformKeyboardKey::Z)},
                {FName("LeftBracket"), static_cast<Int32>(EPlatformKeyboardKey::LeftBracket)},
                {FName("Backslash"), static_cast<Int32>(EPlatformKeyboardKey::Backslash)},
                {FName("RightBracket"), static_cast<Int32>(EPlatformKeyboardKey::RightBracket)},
                {FName("GraveAccent"), static_cast<Int32>(EPlatformKeyboardKey::GraveAccent)},
                {FName("World1"), static_cast<Int32>(EPlatformKeyboardKey::World1)},
                {FName("World2"), static_cast<Int32>(EPlatformKeyboardKey::World2)},
                {FName("Escape"), static_cast<Int32>(EPlatformKeyboardKey::Escape)},
                {FName("Enter"), static_cast<Int32>(EPlatformKeyboardKey::Enter)},
                {FName("Tab"), static_cast<Int32>(EPlatformKeyboardKey::Tab)},
                {FName("Backspace"), static_cast<Int32>(EPlatformKeyboardKey::Backspace)},
                {FName("Insert"), static_cast<Int32>(EPlatformKeyboardKey::Insert)},
                {FName("Delete"), static_cast<Int32>(EPlatformKeyboardKey::Delete)},
                {FName("Right"), static_cast<Int32>(EPlatformKeyboardKey::Right)},
                {FName("Left"), static_cast<Int32>(EPlatformKeyboardKey::Left)},
                {FName("Down"), static_cast<Int32>(EPlatformKeyboardKey::Down)},
                {FName("Up"), static_cast<Int32>(EPlatformKeyboardKey::Up)},
                {FName("PageUp"), static_cast<Int32>(EPlatformKeyboardKey::PageUp)},
                {FName("PageDown"), static_cast<Int32>(EPlatformKeyboardKey::PageDown)},
                {FName("Home"), static_cast<Int32>(EPlatformKeyboardKey::Home)},
                {FName("End"), static_cast<Int32>(EPlatformKeyboardKey::End)},
                {FName("CapsLock"), static_cast<Int32>(EPlatformKeyboardKey::CapsLock)},
                {FName("ScrollLock"), static_cast<Int32>(EPlatformKeyboardKey::ScrollLock)},
                {FName("NumLock"), static_cast<Int32>(EPlatformKeyboardKey::NumLock)},
                {FName("PrintScreen"), static_cast<Int32>(EPlatformKeyboardKey::PrintScreen)},
                {FName("Pause"), static_cast<Int32>(EPlatformKeyboardKey::Pause)},
                {FName("F1"), static_cast<Int32>(EPlatformKeyboardKey::F1)},
                {FName("F2"), static_cast<Int32>(EPlatformKeyboardKey::F2)},
                {FName("F3"), static_cast<Int32>(EPlatformKeyboardKey::F3)},
                {FName("F4"), static_cast<Int32>(EPlatformKeyboardKey::F4)},
                {FName("F5"), static_cast<Int32>(EPlatformKeyboardKey::F5)},
                {FName("F6"), static_cast<Int32>(EPlatformKeyboardKey::F6)},
                {FName("F7"), static_cast<Int32>(EPlatformKeyboardKey::F7)},
                {FName("F8"), static_cast<Int32>(EPlatformKeyboardKey::F8)},
                {FName("F9"), static_cast<Int32>(EPlatformKeyboardKey::F9)},
                {FName("F10"), static_cast<Int32>(EPlatformKeyboardKey::F10)},
                {FName("F11"), static_cast<Int32>(EPlatformKeyboardKey::F11)},
                {FName("F12"), static_cast<Int32>(EPlatformKeyboardKey::F12)},
                {FName("F13"), static_cast<Int32>(EPlatformKeyboardKey::F13)},
                {FName("F14"), static_cast<Int32>(EPlatformKeyboardKey::F14)},
                {FName("F15"), static_cast<Int32>(EPlatformKeyboardKey::F15)},
                {FName("F16"), static_cast<Int32>(EPlatformKeyboardKey::F16)},
                {FName("F17"), static_cast<Int32>(EPlatformKeyboardKey::F17)},
                {FName("F18"), static_cast<Int32>(EPlatformKeyboardKey::F18)},
                {FName("F19"), static_cast<Int32>(EPlatformKeyboardKey::F19)},
                {FName("F20"), static_cast<Int32>(EPlatformKeyboardKey::F20)},
                {FName("F21"), static_cast<Int32>(EPlatformKeyboardKey::F21)},
                {FName("F22"), static_cast<Int32>(EPlatformKeyboardKey::F22)},
                {FName("F23"), static_cast<Int32>(EPlatformKeyboardKey::F23)},
                {FName("F24"), static_cast<Int32>(EPlatformKeyboardKey::F24)},
                {FName("F25"), static_cast<Int32>(EPlatformKeyboardKey::F25)},
                {FName("KP0"), static_cast<Int32>(EPlatformKeyboardKey::KP0)},
                {FName("KP1"), static_cast<Int32>(EPlatformKeyboardKey::KP1)},
                {FName("KP2"), static_cast<Int32>(EPlatformKeyboardKey::KP2)},
                {FName("KP3"), static_cast<Int32>(EPlatformKeyboardKey::KP3)},
                {FName("KP4"), static_cast<Int32>(EPlatformKeyboardKey::KP4)},
                {FName("KP5"), static_cast<Int32>(EPlatformKeyboardKey::KP5)},
                {FName("KP6"), static_cast<Int32>(EPlatformKeyboardKey::KP6)},
                {FName("KP7"), static_cast<Int32>(EPlatformKeyboardKey::KP7)},
                {FName("KP8"), static_cast<Int32>(EPlatformKeyboardKey::KP8)},
                {FName("KP9"), static_cast<Int32>(EPlatformKeyboardKey::KP9)},
                {FName("KPDecimal"), static_cast<Int32>(EPlatformKeyboardKey::KPDecimal)},
                {FName("KPDivide"), static_cast<Int32>(EPlatformKeyboardKey::KPDivide)},
                {FName("KPMultiply"), static_cast<Int32>(EPlatformKeyboardKey::KPMultiply)},
                {FName("KPSubtract"), static_cast<Int32>(EPlatformKeyboardKey::KPSubtract)},
                {FName("KPAdd"), static_cast<Int32>(EPlatformKeyboardKey::KPAdd)},
                {FName("KPEnter"), static_cast<Int32>(EPlatformKeyboardKey::KPEnter)},
                {FName("KPEqual"), static_cast<Int32>(EPlatformKeyboardKey::KPEqual)},
                {FName("LeftShift"), static_cast<Int32>(EPlatformKeyboardKey::LeftShift)},
                {FName("LeftControl"), static_cast<Int32>(EPlatformKeyboardKey::LeftControl)},
                {FName("LeftAlt"), static_cast<Int32>(EPlatformKeyboardKey::LeftAlt)},
                {FName("LeftSuper"), static_cast<Int32>(EPlatformKeyboardKey::LeftSuper)},
                {FName("RightShift"), static_cast<Int32>(EPlatformKeyboardKey::RightShift)},
                {FName("RightControl"), static_cast<Int32>(EPlatformKeyboardKey::RightControl)},
                {FName("RightAlt"), static_cast<Int32>(EPlatformKeyboardKey::RightAlt)},
                {FName("RightSuper"), static_cast<Int32>(EPlatformKeyboardKey::RightSuper)},
                {FName("Menu"), static_cast<Int32>(EPlatformKeyboardKey::Menu)},
            };
            const auto Iterator = KeyNameToScanCode.Find(FName(I_Name));
            if (Iterator == KeyNameToScanCode.end())
            { return NullOpt; }
            return TOptional<Int32>(Iterator->second);
        }

        inline TOptional<Int32> ParseButtonName(FStringView I_Name)
        {
            static const TMap<FName, Int32> ButtonNameToIndex{
                {FName("Left"), static_cast<Int32>(EPlatformMouseButton::Left)},
                {FName("Right"), static_cast<Int32>(EPlatformMouseButton::Right)},
                {FName("Middle"), static_cast<Int32>(EPlatformMouseButton::Middle)},
                {FName("Button4"), static_cast<Int32>(EPlatformMouseButton::Button4)},
                {FName("Button5"), static_cast<Int32>(EPlatformMouseButton::Button5)},
                {FName("Button6"), static_cast<Int32>(EPlatformMouseButton::Button6)},
                {FName("Button7"), static_cast<Int32>(EPlatformMouseButton::Button7)},
                {FName("Button8"), static_cast<Int32>(EPlatformMouseButton::Button8)},
            };
            const auto Iterator = ButtonNameToIndex.Find(FName(I_Name));
            if (Iterator == ButtonNameToIndex.end())
            { return NullOpt; }
            return TOptional<Int32>(Iterator->second);
        }

        [[nodiscard]] inline TOptional<FString>
        TryGetStringPascalOrCamel(const FJSON& I_Obj, FStringView I_PascalKey, FStringView I_CamelKey)
        {
            if (auto V = I_Obj.TryGetString(I_PascalKey); V.HasValue() && !V.GetValue().IsEmpty())
            { return V; }
            return I_Obj.TryGetString(I_CamelKey);
        }

        inline EKeyboardModifier ParseModifiers(const FJSON& I_Obj)
        {
            EKeyboardModifier Out = EKeyboardModifier::None;
            TOptional<TArray<FString>> ArrOpt = I_Obj.TryGetArray<FString>("Modifiers");
            if (!ArrOpt.HasValue())
            { ArrOpt = I_Obj.TryGetArray<FString>("modifiers"); }
            if (!ArrOpt.HasValue()) { return Out; }
            for (const auto& S : ArrOpt.GetValue())
            {
                const FStringView Sv(S);
                if (Detail::AsciiEqualsIgnoreCase(Sv, FStringView("Shift"))) { Out = Out | EKeyboardModifier::Shift; }
                else if (Detail::AsciiEqualsIgnoreCase(Sv, FStringView("Control"))) { Out = Out | EKeyboardModifier::Control; }
                else if (Detail::AsciiEqualsIgnoreCase(Sv, FStringView("Alt"))) { Out = Out | EKeyboardModifier::Alt; }
                else if (Detail::AsciiEqualsIgnoreCase(Sv, FStringView("Super"))) { Out = Out | EKeyboardModifier::Super; }
                else if (Detail::AsciiEqualsIgnoreCase(Sv, FStringView("CapsLock"))) { Out = Out | EKeyboardModifier::CapsLock; }
                else if (Detail::AsciiEqualsIgnoreCase(Sv, FStringView("NumLock"))) { Out = Out | EKeyboardModifier::NumLock; }
            }
            return Out;
        }
    }

    /**
     * Parse one mapping descriptor object (script or JSON). Accepts PascalCase or camelCase keys;
     * source: keyboard | keyboardkey | mouse | mousebutton (case-insensitive); trigger and key/button names are case-insensitive.
     */
    [[nodiscard]] inline TOptional<FInputMapping>
    ParseInputMappingDescriptor(const FJSON& I_Item)
    {
        auto ActionOpt = Detail::TryGetStringPascalOrCamel(I_Item, FStringView("Action"), FStringView("action"));
        if (!ActionOpt.HasValue() || ActionOpt.GetValue().IsEmpty()) { return NullOpt; }
        auto SourceOpt = Detail::TryGetStringPascalOrCamel(I_Item, FStringView("Source"), FStringView("source"));
        if (!SourceOpt.HasValue()) { return NullOpt; }
        auto TriggerOpt = Detail::TryGetStringPascalOrCamel(I_Item, FStringView("Trigger"), FStringView("trigger"));
        if (!TriggerOpt.HasValue()) { return NullOpt; }

        EInputSource SourceType;
        Int32        SourceValue;
        const FStringView SourceSv(SourceOpt.GetValue().GetNative());
        if (Detail::AsciiEqualsIgnoreCase(SourceSv, FStringView("keyboard")) ||
            Detail::AsciiEqualsIgnoreCase(SourceSv, FStringView("keyboardkey")))
        {
            SourceType = EInputSource::KeyboardKey;
            auto KeyOpt = Detail::TryGetStringPascalOrCamel(I_Item, FStringView("Key"), FStringView("key"));
            if (!KeyOpt.HasValue() || KeyOpt.GetValue().IsEmpty()) { return NullOpt; }
            auto Val = Detail::ParseKeyName(FStringView(KeyOpt.GetValue().GetNative()));
            if (!Val.HasValue()) { return NullOpt; }
            SourceValue = Val.GetValue();
        }
        else if (Detail::AsciiEqualsIgnoreCase(SourceSv, FStringView("mouse")) ||
                 Detail::AsciiEqualsIgnoreCase(SourceSv, FStringView("mousebutton")))
        {
            SourceType = EInputSource::MouseButton;
            auto BtnOpt = Detail::TryGetStringPascalOrCamel(I_Item, FStringView("Button"), FStringView("button"));
            if (!BtnOpt.HasValue() || BtnOpt.GetValue().IsEmpty()) { return NullOpt; }
            auto Val = Detail::ParseButtonName(FStringView(BtnOpt.GetValue().GetNative()));
            if (!Val.HasValue()) { return NullOpt; }
            SourceValue = Val.GetValue();
        }
        else { return NullOpt; }

        EInputTrigger TriggerType;
        const FStringView TriggerSv(TriggerOpt.GetValue().GetNative());
        if (Detail::AsciiEqualsIgnoreCase(TriggerSv, FStringView("Press"))) { TriggerType = EInputTrigger::Press; }
        else if (Detail::AsciiEqualsIgnoreCase(TriggerSv, FStringView("Release"))) { TriggerType = EInputTrigger::Release; }
        else if (Detail::AsciiEqualsIgnoreCase(TriggerSv, FStringView("Hold"))) { TriggerType = EInputTrigger::Hold; }
        else { return NullOpt; }

        const EKeyboardModifier Mods = Detail::ParseModifiers(I_Item);

        return FInputMapping{
            .ActionName = FName(ActionOpt.GetValue().GetNative()),
            .SourceType = SourceType,
            .SourceValue = SourceValue,
            .Trigger = TriggerType,
            .Modifiers = Mods,
        };
    }
}
