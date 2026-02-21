module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.FileSystem;
#define VISERA_MODULE_NAME "Platform.FileSystem"
export import Visera.Platform.Interface.Path;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Tuple;
       import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    /**
     * Platform-enhanced file system abstraction.
     * Platform-native implementations (no std::filesystem).
     */
    class VISERA_PLATFORM_API IPlatformFileSystem
    {
    public:
        /** @return True if path exists and is a regular file. */
        [[nodiscard]] virtual Bool
        ExistsFile(const IPlatformPath& I_Path) const = 0;

        /** @return True if path exists and is a directory. */
        [[nodiscard]] virtual Bool
        ExistsDirectory(const IPlatformPath& I_Path) const = 0;

        /** Create directory and all parents. Returns 0=Success, 1=NotFound, 2=PermissionDenied, 3=Other. */
        [[nodiscard]] virtual Int32
        CreateDirectories(const IPlatformPath& I_Path) const = 0;

        /** Read entire file. Returns NullOpt on error. */
        [[nodiscard]] virtual TOptional<TArray<FByte>>
        ReadFile(const IPlatformPath& I_Path) const = 0;

        /** Write data to file (overwrites). Returns 0=Success, 1=NotFound, 2=PermissionDenied, 3=Other. */
        [[nodiscard]] virtual Int32
        WriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const = 0;

        /** Delete a single file. Best-effort; logs on failure. Returns 0=Success, 1=NotFound, 2=PermissionDenied, 3=Other. */
        [[nodiscard]] virtual Int32
        DeleteFile(const IPlatformPath& I_Path) const = 0;

        /** Enumerate regular files in directory. I_bRecursive = true for subdirectories. */
        [[nodiscard]] virtual TArray<FPath>
        EnumerateFiles(const IPlatformPath& I_Directory, Bool I_bRecursive) const = 0;

        /** Open file for read/write. Returns nullptr on error. */
        [[nodiscard]] virtual TUniquePtr<FFile>
        OpenFile(const IPlatformPath& I_Path, EFileMode I_Mode) const = 0;

        /** Atomically replace I_Target with I_Source. Source is removed, target gets source content. */
        [[nodiscard]] virtual Int32
        ReplaceFile(const IPlatformPath& I_Source, const IPlatformPath& I_Target) const = 0;

        /** Atomically write: temp + replace. Preserves target on failure. */
        [[nodiscard]] virtual Int32
        AtomicWriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const = 0;

        /**
         * Atomically create unique temp file in I_Directory. I_Prefix is path segment (e.g. "tmp").
         * Caller should Flush() before ReplaceFile. Same directory guarantee for ReplaceFile.
         */
        [[nodiscard]] virtual TPair<TUniquePtr<FFile>, TUniquePtr<IPlatformPath>>
        CreateTempFileNear(const IPlatformPath& I_Directory, const IPlatformPath& I_Prefix) const = 0;

        virtual ~IPlatformFileSystem() = default;
    };
}
