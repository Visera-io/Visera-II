# Core.Log (Visera.Core.Log)

**Core.Log** provides the engine's unified logging interface and global Logger instance. Use `FLog::Get()` for the singleton `FLogger`; control level with `ELogLevel` (Trace, Debug, Info, Warn, Error, Fatal); supports multiple sinks and formatting. Macros (e.g. `LOG_ERROR`) wrap Logger calls for file and line output.

## Responsibilities
- Global access `FLog::Get()` returns `FLogger&` for modules to log.
- Defines log level enum `ELogLevel` (and `VISERA_LOG_LEVEL_*` macros) for filtering and config.
- Implementation (sinks, format, thread safety) is in [Logger](Logger.md); this module re-exports and exposes the static interface.

## Main types and API

| Type / API | Description |
|------------|------|
| `FLog` | Static class; Get() returns global FLogger. |
| `ELogLevel` | Log level (Trace / Debug / Info / Warn / Error / Fatal). |
| `FLogger` | Logger type; see [Logger](Logger.md). |

## See also
- [Core](../index.md) — Parent module
- [Logger](Logger.md) — Logger implementation and config
- [Compression](../Compression/index.md) — Logs may write to compressed streams
