# Core.Compression (Visera.Core.Compression)

**Core.Compression** provides zlib-based compression and decompression for asset packaging, network transfer, or save data. The API focuses on whole-buffer compress/decompress; output format includes a 4-byte original-length header for preallocating the decompression buffer.

## Responsibilities
- Compress a memory region (as `FStringView`) into a byte array (`TArray<FByte>`), or decompress compressed data back to original length.
- Uses zlib `compress`/`uncompress`; the first 4 bytes of compressed result are the original length (UInt32) for decompression logic.
- Returns `ECompressionStatue` (Success, NoEnoughMemory, NoEnoughBuffer, InvalidLevel) for success or failure.

## Main types and API

| Type / API | Description |
|------------|------|
| `ECompressionStatue` | Result of compress/decompress: Success, NoEnoughMemory, NoEnoughBuffer, InvalidLevel. |
| `Compress(I_Buffer, O_Buffer)` | Compresses `FStringView I_Buffer` and writes to `TArray<FByte>* O_Buffer` (includes 4-byte length header). |
| `Decompress(...)` | Decompresses header-prefixed data into output buffer (see source for signature). |

## Dependencies

- Uses [OS.Memory](../OS/Memory/index.md) for memory copy, [Containers.Array](../Containers/Array.md) as output container, [Types.String](../Types/String.md) `FStringView` as input view, [Log](../Log/index.md) for errors.

## See also
- [Core](../index.md) — Parent module
- [OS.Memory](../OS/Memory/index.md) — Memory operations
- [Containers.Array](../Containers/Array.md) — Output container
