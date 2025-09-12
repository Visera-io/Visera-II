module;
#include <Visera-Core.hpp>
#include <zlib.h>
export module Visera.Core.Compression;
#define VISERA_MODULE_NAME "Core.Compression"
import Visera.Core.OS.Memory;

export namespace Visera
{
    enum class ECompressionStatue
    {
        Success         = Z_OK,
        NoEnoughMemory  = Z_MEM_ERROR,
        NoEnoughBuffer  = Z_BUF_ERROR,
        InvalidLevel    = Z_STREAM_ERROR,
    };

    [[nodiscard]] VISERA_CORE_API ECompressionStatue
    Compress(FStringView I_Buffer, TMutable<TArray<FByte>> O_Buffer)
    {
        const FByte* SourceData = reinterpret_cast<const FByte*>(I_Buffer.data());
        Int32 SourceSize = O_Buffer->size();

        if (SourceSize == 0) { return ECompressionStatue::Success; }

        // Memory layout of CompressedData:
        // [ 4 bytes: original size ] [ compressed data... ]
        //   ^                         ^
        //   |                         |
        // sizeof(Int32) header        Actual zlib compressed data
        uLongf CompressedSize = compressBound(SourceSize);
        O_Buffer->resize(CompressedSize + sizeof(Int32));

        // Store original size at the beginning for decompression
        Int32 OriginalSize = SourceSize;
        Memory::Memcpy(O_Buffer->data(), &OriginalSize, sizeof(Int32));

        // Compress the data
        Int32 ZlibResult = compress(
            O_Buffer->data() + sizeof(Int32),  // Destination (after size header)
            &CompressedSize,                           // Compressed size (in/out)
            SourceData,                                // Source data
            SourceSize                                 // Source size
        );

        if (ZlibResult == Z_OK)
        {
            // Resize array to actual compressed size + header
            O_Buffer->resize(CompressedSize + sizeof(Int32));
        }

        return static_cast<ECompressionStatue>(ZlibResult);
    }
}