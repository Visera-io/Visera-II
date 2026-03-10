# Core.Delegate (Visera.Core.Delegate)

**Core.Delegate** provides delegate types: Unicast and Multicast. For callbacks, events, and decoupling callers from implementers; bind free functions, member functions, or callable objects and invoke at the right time. Differs from [Types.Function](../Types/Function.md): delegates focus on binding a set of receivers and event-style invocation; Function is a single callable wrapper.

## Responsibilities
- **Unicast**: Single target; invocation triggers one callback; for get/set-style callbacks or single listener.
- **Multicast**: Multiple targets; invocation triggers all bound callbacks in order; for event bus, signal-slot, or observer pattern.
- Supports bind/unbind, empty check, and type-safe signatures (template or predefined function type).

## Submodules
| Module | Description |
|------|------|
| [Unicast](Unicast.md) | Unicast delegate: single-target callback. |
| [Multicast](Multicast.md) | Multicast delegate: multiple targets invoked in order. |

## See also
- [Core](../index.md) — Parent module
- [Types.Function](../Types/Function.md) — Callable wrapper
- [Concurrency](../Concurrency/index.md) — Callbacks in async and channels
