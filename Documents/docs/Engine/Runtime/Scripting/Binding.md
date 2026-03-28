# Runtime.Scripting.Binding (`Visera.Runtime.Scripting.Binding`)

Aggregates script-facing native APIs: **`RegisterAllBindings`** attaches Graphics, Audio, Input, logging, and optional **`visera.setMainWindow`** to a **`FJavaScriptContext`**.

## `RegisterAllBindings` parameters

- **`FGraphics*`** — May be null if only minimal APIs are needed.
- **`FAudio*`** — Null when audio is disabled; when set, registers bank/event helpers for Wwise.
- **`FInput*`** / **`FJavaScriptInputBindingState*`** — Both null or both set for input bindings.
- **`TOptional<FWindowCreateInfo>*`** — When non-null, registers **`visera.setMainWindow`** so **`OnInit`** can describe the main window for the engine.

## Submodules

| Module | Doc |
|--------|-----|
| `Visera.Runtime.Scripting.Binding.Graphics` | [Binding/Graphics](Binding/Graphics.md) |
| `Visera.Runtime.Scripting.Binding.Input` | [Binding/Input](Binding/Input.md) |
| `Visera.Runtime.Scripting.Binding.Audio` | [Binding/Audio](Binding/Audio.md) |

## Source

`Engine/Runtime/Source/Scripting/Binding/Visera-Runtime-Scripting-Binding.ixx`

## See also

- [Scripting overview](index.md)
