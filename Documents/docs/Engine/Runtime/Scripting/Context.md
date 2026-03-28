# Runtime.Scripting.Context (`Visera.Runtime.Scripting.Context`)

**`FJavaScriptContext`** wraps **`v8::Context`**: one context per scripting environment, tied to a **`FJavaScriptVM`**.

## API

| Method | Role |
|--------|------|
| `GetContext()` | Local handle for entering V8 scope during execution. |
| `GetIsolate()` | Owning isolate. |
| `RegisterFunction` | Attach a C++ callback to a name on the global object. |
| `ExecuteScript` | Run UTF-8 source in this context; optional file name for stack traces. |

**`FV8FunctionCallback`** is the callback type for functions exposed from C++ to JavaScript.

## Source

`Engine/Runtime/Source/Scripting/Context/Visera-Runtime-Scripting-Context.ixx`

## See also

- [Binding](Binding.md) — registers many functions on the context
- [Scripting overview](index.md)
