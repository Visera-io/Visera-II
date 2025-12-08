module;
#include <Visera-Core.hpp>
#include <zlib.h>
#include <chrono>
export module Visera.Core.Compression;
#define VISERA_MODULE_NAME "Core.Compression"
import Visera.Core.Log;
import Visera.Core.OS.Memory;
import Visera.Core.Types.Array;

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
        const UInt32 SourceSize = I_Buffer.size();

        if (SourceSize == 0) { return ECompressionStatue::Success; }

        // Memory layout of CompressedData:
        // [ 4 bytes: original size ] [ compressed data... ]
        //   ^                         ^
        //   |                         |
        // sizeof(UInt32) header       Actual zlib compressed data
        uLongf CompressedSize = compressBound(SourceSize);
        O_Buffer->resize(CompressedSize + sizeof(UInt32));

        // Store original size at the beginning for decompression
        Memory::Memcpy(O_Buffer->data(), &SourceSize, sizeof(UInt32));

        // Compress the data
        auto Result = compress(
            O_Buffer->data() + sizeof(Int32),  // Destination (after size header)
            &CompressedSize,                           // Compressed size (in/out)
            SourceData,                                // Source data
            SourceSize                                 // Source size
        );

        if (Result == Z_OK)
        { O_Buffer->resize(CompressedSize + sizeof(UInt32)); }

        if (I_Buffer.size() < O_Buffer->size())
        LOG_DEBUG("Compression resulted in larger size ({} -> {})"
                  "-- Input may be too small or non-redundant.",
                  I_Buffer.size(), O_Buffer->size());

        return static_cast<ECompressionStatue>(Result);
    }
}