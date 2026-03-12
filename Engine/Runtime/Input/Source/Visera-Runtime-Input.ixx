module;
#include <Visera-Input.hpp>
export module Visera.Runtime.Input;
#define VISERA_MODULE_NAME "Runtime.Input"
export import Visera.Runtime.Input.Action;
export import Visera.Runtime.Input.Device;
export import Visera.Runtime.Input.Mapping;
       import Visera.Runtime.Global;
       import Visera.Platform;
       import Visera.Platform.Interface.Window;
       import Visera.Platform.Interface.Device;
       import Visera.Core.Log;
       import Visera.Core.Types.Path;
       import Visera.Core.Types.JSON;
       import Visera.Core.Types.Pointer;
       import Visera.Core.Containers.Array;
       import Visera.Core.Containers.Map;
       import Visera.Core.Algorithm.Ranges;
       import Visera.Core.Types.Tuple;
       import Visera.Core.OS.Time;
export namespace Visera
{
    /**
     * Global input service: one keyboard (global state), per-window mouse state in Mice, and action/mapping (e.g. .vinputmap).
     * Uses only Interface.Device enums (EPlatformKeyboardKey, EPlatformKeyboardKeyState, etc.); platform-specific
     * handling and casts are confined to the Platform layer so FInput stays platform-agnostic.
     * Call PollAndSync() on the main thread each frame; GetMouse() returns the mouse for the current focused window (or dummy).
     *
     * ActionMapping semantics:
     * - Window callbacks (NotifyKeyboardKey / NotifyMouseButton) only notify: they call TriggerMatchingActions
     *   so that mapped actions receive OnTriggered. They do not modify key/button state (Action, HoldDuration, PressedAt).
     * - Key and button state is updated solely in PollAndSync (via UpdateKeyboardState / UpdateMouseButtonState from
     *   the platform state table). IsActionActive() and key/button OnPressed/OnReleased/OnHeld reflect that polled state.
     */
    class VISERA_RUNTIME_API FInput : public IRuntimeService
    {
    public:
        /** Key for DummyMouse in Mice; nullptr means no window / dummy. */
        static constexpr IPlatformWindow*
        DummyWindow = nullptr;
        /** Global keyboard; never null after construction. */
        [[nodiscard]] inline FKeyboard*
        GetKeyboard() { return Keyboard.Get(); }
        /** Mouse for the current focused window, or Mice[DummyWindow] if no focus or window not in Mice. Never returns nullptr. */
        [[nodiscard]] inline FMouse*
        GetMouse() { return Mice[CurrentFocusedWindow].Get(); }

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
        RemoveMappings(FName I_ActionName);

        /** Load mappings from .vinputmap JSON file. Creates actions as needed. Returns False on parse/file error. */
        [[nodiscard]] Bool
        LoadInputMap(const FPath& I_Path);

        /** True if any mapping for I_ActionName is active this frame (e.g. Hold = key down, Press = just pressed). Call after PollAndSync(). Keyboard only for now. */
        [[nodiscard]] Bool
        IsActionActive(FName I_ActionName) const;

        /** Process input events (PollEvents) and update polling state (Sync). Uses focused window from FPlatform::GetFocusedWindow(). Must be called on the main thread. */
        void
        PollAndSync();

        /** Bind to platform window input callbacks. Call when a window is created (e.g. from FWindow::OnBootstrap). Returns False if any callback failed to bind. */
        [[nodiscard]] Bool
        RegisterWindow(IPlatformWindow* I_PlatformWindow);
        /** Remove window from per-window mouse state and unbind. Call when a platform window is destroyed (e.g. from FWindow::OnTerminate). */
        void
        UnregisterWindow(IPlatformWindow* I_PlatformWindow);

        /** Notify mouse button from window callback. Only triggers action mapping (TriggerMatchingActions); does not modify button state. */
        void
        NotifyMouseButton(IPlatformWindow* I_SourceWindow, FMouse::EButton I_Button, FMouse::EAction I_Action, UInt8 I_Mods);
        /** Notify key from window callback. Only triggers action mapping (TriggerMatchingActions); does not modify key state. */
        void
        NotifyKeyboardKey(IPlatformWindow* I_SourceWindow, FKeyboard::EKey I_Key, Int32 I_ScanCode, FKeyboard::EAction I_Action, UInt8 I_Mods);
        /** Notify cursor move. Routed to the FMouse for I_SourceWindow; fallback to DummyMouse if not in Mice. */
        void
        NotifyCursorMove(IPlatformWindow* I_SourceWindow, Float I_PosX, Float I_PosY);
        /** Notify scroll. Routed to the FMouse for I_SourceWindow; fallback to DummyMouse if not in Mice. */
        void
        NotifyScroll(IPlatformWindow* I_SourceWindow, Float I_OffsetX, Float I_OffsetY);

