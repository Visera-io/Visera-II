module;
#include <Visera-Input.hpp>
export module Visera.Runtime.Input;
#define VISERA_MODULE_NAME "Runtime.Inpu"
export import Visera.Runtime.Input.Action;
export import Visera.Runtime.Input.Device;
export import Visera.Runtime.Input.Mapping;
       import Visera.Runtime.Global;
       import Visera.Runtime.Window;
       import Visera.Platform;
       import Visera.Core.Log;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;

export namespace Visera
{
    class VISERA_RUNTIME_API FInput : public IGlobalService
    {
    public:
        [[nodiscard]] inline FKeyboard*
        GetKeyboard() { return &Keyboard; }
        [[nodiscard]] inline FMouse*
        GetMouse()    { return &Mouse; }

        /** Get or create an input action by name (UE5: UInputAction). */
        [[nodiscard]] FInputAction*
        GetOrAddAction(FName I_ActionName);

        /** Get action by name. Returns nullptr if not found. */
        [[nodiscard]] FInputAction*
        GetAction(FName I_ActionName) const;

        /** Add a mapping. Action must exist (call GetOrAddAction first). */
        void
        AddMapping(const FInputMapping& I_Mapping);

        /** Remove all mappings for the given action. */
        void
        RemoveMappingsForAction(FName I_ActionName);

        /** Load mappings from .vinputmap JSON file. Creates actions as needed. Returns false on parse/file error. */
        [[nodiscard]] Bool
        LoadInputMap(const FPath& I_Path);

        /** Process input events (PollEvents) and update polling state (Sync). No-op when Window service is absent (OffScreen). */
        void
        PollAndSync();

    private:
        void TryBindWindowCallbacks();
        void TriggerMatchingActions(EInputSource I_SourceType, Int32 I_SourceValue, UInt8 I_Action, UInt8 I_Mods);

    private:
        FKeyboard                    Keyboard;
        FMouse                       Mouse;
        TSharedPtr<FWindow>          Window;
        Bool                         bCallbacksBound{False};
        TMap<FName, TUniquePtr<FInputAction>> Actions;
        TArray<FInputMapping>        Mappings;

