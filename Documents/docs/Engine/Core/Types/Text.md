# Core.Types.Text (Visera.Core.Types.Text)

**Core.Types.Text** provides non-owning text view (e.g. FStringView): wraps std::string_view or equivalent, satisfies std::ranges::range, works with [Algorithm.Ranges](../Algorithm/Ranges.md). For passing read-only string without copy in parameters or return; construct from FString, const char*, std::string. Pairs with [String](String.md) owned FString.

## See also
- [Types](index.md) — Parent module
- [String](String.md) — Owned string
- [Char](Char.md) — Character and encoding