    private:
        /** Called by NotifyKeyboardKey/NotifyMouseButton only. Fires OnTriggered for matching actions; does not modify key/button state. */
        void TriggerMatchingActions(EInputSource I_SourceType, Int32 I_SourceValue, UInt8 I_Action, UInt8 I_Mods);
        void RebuildReverseIndex();
        /** Sole updater of key state (Action, HoldDuration, PressedAt, OnPressed/OnReleased/OnHeld). Called from PollAndSync only. */
        void UpdateKeyboardState(IPlatformWindow* I_Window);
        /** Sole updater of button state (Action, HoldDuration, PressedAt, OnPressed/OnReleased/OnHeld). Called from PollAndSync only. */
        void UpdateMouseButtonState(IPlatformWindow* I_Window);

    private:
        TUniquePtr<FKeyboard>                      Keyboard;
        /** Per-window mouse state; DummyWindow (nullptr) always holds the dummy mouse. */
        TMap<IPlatformWindow*, TUniquePtr<FMouse>> Mice;
        /** Updated each PollAndSync() from FPlatform::GetFocusedWindow(). */
        IPlatformWindow*                           CurrentFocusedWindow {nullptr};
        /** Caller-owned buffers passed to QueryKeyboardState/QueryMouseButtonState; filled by platform then used to update FKeyboard/FMouse. */
        EPlatformKeyboardKeyState                  KeyboardStateTable[kKeyboardStateTableSize];
        EPlatformMouseButtonState                  MouseButtonStateTable[kMouseButtonStateTableSize];
        TMap<FName, TUniquePtr<FInputAction>>      Actions;
        TArray<FInputMapping>                      Mappings;
        /** Key for reverse index: (SourceType, SourceValue) only. Sorted by this to binary-search mapping set. */
        struct FInputMappingReverseKey
        {
            EInputSource SourceType{};
            Int32        SourceValue{0};
            [[nodiscard]] friend Bool operator<(const FInputMappingReverseKey& A, const FInputMappingReverseKey& B)
            {
                if (A.SourceType != B.SourceType)
                { return static_cast<UInt8>(A.SourceType) < static_cast<UInt8>(B.SourceType); }
                return A.SourceValue < B.SourceValue;
            }
        };
        /** Sorted by (SourceType, SourceValue); each entry is (key, index into Mappings). Used for O(log N + K) lookup in TriggerMatchingActions. Rebuilt when bReverseIndexDirty. */
        TArray<TPair<FInputMappingReverseKey, UInt64>> ReverseIndex;
        /** True when Mappings changed and ReverseIndex must be rebuilt before next lookup. */
        Bool                                           bReverseIndexDirty {True};
        FHiResClock                                    InputClock;

    public:
        FInput(FString I_Name, FServiceRegistry* I_Registry, FJSONView I_ConfigView,
               TMulticastDelegate<const FJSONRoute&>* I_OnConfigChange, FStringView I_RuntimeName)
            : IRuntimeService(I_Name, I_Registry, std::move(I_ConfigView), I_OnConfigChange, I_RuntimeName)
        {
            Dependencies = {};
            Keyboard = MakeUnique<FKeyboard>();
            auto& DummyMouse = Mice[DummyWindow];
            DummyMouse = MakeUnique<FMouse>();
            DummyMouse->GetCursor().Position = FVector2F(-1000.f, -1000.f);

            if (!OnBootstrap.TryBind([] { return True; }))
            { LOG_FATAL("Failed to bind FInput OnBootstrap!"); }
            if (!OnTerminate.TryBind([] { return True; }))
            { LOG_FATAL("Failed to bind FInput OnTerminate!"); }
        }
    };

