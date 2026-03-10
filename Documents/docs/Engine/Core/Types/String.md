# Core.Types.String (Visera.Core.Types.String)

**Core.Types.String** provides owned string type FString and utilities. FString is based on std::string with std-compatible interface (Data, GetSize, Append, Resize, etc.) and interop with FStringView and std::string_view; supports intrusive unset state for use with [Optional](Optional.md) without extra allocation.

## Responsibilities
- Mutable owned string; copy and move, iterators, range for, and [Algorithm.Ranges](../Algorithm/Ranges.md).
- Construct and assign from const char*, std::string, FStringView; implicit or explicit convert to FStringView for read-only view.
- TStringLiteral<N> for compile-time string literal in templates and constexpr.

## Main types
| Type | Description |
|------|------|
| `FString` | Owned string; interface similar to std::string. |
| `FStringView` | Non-owning view, std::ranges::range; in this module or [Text](Text.md). |
| `TStringLiteral<N>` | Compile-time string literal; Size() includes \0. |

## See also
- [Types](index.md) — Parent module
- [Text](Text.md) — FStringView and read-only view
- [Algorithm.Ranges](../Algorithm/Ranges.md) — Find, split, etc. on string view
