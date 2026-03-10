# Core.Types.Optional (Visera.Core.Types.Optional)

**Core.Types.Optional** provides optional type TOptional<T> and tags: NullOpt (no value), InPlace (in-place construct), and FIntrusiveUnsetOptionalState (intrusive optional when the type can represent unset, no extra bool). Like std::optional for return, parameter or member "maybe no value".

## Responsibilities
- When HasValue() is true, GetValue() or operator* is safe; otherwise do not dereference.
- Default construct (no value), construct from value, NullOpt, InPlace; copy/move when T supports.
- HasIntrusiveUnsetOptionalState<T>: when true, T can represent unset via comparison with FIntrusiveUnsetOptionalState for zero-overhead optional string etc.

## Main types and constants
| Type / Constant | Description |
|-------------|------|
| `TOptional<T>` | Optional container; HasValue(), GetValue(), operator*, operator->. |
| `NullOpt` | Constant for no value; for construct or assign. |
| `InPlace` | In-place construct tag for TOptional(InPlace, args...). |
| `FIntrusiveUnsetOptionalState` | Intrusive unset marker type. |
| `HasIntrusiveUnsetOptionalState<T>` | Trait: whether T supports intrusive unset. |

## See also
- [Types](index.md) — Parent module
- [Pointer](Pointer/index.md) — Smart pointers (optional-like semantics)
- [String](String.md) — Some string types support intrusive optional
