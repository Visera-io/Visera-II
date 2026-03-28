# Runtime.Scripting.VM (`Visera.Runtime.Scripting.VM`)

**`FJavaScriptVM`** wraps a **`v8::Isolate`**: shared V8 platform initialization (reference-counted), array-buffer allocator, and isolate lifetime.

## Free function

- **`ExecuteScript`** — Run source in a given isolate/context; returns **`TOptional<FString>`**: empty on success, or an error message on failure (with logging).

## Source

`Engine/Runtime/Source/Scripting/VM/Visera-Runtime-Scripting-VM.ixx`

## See also

- [Context](Context.md) — uses one VM per scripting stack
- [Scripting overview](index.md)