    public:
        FInput(FName I_Name, FServiceRegistry* I_Registry, const FJSON& I_Config)
            : IGlobalService(I_Name, I_Registry, I_Config)
        {
            Dependencies =
            {

            };

            if (!OnBootstrap.TryBind([this]
            {
                     return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    void FInput::TryBindWindowCallbacks()
    {
        if (bCallbacksBound || !Window) { return; }
        auto PlatformWindow = Window->GetPlatformWindow();
        if (!PlatformWindow) { return; }
        if (!PlatformWindow->MouseButtonCallback.TryBind(
        [this](Int32 I_Button, Int32 I_Action, Int32 I_Mods)
        {
            const auto Button = static_cast<FMouse::EButton>(I_Button);
            const auto Mods = static_cast<UInt8>(I_Mods);
            const auto Action = static_cast<UInt8>(I_Action <= 2 ? I_Action : 0);
            TriggerMatchingActions(EInputSource::MouseButton, I_Button, Action, Mods);
            switch (static_cast<FMouse::EAction>(I_Action))
            {
            case FMouse::EAction::Release : return Mouse.OnReleased.Broadcast(Button);
            case FMouse::EAction::Press   : return Mouse.OnPressed.Broadcast(Button);
            case FMouse::EAction::Hold    : return Mouse.OnHeld.Broadcast(Button);
            default: LOG_ERROR("({}) Unhandled button action ({})!", GetRuntimeName(), I_Action);
            }
        }))
        { LOG_FATAL("Failed to bind MouseButtonCallback event!"); return; }

        if (!PlatformWindow->KeyboardCallback.TryBind(
        [this](Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods)
        {
            const auto Key = static_cast<FKeyboard::EKey>(I_Key);
            const auto Mods = static_cast<UInt8>(I_Mods);
            const auto Action = static_cast<UInt8>(I_Action <= 2 ? I_Action : 0);
            TriggerMatchingActions(EInputSource::KeyboardKey, I_Key, Action, Mods);
            switch (static_cast<FKeyboard::EAction>(I_Action))
            {
            case FKeyboard::EAction::Release : return Keyboard.OnReleased.Broadcast(Key);
            case FKeyboard::EAction::Press   : return Keyboard.OnPressed.Broadcast(Key);
            case FKeyboard::EAction::Hold    : return Keyboard.OnHeld.Broadcast(Key);
            default: LOG_ERROR("({}) Unhandled key action ({})!", GetRuntimeName(), I_Action);
            }
        }))
        { LOG_FATAL("Failed to bind KeyboardCallback event!"); return; }

        if (!PlatformWindow->CursorMoveCallback.TryBind(
        [this](Double I_PosX, Double I_PosY)
        {
            Mouse.OnCursorMoved.Broadcast(I_PosX, I_PosY);
        }))
        { LOG_FATAL("Failed to bind CursorMoveCallback event!"); return; }

        if (!PlatformWindow->ScrollCallback.TryBind(
        [this](Double I_OffsetX, Double I_OffsetY)
        {
            Mouse.OnScrolled.Broadcast(static_cast<Float>(I_OffsetX), static_cast<Float>(I_OffsetY));
        }))
        { LOG_FATAL("Failed to bind ScrollCallback event!"); return; }
        bCallbacksBound = True;
    }

    FInputAction*
    FInput::GetOrAddAction(FName I_ActionName)
    {
        auto It = Actions.Find(I_ActionName);
        if (It != Actions.end())
        { return It->second.Get(); }
        auto [NewIt, _] = Actions.Emplace(I_ActionName, MakeUnique<FInputAction>(I_ActionName));
        return NewIt->second.Get();
    }

    FInputAction*
    FInput::GetAction(FName I_ActionName) const
    {
        auto It = Actions.Find(I_ActionName);
        return (It != Actions.end()) ? It->second.Get() : nullptr;
    }

    void
    FInput::AddMapping(const FInputMapping& I_Mapping)
    {
        Mappings.PushBack(I_Mapping);
    }

    Bool
    FInput::LoadInputMap(const FPath& I_Path)
    {
        auto JsonOpt = FJSON::Load(I_Path);
        if (!JsonOpt.HasValue())
        { LOG_ERROR("({}) Failed to load input map: {}", GetRuntimeName(), I_Path.GetString()); return False; }
        auto MappingsOpt = ParseInputMap(JsonOpt.GetValue());
        if (!MappingsOpt.HasValue())
        { LOG_ERROR("({}) Failed to parse input map: {}", GetRuntimeName(), I_Path.GetString()); return False; }
        for (const auto& M : MappingsOpt.GetValue())
        {
            (void)GetOrAddAction(M.ActionName);
            AddMapping(M);
        }
        return True;
    }

    void
    FInput::RemoveMappingsForAction(FName I_ActionName)
    {
        for (auto It = Mappings.begin(); It != Mappings.end(); )
        {
            if (It->ActionName == I_ActionName)
            { It = Mappings.Erase(It); }
            else
            { ++It; }
        }
    }

    void
    FInput::TriggerMatchingActions(EInputSource I_SourceType, Int32 I_SourceValue, UInt8 I_Action, UInt8 I_Mods)
    {
        for (const auto& Mapping : Mappings)
        {
            Bool bMatch = False;
            if (I_SourceType == EInputSource::KeyboardKey)
            { bMatch = Mapping.MatchesKey(I_SourceValue, I_Action, I_Mods); }
            else if (I_SourceType == EInputSource::MouseButton)
            { bMatch = Mapping.MatchesButton(I_SourceValue, I_Action, I_Mods); }
            if (!bMatch) { continue; }

            if (auto* A = GetAction(Mapping.ActionName))
            {
                auto& D = A->OnTriggered;
                D.Broadcast(A);
            }
        }
    }

    void FInput::PollAndSync()
    {
        if (!Window) { Window = GetService<FWindow>(EName::Window).Lock(); }
        if (Window)
        {
            TryBindWindowCallbacks();
            auto PlatformWindow = Window->GetPlatformWindow();
            PlatformWindow->PollEvents();

            Keyboard.Sync([PlatformWindow](FKeyboard::EKey I_Key)->FKeyboard::EAction
            { return static_cast<FKeyboard::EAction>(PlatformWindow->GetKeyboardKey(static_cast<Int32>(I_Key))); });
        }
    }
}