module;
#include <Visera-Core.hpp>
#include <fstream>
export module Visera.Core.OS.FileSystem;
#define VISERA_MODULE_NAME "Core.OS"
export import Visera.OS.FileSystem.File;
export import Visera.Core.Types.Path;
export import Visera.Core.Traits.Flags;
       import Visera.Core.Containers.Array;
       import Visera.Core.Types.String;
       import Visera.Core.Types.Pointer.Unique;
       import Visera.Core.Log;

export namespace Visera
{
    enum class EIOStatus : UInt8
    {
        Success          = 0,
        NotFound         = 1,
        PermissionDenied = 2,
        Other            = 3,
    };

    enum class EStreamMode : Int32
    {
        None       = 0,
        Binary     = std::ios_base::binary,
    };
    VISERA_MAKE_FLAGS(EStreamMode);

    enum class EFileMode : Int32
    {
        Binary     = 1 << 0,
        Read       = 1 << 1,
        Write      = 1 << 2,
        Append     = 1 << 3,
        ReadWrite  = 1 << 4,
        WriteRead  = 1 << 5,
        AppendRead = 1 << 6,
    };
    VISERA_MAKE_FLAGS(EFileMode);

    class VISERA_CORE_API FFileSystem
    {
    public:
        [[nodiscard]] TUniquePtr<std::ifstream> static inline
        OpenIStream(const FPath& I_Path, EStreamMode I_Mode = EStreamMode::None);
        [[nodiscard]] TUniquePtr<std::ofstream> static inline
        OpenOStream(const FPath& I_Path, EStreamMode I_Mode = EStreamMode::None);
        [[nodiscard]] TUniquePtr<FFile> static inline
        OpenFile(const FPath& I_Path, EFileMode I_Mode);

    public:
        explicit FFileSystem() = default;
        explicit FFileSystem(const FFileSystem& I_Another)       = default;
        explicit FFileSystem(FFileSystem&& I_Another)   noexcept = default;
        FFileSystem& operator=(const FFileSystem& I_Another)     = default;
        FFileSystem& operator=(FFileSystem&& I_Another) noexcept = default;

    private:
        [[nodiscard]] static inline const char*
        GetFileModeString(EFileMode I_Mode);
    };

    TUniquePtr<std::ifstream> FFileSystem::
    OpenIStream(const FPath& I_Path, EStreamMode I_Mode)
    {
        const auto& PathStr = I_Path.GetString().GetNative();
        auto IStream = MakeUnique<std::ifstream>(PathStr, ToUnderlying(I_Mode));
        if (!IStream->is_open())
        {
            LOG_DEBUG("Failed to open input stream: {}", PathStr);
            return nullptr;
        }
        return std::move(IStream);
    }

    TUniquePtr<std::ofstream> FFileSystem::
    OpenOStream(const FPath& I_Path, EStreamMode I_Mode)
    {
        const auto& PathStr = I_Path.GetString().GetNative();
        auto OStream = MakeUnique<std::ofstream>(PathStr, ToUnderlying(I_Mode));
        if (!OStream->is_open())
        {
            LOG_DEBUG("Failed to open output stream: {}", PathStr);
            return nullptr;
        }
        return std::move(OStream);
    }

    inline const char* FFileSystem::
    GetFileModeString(EFileMode I_Mode)
    {
        const Bool bBinary = (I_Mode & EFileMode::Binary);
        const EFileMode AccessMode = static_cast<EFileMode>(ToUnderlying(I_Mode) & ~ToUnderlying(EFileMode::Binary));

        if (AccessMode & EFileMode::Read)
        { return bBinary ? "rb" : "r"; }
        else if (AccessMode & EFileMode::Write)
        { return bBinary ? "wb" : "w"; }
        else if (AccessMode & EFileMode::Append)
        { return bBinary ? "ab" : "a"; }
        else if (AccessMode & EFileMode::ReadWrite)
        { return bBinary ? "r+b" : "r+"; }
        else if (AccessMode & EFileMode::WriteRead)
        { return bBinary ? "w+b" : "w+"; }
        else if (AccessMode & EFileMode::AppendRead)
        { return bBinary ? "a+b" : "a+"; }
        else
        { return bBinary ? "rb" : "r"; } // Default to read mode
    }

    TUniquePtr<FFile> FFileSystem::
    OpenFile(const FPath& I_Path, EFileMode I_Mode)
    {
        const char* ModeStr = GetFileModeString(I_Mode);
        const auto& PathStr = I_Path.GetString().GetNative();
        FILE* Handle = std::fopen(PathStr.c_str(), ModeStr);
        if (Handle == nullptr)
        {
            LOG_DEBUG("Failed to open file: {}", PathStr);
            return nullptr;
        }
        return MakeUnique<FFile>(Handle);
    }
}

VISERA_MAKE_FORMATTER(Visera::EIOStatus,
    const char* Name = "Unknown";
    switch (I_Formatee)
    {
        case Visera::EIOStatus::Success:          Name = "Success";          break;
        case Visera::EIOStatus::NotFound:         Name = "NotFound";         break;
        case Visera::EIOStatus::PermissionDenied: Name = "PermissionDenied"; break;
        case Visera::EIOStatus::Other:            Name = "Other";            break;
        default: break;
    },
    "{}", Name
);