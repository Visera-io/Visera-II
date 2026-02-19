# Input Map

Visera Input Map defines data-driven mappings from physical input (keyboard keys, mouse buttons) to abstract actions. Actions are created and mappings are loaded from `.vinputmap` JSON; you subscribe to `OnTriggered` in code.

## Asset format

- **File extension**: `.vinputmap`
- **Schema**: `Engine/Schemas/InputMap.schema.json`

Required fields:

| Field | Type | Description |
|-------|------|-------------|
| `Version` | integer (≥1) | Input map format version. |
| `Mappings` | array | List of physical input → action mappings. |

Each mapping object:

| Field | Type | Description |
|-------|------|-------------|
| `Action` | string | Abstract action name (e.g. `Jump`, `Attack`). |
| `Source` | `"KeyboardKey"` \| `"MouseButton"` | Physical input source. |
| `Key` | string | Required when `Source` is `KeyboardKey`. See schema enum for valid keys (e.g. `Space`, `Escape`, `A`). |
| `Button` | string | Required when `Source` is `MouseButton`. Valid: `Left`, `Right`, `Middle`, `Button4`–`Button8`. |
| `Trigger` | `"Press"` \| `"Release"` \| `"Hold"` | When the mapping fires. |
| `Modifiers` | array of strings | Optional. Values: `Shift`, `Control`, `Alt`, `Super`, `CapsLock`, `NumLock`. Empty array = none. |

## How to add a new Action

**Step 1: Edit `.vinputmap`** – Add a mapping entry:

```json
{
  "Action": "Attack",
  "Source": "MouseButton",
  "Button": "Left",
  "Trigger": "Press",
  "Modifiers": []
}
```

**Step 2: Load the input map** – Call `LoadInputMap` once (e.g. in game init):

```cpp
if (!Engine->GetInput()->LoadInputMap(FPath{"Assets/Input/Default.vinputmap"}))
{ LOG_ERROR("Failed to load input map"); }
```

**Step 3: Subscribe in code** – Get the action and bind a callback:

```cpp
auto* AttackAction = Engine->GetInput()->GetAction(FName{"Attack"});
if (AttackAction) AttackAction->OnTriggered.Subscribe([](FInputAction* A) {
    // Handle attack
});
```

Both steps 2 and 3 are required: the JSON defines the mapping; the code defines the response.

## C++ API

- **`FInput::LoadInputMap(FPath)`** – Load mappings from `.vinputmap`. Creates actions as needed. Returns `false` on error.
- **`FInput::GetOrAddAction(FName)`** – Get or create an action.
- **`FInput::GetAction(FName)`** – Get action by name. Returns `nullptr` if not found.
- **`FInputAction::OnTriggered`** – `TMulticastDelegate<FInputAction*>`; subscribe to respond when the action fires.
- **`ParseInputMap(FJSON)`** – Parse JSON to `TArray<FInputMapping>`. Returns `NullOpt` on parse error.

Valid `Key` and `Button` values are listed in `Engine/Schemas/InputMap.schema.json` for editor autocomplete and validation.
