# Core.Types.UUID (Visera.Core.Types.UUID)

UUID type: universally unique identifier (128-bit). Used for assets, entities, and stable IDs. Conforms to **RFC 4122** (canonical 16-octet layout).

## Layout

- **`FByte Data[16]`** — the UUID as a 16-octet sequence in canonical byte order (same as used by the RFC text form, e.g. `xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx`). Serialization and formatting should use `Data` only.

## API

| Member | Description |
|--------|-------------|
| **`static FUUID Generate()`** | Returns a new RFC 4122 **version 4** (random) UUID. Uses `Core.Math.Random` (thread-local `FRandomSeed` + `FPCG32`); no Platform dependency. |
| **`Bool IsNil() const`** | Returns whether all 16 bytes are zero. `constexpr`-friendly. |
| **`operator<=>` / `operator==`** | Defaulted; lexicographic comparison over `Data[16]`. |

## Generation

- **Core**: `FUUID::Generate()` — suitable when Platform is not available or when you want a single, consistent RNG path. Thread-safe (per-thread RNG).
- **Platform**: `IPlatform::GenerateUUID()` — uses OS APIs (e.g. macOS `uuid_generate_random`, Windows `CoCreateGuid`) when you prefer system-provided randomness.

## See also

- [Types](index.md) — parent module
- [Handle](Handle.md) — handle type
- [Name](Name/index.md) — name type
