module;
#include <Visera-Platform.hpp>
export module Visera.Platform.Interface.FileSystem;
#define VISERA_MODULE_NAME "Platform.FileSystem"
export import Visera.Platform.Interface.Path;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.Optional;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer.Unique;

export namespace Visera
{
    /**
     * Platform-enhanced file system abstraction.
     * Extends Core.OS.FileSystem with platform-specific features (atomic write, temp file, etc.).
     */
    class VISERA_PLATFORM_API IPlatformFileSystem
    {
    public:
        /** @return True if path exists. */
        [[nodiscard]] virtual Bool
        Exists(const IPlatformPath& I_Path) const { return FFileSystem::Exists(I_Path.ToPath()); }

        /** Create directory and all parents. Returns 0=Success, 1=NotFound, 2=PermissionDenied, 3=Other. */
        [[nodiscard]] virtual Int32
        CreateDirectories(const IPlatformPath& I_Path) const { return static_cast<Int32>(static_cast<UInt8>(FFileSystem::CreateDirectory(I_Path.ToPath()))); }

        /** Read entire file. Returns NullOpt on error. */
        [[nodiscard]] virtual TOptional<TArray<FByte>>
        ReadFile(const IPlatformPath& I_Path) const;

        /** Write data to file (overwrites). Returns 0=Success, 1=NotFound, 2=PermissionDenied, 3=Other. */
        [[nodiscard]] virtual Int32
        WriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const;

        /** Delete a single file. Best-effort; logs on failure. Returns 0=Success, 1=NotFound, 2=PermissionDenied, 3=Other. */
        [[nodiscard]] virtual Int32
        DeleteFile(const IPlatformPath& I_Path) const;

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

    inline TOptional<TArray<FByte>> IPlatformFileSystem::
    ReadFile(const IPlatformPath& I_Path) const
    {
        auto F = FFileSystem::OpenFile(I_Path.ToPath(), EFileMode::Read | EFileMode::Binary);
        if (!F || !F->IsOpen()) return NullOpt;
        return F->ReadAll();
    }

    inline Int32 IPlatformFileSystem::
    WriteFile(const IPlatformPath& I_Path, const void* I_Data, UInt64 I_Size) const
    {
        auto F = FFileSystem::OpenFile(I_Path.ToPath(), EFileMode::Write | EFileMode::Binary);
        if (!F || !F->IsOpen()) return 3; // Other
        if (I_Size > 0 && I_Data != nullptr)
        {
            const UInt64 Written = F->Write(I_Data, I_Size, 1);
            if (Written != 1) return 3; // Other
        }
        return 0; // Success
    }

    inline Int32 IPlatformFileSystem::
    DeleteFile(const IPlatformPath& I_Path) const
    {
        return static_cast<Int32>(static_cast<UInt8>(FFileSystem::DeleteFile(I_Path.ToPath())));
    }
}
