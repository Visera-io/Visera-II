# Core.OS.FileSystem (Visera.Core.OS.FileSystem)

**Core.OS.FileSystem** provides platform-agnostic filesystem abstraction: open/close file, read/write streams, path handling via [Path](../../Types/Path.md). Supports binary and text mode, read-only/write-only/read-write/append (EFileMode), and unified error state (EIOStatus).

## Responsibilities
- FFileSystem static methods: OpenIStream/OpenOStream (return TUniquePtr of std::ifstream/std::ofstream), OpenFile (return TUniquePtr of FFile); all accept FPath and optional mode.
- Defines EIOStatus (Success, NotFound, PermissionDenied, Other), EStreamMode (None, Binary), EFileMode (Binary, Read, Write, Append and combinations) for result and open mode.
- Actual I/O via [File](File.md) Read/Write/ReadAll/Seek/Tell or via returned stream.

## Main types and API

| Type / API | Description |
|------------|------|
| `EIOStatus` | Result: Success, NotFound, PermissionDenied, Other. |
| `EFileMode` | File open mode Flags (Binary, Read, Write, Append, ReadWrite, etc.). |
| `EStreamMode` | Stream mode (None, Binary). |
| `FFileSystem::OpenIStream(path, mode)` | Open read-only stream. |
| `FFileSystem::OpenOStream(path, mode)` | Open write-only stream. |
| `FFileSystem::OpenFile(path, mode)` | Open as FFile handle. |

## See also
- [OS](../index.md) — Parent module
- [File](File.md) — File handle and Read/Write/Seek
- [Types.Path](../../Types/Path.md) — Path type
