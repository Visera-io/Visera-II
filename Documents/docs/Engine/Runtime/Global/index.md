# Runtime.Global (Visera.Runtime.Global)

**Runtime.Global** provides runtime global config and service registration. Engine subsystems (RHI, AssetHub, Input, etc.) register as services in a global table; retrieved by name (e.g. EName::RHI) or interface (e.g. IGlobalService) for decoupling and singleton access. Config holds engine-level settings (resolution, audio toggle, etc.) loaded at startup or runtime.

## Responsibilities
- **Configuration**: Global config storage and access; may load from config file or command line.
- **Service**: Service registry; register name and implementation; resolve by name or type for service pointer; RHI, window, etc. often exposed as services to game and editor.

## Submodules
| Module | Description |
|------|------|
| [Configuration](Configuration.md) | Global config type and access. |
| [Service](Service.md) | Service registration and resolution (e.g. RHI service). |

## See also
- [Runtime](../index.md) — Parent module
- [RHI](../RHI/index.md) — Often used as global service
- [Window](../Window/index.md) — Window and swap chain may depend on global service
