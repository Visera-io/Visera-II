# Runtime.Input.Device.Keyboard (Visera.Runtime.Input.Device.Keyboard)

Keyboard input device. Key states and events. Keys are stored in a dense array indexed by `EPlatformKeyboardKey` from `FirstKey` (Space) through `LastKey` (Menu); `GetKey(I_Key)` maps to the slot at `Key - FirstKey`, avoiding unused entries for lower key codes.

## See also

- [Device](index.md) — parent module
- [Mouse](Mouse.md) — mouse device
- [Action](../Action.md) — input actions
