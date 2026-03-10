# Core.Types (Visera.Core.Types)

**Core.Types** provides engine-wide base types: string (FString/FStringView), path, JSON, optional, smart pointers (Shared/Unique/Weak), Name, Handle, Tuple, Function, Char, Half, UUID, and re-exports [Containers](../Containers/index.md) (TArray, TMap, etc.), so most code only needs `import Visera.Core.Types` for containers and string.

## Responsibilities
- **String and text**: FString is mutable owned string; FStringView is non-owning view, std::ranges::range, works with [Algorithm.Ranges](../Algorithm/Ranges.md); TStringLiteral<N> is compile-time string literal.
- **Path**: FPath is filesystem path; cross-platform format and conversion by [Platform](../../Platform/index.md) or OS layer.
- **JSON**: Parse and serialize JSON for config and data exchange.
- **Optional and pointers**: TOptional<T> (NullOpt, InPlace, intrusive unset), TUniquePtr/TSharedPtr/TWeakPtr for optional value and lifetime.
- **Name / Handle / UUID**: Name is stable string ID; Handle is opaque resource handle; UUID is 128-bit globally unique ID for assets and entities.
- **Other**: Tuple, Function (callable wrapper), Char encoding, Half float.

## Submodules
| Module | Description |
|------|------|
| [String](String.md) | Owned string FString. |
| [Text](Text.md) | View text FStringView. |
| [Path](Path.md) | Path type FPath. |
| [Optional](Optional.md) | Optional TOptional<T>, NullOpt, InPlace. |
| [Pointer](Pointer/index.md) | [Shared](Pointer/Shared.md), [Unique](Pointer/Unique.md), [Weak](Pointer/Weak.md). |
| [Name](Name/index.md) | Name and NamePool. |
| [Handle](Handle.md) | Handle type. |
| [JSON](JSON.md) | JSON parse and serialize. |
| [Tuple](Tuple.md), [Function](Function.md), [Char](Char.md), [Half](Half.md), [UUID](UUID.md) | Tuple, callable, char, half, UUID. |

## See also
- [Core](../index.md) — Parent module
- [Containers](../Containers/index.md) — Containers (re-exported by Types)