    Bool
    FInput::RegisterWindow(IPlatformWindow* I_PlatformWindow)
    {
        if (!I_PlatformWindow)
        { LOG_ERROR("({}) RegisterWindow: null platform window.", GetRuntimeName()); return False; }

        // Window callbacks use Interface.Device enums; convert to FKeyboard::EAction / FMouse::EAction here only.
        if (!I_PlatformWindow->KeyboardCallback.TryBind([this, I_PlatformWindow](EPlatformKeyboardKey I_Key, Int32 I_ScanCode, EPlatformKeyboardKeyState I_Action, EPlatformKeyboardModifier I_Mods)
            { NotifyKeyboardKey(I_PlatformWindow, I_Key, I_ScanCode, I_Action == EPlatformKeyboardKeyState::Release ? FKeyboard::EAction::Release : FKeyboard::EAction::Press, static_cast<UInt8>(I_Mods)); }))
        { LOG_ERROR("({}) RegisterWindow: failed to bind KeyboardCallback.", GetRuntimeName()); return False; }
        if (!I_PlatformWindow->MouseButtonCallback.TryBind([this, I_PlatformWindow](EPlatformMouseButton I_Button, EPlatformMouseButtonState I_Action, EPlatformKeyboardModifier I_Mods)
            { NotifyMouseButton(I_PlatformWindow, I_Button, I_Action == EPlatformMouseButtonState::Release ? FMouse::EAction::Release : FMouse::EAction::Press, static_cast<UInt8>(I_Mods)); }))
        { LOG_ERROR("({}) RegisterWindow: failed to bind MouseButtonCallback.", GetRuntimeName()); return False; }
        if (!I_PlatformWindow->CursorMoveCallback.TryBind([this, I_PlatformWindow](Double I_PosX, Double I_PosY)
            { NotifyCursorMove(I_PlatformWindow, static_cast<Float>(I_PosX), static_cast<Float>(I_PosY)); }))
        { LOG_ERROR("({}) RegisterWindow: failed to bind CursorMoveCallback.", GetRuntimeName()); return False; }
        if (!I_PlatformWindow->ScrollCallback.TryBind([this, I_PlatformWindow](Double I_OffsetX, Double I_OffsetY)
            { NotifyScroll(I_PlatformWindow, static_cast<Float>(I_OffsetX), static_cast<Float>(I_OffsetY)); }))
        { LOG_ERROR("({}) RegisterWindow: failed to bind ScrollCallback.", GetRuntimeName()); return False; }

        Mice.InsertOrAssign(I_PlatformWindow, MakeUnique<FMouse>());
        LOG_DEBUG("({}) Registered platform window for input (Mice size {}).", GetRuntimeName(), Mice.GetSize());
        return True;
    }

    void
    FInput::UnregisterWindow(IPlatformWindow* I_PlatformWindow)
    {
        if (!I_PlatformWindow) { return; }  // never erase Mice[DummyWindow]
        if (CurrentFocusedWindow == I_PlatformWindow)
        { CurrentFocusedWindow = nullptr; }
        Mice.Erase(I_PlatformWindow);
        LOG_DEBUG("({}) Unregistered platform window (Mice size {}).", GetRuntimeName(), Mice.GetSize());
    }

    /** Window callback path: only notify action mapping; state is updated solely in PollAndSync (UpdateMouseButtonState). */
    void FInput::NotifyMouseButton(IPlatformWindow* I_SourceWindow, FMouse::EButton I_Button, FMouse::EAction I_Action, UInt8 I_Mods)
    {
        const auto ButtonIndex = static_cast<Int32>(I_Button);
        if (ButtonIndex < 0 || ButtonIndex > FMouse::LastButton) { return; }
        TriggerMatchingActions(EInputSource::MouseButton, ButtonIndex, static_cast<UInt8>(I_Action), I_Mods);
    }

    /** Window callback path: only notify action mapping; state is updated solely in PollAndSync (UpdateKeyboardState). */
    void FInput::NotifyKeyboardKey(IPlatformWindow* I_SourceWindow, FKeyboard::EKey I_Key, Int32 I_ScanCode, FKeyboard::EAction I_Action, UInt8 I_Mods)
    {
        if (!Keyboard) { return; }
        const UInt32 Idx = static_cast<UInt32>(I_Key);
        if (Idx > static_cast<UInt32>(FKeyboard::LastKey)) { return; }
        TriggerMatchingActions(EInputSource::KeyboardKey, static_cast<Int32>(I_Key), static_cast<UInt8>(I_Action), I_Mods);
    }

    void FInput::NotifyCursorMove(IPlatformWindow* I_SourceWindow, Float I_PosX, Float I_PosY)
    {
        FMouse* TargetMouse = Mice[I_SourceWindow].Get();
        if (!TargetMouse) { TargetMouse = Mice[DummyWindow].Get(); }
        FMouse::FCursor& C = TargetMouse->GetCursor();
        C.Position = FVector2F(I_PosX, I_PosY);
        C.OnMoved.Broadcast(C);
    }

