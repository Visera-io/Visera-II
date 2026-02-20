# Profiling

This page tracks runtime profiling instrumentation currently integrated in engine modules under `VISERA_PROFILING_MODE`.

---

## How to enable

- CMake option: `VISERA_PROFILING_MODE=ON`
- This defines compile macro `VISERA_PROFILING_MODE`
- `PROFILING_ONLY_FIELD(...)` blocks are compiled only when profiling mode is enabled

---

## Runtime.Audio

File:

- `External/Visera/Engine/Runtime/Audio/Source/Visera-Runtime-Audio.ixx`

Metrics (`FProfilingMetrics`) cover:

- Queue enqueue counts (`Critical`, `Normal`, `Spam`)
- Queue peak pending depth (`Critical`, `Normal`, `Spam`)
- Spam drops (`limit_rejects`, `overflow_drops`)
- Pump batch peak size/capacity
- Coalescing peak counts (`RTPC`, `Position`, `Impact`)
- Inline arena sizing estimate for `PumpBatch`

Log format:

- Startup summary and shutdown summary use `[Profiling]` prefix

---

## Runtime.RHI

### CommandList

File:

- `External/Visera/Engine/Runtime/RHI/Source/CommandList/Visera-Runtime-RHI-CommandList.ixx`

Metrics (`FProfilingMetrics`) cover:

- Peak command count
- Peak command buffer size
- Peak command buffer capacity
- Peak single-command byte size and command type

Logging policy:

- Emits `[Profiling]` logs only on new peaks (to avoid per-frame spam)

### Registry

File:

- `External/Visera/Engine/Runtime/RHI/Source/Registry/Visera-Runtime-RHI-Registry.ixx`

Metrics (`FProfilingMetrics`) cover:

- Resource create/reuse counts (`Texture`, `Buffer`, `Sampler`, `DescriptorSet`)
- Garbage queued/recycled/destroyed counts
- Peak garbage-bin and recycle-bin occupancy
- `CollectGarbage` and `ClearGarbage` call counts

Log format:

- Module summary logs at registry destruction with `[Profiling]` prefix

---

## Runtime.AssetHub

File:

- `External/Visera/Engine/Runtime/AssetHub/Source/Visera-Runtime-AssetHub.ixx`

Metrics (`FProfilingMetrics`) cover:

- Load calls (`Image`, `Shader`, `Font`) and `Load*FromCache` calls
- Cache behavior per asset type:
  - Hot hits
  - Cold promotions
  - Misses
  - Expired cold entry pruning
- Store counts per asset type
- Cache peak stats per asset type:
  - Hot entry count
  - Cold entry count
  - Hot weighted bytes
- Save call/success counts (`Image`, `Shader`)
- Cache clear call counts (`All`, `Image`, `Shader`, `Font`)

Log format:

- `[Profiling]` summary printed in `OnTerminate`

---

## Notes

- Profiling metrics are intended for development tuning and capacity planning.
- These counters are not a replacement for full tracing/profilers; they provide low-cost in-engine observability.
