# Runtime.Scripting (`Visera.Runtime.Scripting`)

The **Scripting** service embeds **V8**, loads a JavaScript entry script, registers native bindings, and drives **`OnInit`** (once after load) and **`OnTick(dt)`** each frame from the engine loop.

## Module

- **Export module:** `Visera.Runtime.Scripting`
- **Source:** `Engine/Runtime/Source/Scripting/`

## Main types

| Type | Role |
|------|------|
| `FScriptingCreateInfo` | `EntryScriptPath` (default `@assets://scripts/main.js`). |
| `FScripting` | Owns `FJavaScriptVM`, `FJavaScriptContext`, optional input binding state; registers APIs and runs scripts. |

## Construction and dependencies

`FScripting` is constructed only when **`FEngineCreateInfo.Scripting`**, **`Graphics`**, and **`AssetHub`** are all enabled. **`FGraphics`**, **`FAssetHub`**, optional **`FAudio`**, and optional **`FInput`** must **outlive** `FScripting`.

If the engine did not set **`MainWindow`** but scripting should create it from JS, pass **`RegisterScriptMainWindow = true`**. Then **`ResolveMainWindow()`** returns the `FWindowCreateInfo` written by **`visera.setMainWindow`** during **`OnInit`**, if any.

## Lifecycle

1. VM and context are created; **`RegisterAllBindings`** wires Graphics, Audio, Input, and optional main-window sink.
2. Entry script is read via **`FPlatform::ReadFile`** and executed in the context.
3. If load succeeds, global **`OnInit()`** is invoked when defined.
4. Each frame, **`Tick(dt)`** calls global **`OnTick(dt)`** when it exists and is a function.

## Submodules

| Doc | Module |
|-----|--------|
| [VM](VM.md) | `Visera.Runtime.Scripting.VM` |
| [Context](Context.md) | `Visera.Runtime.Scripting.Context` |
| [Binding](Binding.md) | `Visera.Runtime.Scripting.Binding` |
| [Utils](Utils.md) | `Visera.Runtime.Scripting.Utils` |

## See also

- [Architecture](../../Architecture.md) — engine services and `Run()` order
- [Runtime](../index.md) — parent module