    void FInput::NotifyScroll(IPlatformWindow* I_SourceWindow, Float I_OffsetX, Float I_OffsetY)
    {
        FMouse* TargetMouse = Mice[I_SourceWindow].Get();
        if (!TargetMouse) { TargetMouse = Mice[DummyWindow].Get(); }
        FMouse::FScroll& S = TargetMouse->GetScroll();
        S.Offset = FVector2F(I_OffsetX, I_OffsetY);
        S.OnScrolled.Broadcast(S);
    }

    FInputAction*
    FInput::GetOrAddAction(FName I_ActionName)
    {
        auto Iterator = Actions.Find(I_ActionName);
        if (Iterator != Actions.end())
        { return Iterator->second.Get(); }
        auto [NewIterator, Inserted] = Actions.Emplace(I_ActionName, MakeUnique<FInputAction>(I_ActionName));
        (void)Inserted;
        return NewIterator->second.Get();
    }

    FInputAction*
    FInput::GetAction(FName I_ActionName) const
    {
        auto Iterator = Actions.Find(I_ActionName);
        return (Iterator != Actions.end()) ? Iterator->second.Get() : nullptr;
    }

    void
    FInput::AddMapping(const FInputMapping& I_Mapping)
    {
        Mappings.PushBack(I_Mapping);
        bReverseIndexDirty = True;
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
        for (const auto& ParsedMapping : MappingsOpt.GetValue())
        {
            (void)GetOrAddAction(ParsedMapping.ActionName);
            AddMapping(ParsedMapping);
        }
        bReverseIndexDirty = True;
        LOG_DEBUG("({}) Loaded input map: {} ({} mappings).", GetRuntimeName(), I_Path.GetString(), MappingsOpt.GetValue().GetSize());
        return True;
    }

    /** Rebuild ReverseIndex from current Mappings (sorted by SourceType, SourceValue). Clears bReverseIndexDirty. */
    void
    FInput::RebuildReverseIndex()
    {
        ReverseIndex.Clear();
        const UInt64 MappingCount = Mappings.GetSize();
        for (UInt64 Index = 0; Index < MappingCount; ++Index)
        {
            const FInputMapping& MappingEntry = Mappings[static_cast<std::size_t>(Index)];
            ReverseIndex.PushBack(TPair<FInputMappingReverseKey, UInt64>{
                FInputMappingReverseKey{MappingEntry.SourceType, MappingEntry.SourceValue}, Index});
        }
        Algorithm::Sort(ReverseIndex, [](const TPair<FInputMappingReverseKey, UInt64>& Left, const TPair<FInputMappingReverseKey, UInt64>& Right)
            { return Left.first < Right.first; });
        bReverseIndexDirty = False;
        LOG_DEBUG("({}) Rebuilt reverse index ({} entries).", GetRuntimeName(), ReverseIndex.GetSize());
    }

    void
    FInput::RemoveMappings(FName I_ActionName)
    {
        for (auto Iter = Mappings.begin(); Iter != Mappings.end(); )
        {
            if (Iter->ActionName == I_ActionName)
            { Iter = Mappings.Erase(Iter); }
            else
            { ++Iter; }
        }
        bReverseIndexDirty = True;
    }

    Bool
    FInput::IsActionActive(FName I_ActionName) const
    {
        for (const auto& Mapping : Mappings)
        {
            if (Mapping.ActionName != I_ActionName) { continue; }
            if (Mapping.SourceType != EInputSource::KeyboardKey) { continue; }
            const auto Key = static_cast<FKeyboard::EKey>(Mapping.SourceValue);
            const auto KeyAction = Keyboard ? Keyboard->GetKey(Key).Action : static_cast<FKeyboard::EAction>(0);
            const auto Trigger = static_cast<UInt8>(Mapping.Trigger);
            Bool IsActive = False;
            if (Trigger == static_cast<UInt8>(EInputTrigger::Press))
            { IsActive = (KeyAction == FKeyboard::EAction::Press); }
            else if (Trigger == static_cast<UInt8>(EInputTrigger::Release))
            { IsActive = (KeyAction == FKeyboard::EAction::Release); }
            else if (Trigger == static_cast<UInt8>(EInputTrigger::Hold))
            { IsActive = (KeyAction == FKeyboard::EAction::Press || KeyAction == FKeyboard::EAction::Hold); }
            if (IsActive) { return True; }
        }
        return False;
    }

