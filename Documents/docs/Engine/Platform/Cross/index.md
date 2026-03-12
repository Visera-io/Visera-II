# Platform Cross (moved)

The cross-platform implementations are now under Platform at the same level as Windows and MacOS:

- **[GLFW](../GLFW/index.md)** — window, Device (keyboard, mouse), event loop, file system.
- **[Null](../Null/index.md)** — headless stub (no window, no file system; `GetFileSystem()` returns `nullptr`).

## See also

- [Platform](../index.md) — parent module
- [Interface](../Interface/index.md) — abstract API
