# Runtime.Scripting.Utils (`Visera.Runtime.Scripting.Utils`)

Shared **V8 value conversion** helpers used by VM, Context, and Binding:

- **`ToV8String`** — `FStringView` → `v8::String`
- **`FromV8String`** — `v8::Value` → `FString` (UTF-8)
- **`FromV8Int32`** / **`FromV8Double`** — Numeric extraction with validation

## Source

`Engine/Runtime/Source/Scripting/Utils/Visera-Runtime-Scripting-Utils.ixx`

## See also

- [Scripting overview](index.md)