    /** Fire OnTriggered for actions whose mapping matches this raw input. Does not modify key/button state (state is only updated in PollAndSync). */
    void
    FInput::TriggerMatchingActions(EInputSource I_SourceType, Int32 I_SourceValue, UInt8 I_Action, UInt8 I_Mods)
    {
        if (bReverseIndexDirty) { RebuildReverseIndex(); }

        const FInputMappingReverseKey LookupKey{I_SourceType, I_SourceValue};
        auto KeyProjection =
        [](const TPair<FInputMappingReverseKey, UInt64>& Pair)
        -> const FInputMappingReverseKey&
        {
             return Pair.first;
        };

        auto EqualRange = Algorithm::BinarySearch(ReverseIndex, LookupKey, KeyProjection);
        for (const auto& Entry : EqualRange)
        {
            const UInt64 MappingIndex = Entry.second;
            const FInputMapping& Mapping = Mappings[static_cast<std::size_t>(MappingIndex)];
            Bool IsMatch = False;
            if (I_SourceType == EInputSource::KeyboardKey)
            { IsMatch = Mapping.MatchesKey(I_SourceValue, I_Action, I_Mods); }
            else if (I_SourceType == EInputSource::MouseButton)
            { IsMatch = Mapping.MatchesButton(I_SourceValue, I_Action, I_Mods); }
            if (!IsMatch) { continue; }

            if (FInputAction* Action = GetAction(Mapping.ActionName))
            {
                Action->OnTriggered.Broadcast(Action);
            }
        }
    }

    void FInput::UpdateMouseButtonState(IPlatformWindow* I_Window)
    {
        I_Window->QueryMouseButtonState(MouseButtonStateTable);
        const auto Now = FHiResClock::Now();
        auto& ActiveMouse = Mice[I_Window];
        for (Int32 i = FMouse::FirstButton; i <= FMouse::LastButton; ++i)
        {
            auto  ButtonValue = static_cast<FMouse::EButton>(i);
            auto& Button = ActiveMouse->GetButton(ButtonValue);
            if (EPlatformMouseButtonState::Press == MouseButtonStateTable[static_cast<size_t>(i)])
            {
                if (Button.Action == FMouse::EAction::Release)
                {
                    Button.Action       = FMouse::EAction::Press;
                    Button.HoldDuration = 0.0f;
                    Button.PressedAt    = Now;
                    Button.OnPressed.Broadcast(Button);
                }
                else
                {
                    Button.Action       = FMouse::EAction::Hold;
                    Button.HoldDuration = (Now - Button.PressedAt).Seconds();
                    Button.OnHeld.Broadcast(Button);
                }
            }
            else
            {
                if (Button.Action != FMouse::EAction::Release)
                {
                    Button.Action       = FMouse::EAction::Release;
                    Button.HoldDuration = (Now - Button.PressedAt).Seconds();
                    Button.OnReleased.Broadcast(Button);
                    Button.HoldDuration = 0.0f;
                }
            }
        }
    }

    void FInput::UpdateKeyboardState(IPlatformWindow* I_Window)
    {
        I_Window->QueryKeyboardState(KeyboardStateTable);
        const auto Now = FHiResClock::Now();
        for (Int32 i = FKeyboard::FirstKey; i <= FKeyboard::LastKey; ++i)
        {
            auto  KeyValue  = static_cast<FKeyboard::EKey>(i);
            auto& ActiveKey = Keyboard->GetKey(KeyValue);
            if (EPlatformKeyboardKeyState::Press == KeyboardStateTable[static_cast<size_t>(i)])
            {
                if (ActiveKey.Action == FKeyboard::EAction::Release)
                {
                    ActiveKey.Action       = FKeyboard::EAction::Press;
                    ActiveKey.HoldDuration = 0.0f;
                    ActiveKey.PressedAt    = Now;
                    ActiveKey.OnPressed.Broadcast(ActiveKey);
                }
                else
                {
                    ActiveKey.Action       = FKeyboard::EAction::Hold;
                    ActiveKey.HoldDuration = (Now - ActiveKey.PressedAt).Seconds();
                    ActiveKey.OnHeld.Broadcast(ActiveKey);
                }
            }
            else
            {
                if (ActiveKey.Action != FKeyboard::EAction::Release)
                {
                    ActiveKey.Action       = FKeyboard::EAction::Release;
                    ActiveKey.HoldDuration = (Now - ActiveKey.PressedAt).Seconds();
                    ActiveKey.OnReleased.Broadcast(ActiveKey);
                    ActiveKey.HoldDuration = 0.0f;
                }
            }
        }
    }

    void FInput::PollAndSync()
    {
        FPlatform::PollEvents();
        CurrentFocusedWindow = FPlatform::GetFocusedWindow();
        if (!CurrentFocusedWindow) { return; } //[NOTE]: We ignore the dummy window for now.

        UpdateMouseButtonState (CurrentFocusedWindow);
        UpdateKeyboardState    (CurrentFocusedWindow);
    }
}