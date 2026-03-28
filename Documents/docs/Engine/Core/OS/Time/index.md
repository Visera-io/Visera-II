# Core.OS.Time (Visera.Core.OS.Time)

**Core.OS.Time** provides time types and utilities: clock, Duration, TimePoint and common time constants or conversion. For frame rate, timers, scheduling and log timestamps; no platform-specific API, consistent on all platforms.

## Responsibilities
- **Clock**: Current or monotonic clock for TimePoint or interval measurement.
- **Duration**: Time span (e.g. ms, s); add, subtract, compare.
- **TimePoint**: A point in time from Clock; add/subtract Duration.
- **Common**: Shared time type definitions, unit conversion or constants.

## Submodules
| Module | Description |
|------|------|
| [Clock](Clock.md) | Clock source. |
| [Duration](Duration.md) | Duration type. |
| [TimePoint](TimePoint.md) | Time point type. |
| [Common](Common.md) | Common time types and helpers. |

## See also
- [OS](../index.md) — Parent module
- [Math.Arithmetic.Interval](../../Math/Arithmetic/Interval.md) — Interval and time window
- [Runtime](../../../Runtime/index.md) — Frame pacing and `FViseraEngine` use [Clock](Clock.md) / OS time types; see [Architecture](../../../Architecture.md)
