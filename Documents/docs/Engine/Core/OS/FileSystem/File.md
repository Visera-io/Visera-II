# Core.OS.FileSystem.File (Visera.Core.OS.FileSystem.File)

**Core.OS.FileSystem.File** provides file handle FFile wrapping FILE* for open, read, write, seek and flush. Block Read/Write, ReadAll, Seek/Tell/IsEOF/error/Flush for binary or text block I/O. Move-only; handle closed on destruct.

## Responsibilities
- Wraps FILE*; takes or releases handle on ctor/move; closes on destruct.
- Read(I_Buffer, I_Size, I_Count), Write(I_Data, I_Size, I_Count) (fread/fwrite semantics); ReadAll() reads entire file into TArray<FByte>.
- Seek(I_Offset, I_Whence), Tell(), IsEOF(), GetError()/ClearError(), Flush() match C library behavior.
- Non-copyable, move-only; GetHandle() for underlying FILE* (C or third-party interop).

## Main API

| API | Description |
|-----|------|
| `IsOpen()` | Whether a valid handle is held. |
| `Read(buffer, size, count)` | Read blocks; returns number of blocks read. |
| `Write(data, size, count)` | Write blocks; returns number of blocks written. |
| `ReadAll()` | Read entire file into TArray<FByte>. |
| `Seek(offset, whence)` | Move file position (SEEK_SET/SEEK_CUR/SEEK_END). |
| `Tell()` | Current file position. |
| `IsEOF()` / `GetError()` / `ClearError()` / `Flush()` | Status and flush. |

## See also
- [FileSystem](index.md) — Parent module; get FFile via FFileSystem::OpenFile
- [OS](../index.md) — Parent module
- [Types.Path](../../Types/Path.md) — Path type
