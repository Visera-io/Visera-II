# Core (Visera.Core)

**Core** is the foundation library of the Visera engine. It does not depend on Platform or Runtime and is shared by the engine and toolchain. It provides OS abstraction, math and geometry, base types and containers, logging, concurrency primitives, compression, and metaprogramming.

## Overview

- **Module**: `Visera.Core`
- **Source path**: `Engine/Core/Source/`
- **Dependencies**: Standard library and third-party headers (e.g. Eigen, spdlog, zlib in `Engine/Core/External/`); no other Visera layers.
- **Build**: Built as a static library or module units and linked by Engine and Forge.

Core is organized as C++20 modules (`.ixx`). The root module `Visera.Core` aggregates and re-exports the submodules listed below; types and functions are defined in each submodule. The docs follow the same structure as the source.

## Submodules overview

| Module | Brief |
|--------|-------|
| [Algorithm](Algorithm/index.md) | C++ ranges-based algorithms: find, sort, binary search, partition, and `Less` comparator. |
| [Compression](Compression/index.md) | Compression/decompression (e.g. deflate) for assets and data transfer. |
| [Concurrency](Concurrency/index.md) | Async and lock-free channels (MPSC/SPSC) for multithreading and event passing. |
| [Containers](Containers/index.md) | Containers: `TArray`, `TMap`, `TSet`, `TQueue`, Cache (incl. LRU), List (incl. intrusive), SlotMap. |
| [Delegate](Delegate/index.md) | Unicast and multicast delegates for callbacks and events. |
| [Font](Font/index.md) | Font types and utilities for asset loading and text rendering. |
| [Image](Image/index.md) | Image and pixel types, common formats and layouts. |
| [Log](Log/index.md) | Logging API and Logger (levels, sinks, formatting). |
| [Limits](Limits/index.md) | Numeric bounds and epsilon: `Limits::Epsilon`, `UpperBound`, `LowerBound` (wraps `std::numeric_limits`). |
| [Math](Math/index.md) | Math: algebra (vector/matrix/quaternion), arithmetic, geometry, color, hash, random, trigonometry, bit ops, interpolation, kernels. |
| [Membership](Membership/index.md) | Approximate set-membership: probabilistic structures (e.g. Cuckoo filter) for insert / contain / delete with false positives. |
| [Meta](Meta/index.md) | Metaprogramming and safe/unsafe cast utilities. |
| [OS](OS/index.md) | OS abstraction: file system and file I/O, memory and Arena, thread and sync (atomic, critical section, event, RW lock, spinlock), time (clock, duration, time point). |
| [Traits](Traits/index.md) | Type traits and enum flags utilities. |
| [Types](Types/index.md) | Base types: string (`FString`/`FStringView`), path, JSON, Optional, smart pointers (Shared/Unique/Weak), Name, Handle, Tuple, UUID, Char, Half, Function; re-exports Containers. |

## Usage

Import submodules in C++ as needed, for example:

```cpp
import Visera.Core.Types;        // string, path, containers
import Visera.Core.OS.FileSystem.File;  // file I/O
import Visera.Core.Algorithm.Ranges;    // range algorithms
```

The root module `Visera.Core` re-exports OS, Log, Math, Meta, Types, Font, Image, Traits, Delegate, Algorithm, Compression, Concurrency for a single import.

## See also

- [Engine → Platform](../Platform/index.md) — Platform uses Core types (Path, Text).
- [Engine → Runtime](../Runtime/index.md) — Runtime subsystems depend on Core.
