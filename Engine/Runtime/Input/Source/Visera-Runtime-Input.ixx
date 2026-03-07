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
       import Visera.Platform.Interface.Window;
       import Visera.Core.Log;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;

export namespace Visera
{
    class VISERA_RUNTIME_API FInput : public IRuntimeService
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

        /** True if any mapping for I_ActionName is active this frame (e.g. Hold = key down, Press = just pressed). Call after PollAndSync(). Keyboard only for now. */
        [[nodiscard]] Bool
        IsActionActive(FName I_ActionName) const;

        /** Process input events (PollEvents) and update polling state (Sync). Uses platform window set via SetWindowForSync (e.g. by FWindow). */
        void
        PollAndSync();

        /** Set the platform window used for keyboard Sync in PollAndSync. Call with nullptr when window is gone. */
        void
        SetWindowForSync(IPlatformWindow* I_PlatformWindow) { PlatformWindowForSync = I_PlatformWindow; }

        /** Notify mouse button (called by FWindow from PlatformWindow callback). */
        void
        NotifyMouseButton(FMouse::EButton I_Button, FMouse::EAction I_Action, UInt8 I_Mods);
        /** Notify key (called by FWindow from PlatformWindow callback). */
        void
        NotifyKeyboardKey(FKeyboard::EKey I_Key, Int32 I_ScanCode, FKeyboard::EAction I_Action, UInt8 I_Mods);
        /** Notify cursor move (called by FWindow from PlatformWindow callback). */
        void
        NotifyCursorMove(Float I_PosX, Float I_PosY);
        /** Notify scroll (called by FWindow from PlatformWindow callback). */
        void
        NotifyScroll(Float I_OffsetX, Float I_OffsetY);

    private:
        void TriggerMatchingActions(EInputSource I_SourceType, Int32 I_SourceValue, UInt8 I_Action, UInt8 I_Mods);

    private:
        FKeyboard                    Keyboard;
        FMouse                       Mouse;
        IPlatformWindow*             PlatformWindowForSync {nullptr};
        TMap<FName, TUniquePtr<FInputAction>> Actions;
        TArray<FInputMapping>        Mappings;
        TMulticastDelegate<Int32, Int32, Int32, Int32>::FHandle SubHandleKeyboard    {0};
        TMulticastDelegate<Int32, Int32, Int32>::FHandle       SubHandleMouseButton {0};
        TMulticastDelegate<Double, Double>::FHandle            SubHandleCursorMove  {0};
        TMulticastDelegate<Double, Double>::FHandle            SubHandleScroll       {0};

    public:
        FInput(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
               TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
            : IRuntimeService(I_Name, I_Registry, std::move(I_ConfigView), I_OnConfigChange, I_RuntimeName)
        {
            if (I_Registry && I_Registry->Contains(EService::Window))
            { Dependencies = { EService::Window }; }
            else
            { Dependencies = {}; }

            if (!OnBootstrap.TryBind([this]
            {
                auto WinPtr = GetService<FWindow>(EService::Window).Lock();
                if (WinPtr && WinPtr->GetPlatformWindow())
                {
                    SetWindowForSync(WinPtr->GetPlatformWindow().Get());
                    SubHandleKeyboard    = WinPtr->OnKeyboardKey   .Subscribe([this](Int32 I_Key, Int32 I_ScanCode, Int32 I_Action, Int32 I_Mods)
                    { NotifyKeyboardKey(static_cast<FKeyboard::EKey>(I_Key), I_ScanCode, static_cast<FKeyboard::EAction>(I_Action <= 2 ? I_Action : 0), static_cast<UInt8>(I_Mods)); });
                    SubHandleMouseButton = WinPtr->OnMouseButton  .Subscribe([this](Int32 I_Button, Int32 I_Action, Int32 I_Mods)
                    { NotifyMouseButton(static_cast<FMouse::EButton>(I_Button), static_cast<FMouse::EAction>(I_Action <= 2 ? I_Action : 0), static_cast<UInt8>(I_Mods)); });
                    SubHandleCursorMove  = WinPtr->OnCursorMove   .Subscribe([this](Double I_PosX, Double I_PosY)
                    { NotifyCursorMove(static_cast<Float>(I_PosX), static_cast<Float>(I_PosY)); });
                    SubHandleScroll      = WinPtr->OnScroll       .Subscribe([this](Double I_OffsetX, Double I_OffsetY)
                    { NotifyScroll(static_cast<Float>(I_OffsetX), static_cast<Float>(I_OffsetY)); });
                }
                return True;
            }))
            { LOG_FATAL("Failed to bind bootstrap function!"); }

            if (!OnTerminate.TryBind([this]
            {
                SetWindowForSync(nullptr);
                if (auto WinPtr = GetService<FWindow>(EService::Window).Lock())
                {
                    if (SubHandleKeyboard)    { WinPtr->OnKeyboardKey   .Unsubscribe(SubHandleKeyboard);    SubHandleKeyboard    = 0; }
                    if (SubHandleMouseButton) { WinPtr->OnMouseButton   .Unsubscribe(SubHandleMouseButton); SubHandleMouseButton = 0; }
                    if (SubHandleCursorMove)  { WinPtr->OnCursorMove    .Unsubscribe(SubHandleCursorMove);  SubHandleCursorMove  = 0; }
                    if (SubHandleScroll)      { WinPtr->OnScroll        .Unsubscribe(SubHandleScroll);      SubHandleScroll      = 0; }
                }
                return True;
            }))
            { LOG_FATAL("Failed to bind terminate function!"); }
        }
    };

