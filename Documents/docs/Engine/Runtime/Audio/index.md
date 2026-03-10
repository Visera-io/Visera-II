# Runtime.Audio (Visera.Runtime.Audio)

**Runtime.Audio** provides audio subsystem: abstract interface, Null implementation and Wwise integration. Game or editor uses the interface for sound and music, volume and 3D positioning; Null backend for no-audio or headless builds; Wwise backend for full features in release.

## Responsibilities
- **Interface**: Abstract API for play, stop, volume, listener and events; decoupled from backend.
- **Null**: No-op implementation for tests or no-audio builds.
- **Wwise**: Wwise-based implementation; **Wwise-IO** handles file and stream I/O for Wwise and plugs into engine asset system.

## Submodules
| Module | Description |
|------|------|
| [Interface](Interface.md) | Audio interface abstraction. |
| [Null](Null.md) | Null audio backend. |
| [Wwise](Wwise.md) | Wwise backend. |
| [Wwise-IO](Wwise-IO.md) | Wwise I/O adapter. |

## See also
- [Runtime](../index.md) — Parent module
- [AssetHub](../AssetHub/index.md) — Audio asset loading
