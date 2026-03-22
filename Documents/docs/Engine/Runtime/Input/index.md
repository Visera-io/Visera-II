# Runtime.Input and scripting

**FInput** owns keyboard state, per-window mouse state, **actions** (`FInputAction`), and **mappings** (`FInputMapping`). Window callbacks notify mappings (`OnTriggered`); `PollAndSync` updates key/button state for `IsActionActive` and device delegates.

## C++

- **`FInput::AddAction(FName)`** — returns the action, creating it if needed.
- **`FInput::AddMapping(FInputMapping)`** — ensures the action exists, then appends the mapping.
- **`FInput::RemoveMappings(FName)`** — removes all mappings for that action.
- **`FInput::GetAction` / `IsActionActive`** — query; no implicit creation.

C++ gameplay can subscribe with **`FInputAction::OnTriggered.Subscribe(...)`**. Script callbacks use a separate engine bridge (see below) and **coexist** with C++ subscribers.

## Descriptor shape (`ParseInputMappingDescriptor`)

Single JSON object (also used after `JSON.stringify` from JS):

| Field | Notes |
|------|--------|
| `action` / `Action` | Action name (`FName`; pool is case-insensitive for letters). |
| `source` / `Source` | `keyboard` or `keyboardkey` → keyboard; `mouse` or `mousebutton` → mouse. Case-insensitive. |
| `key` / `Key` | Required for keyboard; key name (case-insensitive). |
| `button` / `Button` | Required for mouse; e.g. `Left`, `left`. |
| `trigger` / `Trigger` | `press`, `release`, `hold` (case-insensitive). |
| `modifiers` / `Modifiers` | Optional array: `shift`, `control`, etc. (case-insensitive). |

## JavaScript (`visera.input`)

Available when the engine runs with **Input + Scripting** enabled. `addAction` registers **one** JS function per action; calling `addAction` again **replaces** that function. The engine subscribes once per action to `OnTriggered` and forwards to the current JS callback with a single argument `{ name: string }`.

- **`visera.input.addMapping(descriptor)`** — object fields as in the table above.
- **`visera.input.addAction(name, fn)`**
- **`visera.input.removeMappings(name)`**
- **`visera.input.isActionActive(name)`** — boolean; meaningful after `PollAndSync`.
- **`visera.input.mouse.cursor.position`** / **`.offset`** — getters returning `{ x, y }` from `FMouse::GetCursor()` (snapshot per read).

TypeScript declarations: [Visera.d.ts](../../../../../Engine/APIs/Visera.d.ts).

## See also

- [Runtime](../index.md)
- [InputMap.schema.json](../../../../../Engine/Schemas/InputMap.schema.json) (reference; runtime does not load `.vimap` files in this flow)