    void FInput::NotifyMouseButton(FMouse::EButton I_Button, FMouse::EAction I_Action, UInt8 I_Mods)
    {
        TriggerMatchingActions(EInputSource::MouseButton, static_cast<Int32>(I_Button), static_cast<UInt8>(I_Action), I_Mods);
        switch (I_Action)
        {
        case FMouse::EAction::Release : Mouse.OnReleased.Broadcast(I_Button); break;
        case FMouse::EAction::Press   : Mouse.OnPressed.Broadcast(I_Button); break;
        case FMouse::EAction::Hold    : Mouse.OnHeld.Broadcast(I_Button); break;
        default: LOG_ERROR("({}) Unhandled button action ({})!", GetRuntimeName(), static_cast<Int32>(I_Action));
        }
    }

    void FInput::NotifyKeyboardKey(FKeyboard::EKey I_Key, Int32 I_ScanCode, FKeyboard::EAction I_Action, UInt8 I_Mods)
    {
        TriggerMatchingActions(EInputSource::KeyboardKey, static_cast<Int32>(I_Key), static_cast<UInt8>(I_Action), I_Mods);
        switch (I_Action)
        {
        case FKeyboard::EAction::Release : Keyboard.OnReleased.Broadcast(I_Key); break;
        case FKeyboard::EAction::Press   : Keyboard.OnPressed.Broadcast(I_Key); break;
        case FKeyboard::EAction::Hold    : Keyboard.OnHeld.Broadcast(I_Key); break;
        default: LOG_ERROR("({}) Unhandled key action ({})!", GetRuntimeName(), static_cast<UInt8>(I_Action));
        }
    }

    void FInput::NotifyCursorMove(Float I_PosX, Float I_PosY)
    {
        Mouse.OnCursorMoved.Broadcast(I_PosX, I_PosY);
    }

    void FInput::NotifyScroll(Float I_OffsetX, Float I_OffsetY)
    {
        Mouse.OnScrolled.Broadcast(I_OffsetX, I_OffsetY);
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

    Bool
    FInput::IsActionActive(FName I_ActionName) const
    {
        for (const auto& M : Mappings)
        {
            if (M.ActionName != I_ActionName) { continue; }
            if (M.SourceType != EInputSource::KeyboardKey) { continue; }
            const auto Key = static_cast<FKeyboard::EKey>(M.SourceValue);
            const auto KeyAct = Keyboard.GetKeyAction(Key);
            const auto Trig = static_cast<UInt8>(M.Trigger);
            Bool bActive = False;
            if (Trig == static_cast<UInt8>(EInputTrigger::Press))
            { bActive = (KeyAct == FKeyboard::EAction::Press); }
            else if (Trig == static_cast<UInt8>(EInputTrigger::Release))
            { bActive = (KeyAct == FKeyboard::EAction::Release); }
            else if (Trig == static_cast<UInt8>(EInputTrigger::Hold))
            { bActive = (KeyAct == FKeyboard::EAction::Press || KeyAct == FKeyboard::EAction::Hold); }
            if (bActive) { return True; }
        }
        return False;
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
        if (!PlatformWindowForSync) { FPlatform::PollEvents(); }
        
        if (PlatformWindowForSync)
        {
            Keyboard.Sync([this](FKeyboard::EKey I_Key)->FKeyboard::EAction
            { return static_cast<FKeyboard::EAction>(PlatformWindowForSync->GetKeyboardKey(static_cast<Int32>(I_Key))); });
        }
    }
}