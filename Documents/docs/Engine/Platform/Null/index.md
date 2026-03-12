# Platform.Null (Visera.Platform.Null)

Null/stub platform implementation. Used for headless runs or tests without a real window.

- **No window**: `CreateWindow` and window-related APIs assert or return safe defaults.
- **No file system**: `GetFileSystem()` returns `nullptr`; callers must not assume a non-null file system on Null.
- **No UUID**: `GenerateUUID()` returns an empty value.
- **Device**: Null.Device provides empty enums for mouse/keyboard so Runtime.Input can compile; no real input.

## See also

- [Platform](../index.md) — parent module
- [Interface](../Interface/index.md) — abstract API
