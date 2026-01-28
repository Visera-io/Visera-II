module;
#include <Visera-Core.hpp>
#include <cstdio>
export module Visera.OS.FileSystem.File;
#define VISERA_MODULE_NAME "Core.OS"
import Visera.Core.Types.Array;

export namespace Visera
{
    class VISERA_CORE_API FFile
    {
    public:
        [[nodiscard]] Bool
        IsOpen() const { return Handle != nullptr; }

        [[nodiscard]] UInt64
        Read(void* I_Buffer, UInt64 I_Size, UInt64 I_Count = 1);

        [[nodiscard]] TArray<FByte>
        ReadAll();

        [[nodiscard]] Bool
        Seek(Int64 I_Offset, Int32 I_Whence);

        [[nodiscard]] Int64
        Tell() const;

        [[nodiscard]] Bool
        IsEOF() const;

        [[nodiscard]] Int32
        GetError() const;

        void
        ClearError();

        [[nodiscard]] FILE*
        GetHandle() const { return Handle; }

    private:
        FILE* Handle = nullptr;

    public:
        explicit FFile(FILE* I_Handle) : Handle{I_Handle} {}
        ~FFile();
        FFile(const FFile& I_Another)            = delete;
        FFile(FFile&& I_Another)      noexcept;
        FFile& operator=(const FFile& I_Another) = delete;
        FFile& operator=(FFile&& I_Another)      noexcept;
    };

    FFile::
    ~FFile()
    {
        if (Handle != nullptr)
        {
            std::fclose(Handle);
            Handle = nullptr;
        }
    }

    FFile::
    FFile(FFile&& I_Another) noexcept
    : Handle{I_Another.Handle}
    {
        I_Another.Handle = nullptr;
    }

    FFile& FFile::
    operator=(FFile&& I_Another) noexcept
    {
        if (this != &I_Another)
        {
            if (Handle != nullptr)
            { std::fclose(Handle); }
            Handle = I_Another.Handle;
            I_Another.Handle = nullptr;
        }
        return *this;
    }

    UInt64 FFile::
    Read(void* I_Buffer, UInt64 I_Size, UInt64 I_Count)
    {
        if (Handle == nullptr || I_Buffer == nullptr)
        { return 0; }
        return fread(I_Buffer, I_Size, I_Count, Handle);
    }

    TArray<FByte> FFile::
    ReadAll()
    {
        TArray<FByte> Result;
        if (Handle == nullptr)
        { return Result; }

        // Save current position
        const Int64 CurrentPos = Tell();
        if (CurrentPos < 0)
        { return Result; }

        // Seek to end to get file size
        if (!Seek(0, SEEK_END))
        { return Result; }

        const Int64 FileSize = Tell();
        if (FileSize < 0)
        {
            // Restore original position on error
            Seek(CurrentPos, SEEK_SET);
            return Result;
        }

        // Seek back to beginning
        if (!Seek(0, SEEK_SET))
        {
            // Restore original position on error
            Seek(CurrentPos, SEEK_SET);
            return Result;
        }

        // Allocate buffer
        Result.Resize(static_cast<UInt64>(FileSize));

        // Read all data
        if (FileSize > 0)
        {
            const UInt64 BytesRead = Read(Result.Data(), 1, static_cast<UInt64>(FileSize));
            if (BytesRead != static_cast<UInt64>(FileSize))
            {
                // If read failed, resize to actual bytes read
                Result.Resize(BytesRead);
            }
        }

        // Restore original position
        Seek(CurrentPos, SEEK_SET);

        return Result;
    }

    Bool FFile::
    Seek(Int64 I_Offset, Int32 I_Whence)
    {
        if (Handle == nullptr)
        { return False; }
        return std::fseek(Handle, static_cast<long>(I_Offset), I_Whence) == 0;
    }

    Int64 FFile::
    Tell() const
    {
        if (Handle == nullptr)
        { return -1; }
        return std::ftell(Handle);
    }

    Bool FFile::
    IsEOF() const
    {
        if (Handle == nullptr)
        { return True; }
        return std::feof(Handle) != 0;
    }

    Int32 FFile::
    GetError() const
    {
        if (Handle == nullptr)
        { return -1; }
        return std::ferror(Handle);
    }

    void FFile::
    ClearError()
    {
        if (Handle != nullptr)
        { std::clearerr(Handle); }
    }
}