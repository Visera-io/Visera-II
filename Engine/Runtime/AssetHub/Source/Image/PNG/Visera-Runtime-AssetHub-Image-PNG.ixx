module;
#include <Visera-AssetHub.hpp>
#include <png.h>
export module Visera.Runtime.AssetHub.Image.PNG;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.Image.Common;
       import Visera.Runtime.AssetHub.Image.Wrapper;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Types.Array;
       import Visera.Core.Log;

export namespace Visera
{
    /**
     * PNG image loader implementing IImageWrapper interface.
     * Internal use only, not exposed to users.
     */
    class VISERA_RUNTIME_API FPNGImageWrapper : public IImageWrapper
    {
    public:
        [[nodiscard]] FImage
        Import(const FPath& I_Path) override;

        [[nodiscard]] Bool
        Export(const FImage& I_Image, const FPath& I_Path) override;

    private:
        TUniquePtr<FFile> File;
        png_structp PNGHandle = nullptr;
        png_infop   PNGInfo   = nullptr;

        EPixelFormat PixelFormat = EPixelFormat::Invalid;
        EColorSpace  ColorSpace  = EColorSpace::Linear;
        UInt32       Width      {0};
        UInt32       Height     {0};
        UInt8        Channels   {0};
        Int32        SRGBIntent {-1};
        Double       Gamma      {1.0};

    public:
        FPNGImageWrapper() = default;
        ~FPNGImageWrapper() override;

    private:
        Bool BeginParsing(const FPath& I_Path);
        void Preprocessing();
        void DetectFormat();
        void DetectColorSpace();
        void EndParsing();
    };

    FPNGImageWrapper::
    ~FPNGImageWrapper()
    {
        EndParsing();
    }

    FImage FPNGImageWrapper::
    Import(const FPath& I_Path)
    {
        if (!BeginParsing(I_Path))
        {
            LOG_ERROR("Failed to parse the image {}!", I_Path);
            return {};
        }

        Preprocessing();
        DetectFormat();
        DetectColorSpace();

        if (PixelFormat == EPixelFormat::Invalid)
        {
            LOG_ERROR("Invalid pixel format for PNG: {}", I_Path);
            EndParsing();
            return {};
        }

        FImage::FCreateInfo CreateInfo
        {
            .Width      = Width,
            .Height     = Height,
            .Depth      = 1,
            .PixelFormat= PixelFormat,
            .ColorSpace = ColorSpace,
        };

        FImage Image(CreateInfo);

        const UInt32 RowBytes = static_cast<UInt32>(png_get_rowbytes(PNGHandle, PNGInfo));
        FByte* ImageData = Image.AccessData();

        for (UInt32 Row = 0; Row < Height; ++Row)
        {
            png_read_row(PNGHandle, &ImageData[Row * RowBytes], nullptr);
        }

        png_read_end(PNGHandle, PNGInfo);
        EndParsing();

        return Image;
    }

    Bool FPNGImageWrapper::
    BeginParsing(const FPath& I_Path)
    {
        // Open file
        File = FFileSystem::OpenFile(I_Path, EFileMode::Read | EFileMode::Binary);
        if (!File || !File->IsOpen())
        {
            LOG_ERROR("Failed to open PNG file: {}", I_Path);
            return False;
        }

        // Read PNG signature (8 bytes)
        FByte Header[8] = {0};
        const UInt64 BytesRead = File->Read(Header, 1, 8);
        if (BytesRead != 8)
        {
            LOG_ERROR("Failed to read PNG signature bytes from: {}", I_Path);
            return False;
        }

        // Verify PNG signature
        if (png_sig_cmp(Header, 0, 8) != 0)
        {
            LOG_ERROR("Invalid PNG signature in file: {}", I_Path);
            return False;
        }

        // Create PNG read structure
        PNGHandle = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!PNGHandle)
        {
            LOG_ERROR("Failed to create PNG read structure: {}", I_Path);
            return False;
        }

        // Create PNG info structure
        PNGInfo = png_create_info_struct(PNGHandle);
        if (!PNGInfo)
        {
            LOG_ERROR("Failed to create PNG info structure: {}", I_Path);
            png_destroy_read_struct(&PNGHandle, nullptr, nullptr);
            return False;
        }

        // Set error handling with setjmp
        if (setjmp(png_jmpbuf(PNGHandle)))
        {
            LOG_ERROR("Error during PNG processing: {}", I_Path);
            png_destroy_read_struct(&PNGHandle, &PNGInfo, nullptr);
            PNGHandle = nullptr;
            PNGInfo = nullptr;
            return False;
        }

        // Initialize I/O with FILE* handle
        png_init_io(PNGHandle, File->GetHandle());
        png_set_sig_bytes(PNGHandle, 8); // We already read 8 bytes

        // Read PNG info
        png_read_info(PNGHandle, PNGInfo);

        return True;
    }

    void FPNGImageWrapper::
    Preprocessing()
    {
        const auto ColorType = png_get_color_type(PNGHandle, PNGInfo);
        auto BitDepth = png_get_bit_depth(PNGHandle, PNGInfo);

        // Convert palette to RGB
        if (ColorType == PNG_COLOR_TYPE_PALETTE)
        {
            LOG_WARN("PNG image is palette-based, converting to RGB");
            png_set_palette_to_rgb(PNGHandle);
        }

        // Convert transparency to alpha
        if (png_get_valid(PNGHandle, PNGInfo, PNG_INFO_tRNS))
        {
            LOG_WARN("PNG image has tRNS, converting to full alpha");
            png_set_tRNS_to_alpha(PNGHandle);
        }

        // Strip 16-bit to 8-bit
        if (BitDepth > 8)
        {
            if (BitDepth == 16)
            {
                LOG_WARN("PNG image is 16-bit, converting to 8-bit");
                png_set_strip_16(PNGHandle);
            }
            else
            {
                LOG_ERROR("Unsupported PNG bit depth: {}", BitDepth);
            }
        }

        // Expand low bit depth
        if (BitDepth < 8)
        {
            LOG_WARN("PNG image bit depth < 8, expanding to 8-bit");
            if (ColorType == PNG_COLOR_TYPE_GRAY)
            { png_set_expand_gray_1_2_4_to_8(PNGHandle); }
            else
            { png_set_packing(PNGHandle); }
        }

        // Add alpha channel if missing (for GPU compatibility)
        if (png_get_channels(PNGHandle, PNGInfo) < 4)
        {
            LOG_WARN("PNG image has < 4 channels, adding alpha channel for GPU compatibility");
            png_set_add_alpha(PNGHandle, 0xFF, PNG_FILLER_AFTER);
        }

        // Update info after transformations
        png_read_update_info(PNGHandle, PNGInfo);

        Width    = static_cast<UInt32>(png_get_image_width(PNGHandle, PNGInfo));
        Height   = static_cast<UInt32>(png_get_image_height(PNGHandle, PNGInfo));
        Channels = static_cast<UInt8> (png_get_channels(PNGHandle, PNGInfo));
    }

    void FPNGImageWrapper::
    DetectFormat()
    {
        // libpng reads PNG images in RGBA order
        switch (Channels)
        {
        case 1:
            PixelFormat = EPixelFormat::R8_UNorm;
            break;
        case 2:
            PixelFormat = EPixelFormat::RG8_UNorm;
            break;
        case 3:
            PixelFormat = EPixelFormat::RGB8_UNorm;
            break;
        case 4:
            PixelFormat = EPixelFormat::RGBA8_UNorm;
            break;
        default:
            LOG_ERROR("Unsupported channel count: {}", Channels);
            PixelFormat = EPixelFormat::Invalid;
            break;
        }
    }

    void FPNGImageWrapper::
    DetectColorSpace()
    {
        // Check for sRGB chunk
        if (png_get_valid(PNGHandle, PNGInfo, PNG_INFO_sRGB))
        {
            png_get_sRGB(PNGHandle, PNGInfo, &SRGBIntent);
            ColorSpace = EColorSpace::sRGB;
            return;
        }

        // Check for gAMA chunk
        if (png_get_valid(PNGHandle, PNGInfo, PNG_INFO_gAMA))
        {
            png_get_gAMA(PNGHandle, PNGInfo, &Gamma);
            // If gamma is close to 1/2.2 (sRGB), treat as sRGB
            if (Gamma > 0.45 && Gamma < 0.46)
            {
                LOG_WARN("PNG has gAMA chunk, assuming sRGB color space");
                ColorSpace = EColorSpace::sRGB;
            }
            else
            {
                ColorSpace = EColorSpace::Linear;
            }
        }
        else
        {
            // Default to sRGB for 8-bit images (common case)
            ColorSpace = EColorSpace::sRGB;
        }
    }

    void FPNGImageWrapper::
    EndParsing()
    {
        if (PNGHandle != nullptr)
        {
            png_destroy_read_struct(&PNGHandle, &PNGInfo, nullptr);
            PNGHandle = nullptr;
            PNGInfo   = nullptr;
        }
    }

    Bool FPNGImageWrapper::
    Export(const FImage& I_Image, const FPath& I_Path)
    {
        const EPixelFormat Format = I_Image.GetPixelFormat();
        const UInt32 Width = I_Image.GetWidth();
        const UInt32 Height = I_Image.GetHeight();

        // PNG only supports 8-bit and 16-bit formats
        // Convert to RGBA8 or RGBA16 based on format
        Int32 BitDepth = 8;
        Int32 ColorType = PNG_COLOR_TYPE_RGBA;
        UInt32 BytesPerChannel = 1;

        switch (Format)
        {
        case EPixelFormat::R8_UNorm:
            ColorType = PNG_COLOR_TYPE_GRAY;
            break;
        case EPixelFormat::RG8_UNorm:
            ColorType = PNG_COLOR_TYPE_GRAY_ALPHA;
            break;
        case EPixelFormat::RGB8_UNorm:
            ColorType = PNG_COLOR_TYPE_RGB;
            break;
        case EPixelFormat::RGBA8_UNorm:
            ColorType = PNG_COLOR_TYPE_RGBA;
            break;
        case EPixelFormat::R16_UNorm:
            BitDepth = 16;
            BytesPerChannel = 2;
            ColorType = PNG_COLOR_TYPE_GRAY;
            break;
        case EPixelFormat::RG16_UNorm:
            BitDepth = 16;
            BytesPerChannel = 2;
            ColorType = PNG_COLOR_TYPE_GRAY_ALPHA;
            break;
        case EPixelFormat::RGB16_UNorm:
            BitDepth = 16;
            BytesPerChannel = 2;
            ColorType = PNG_COLOR_TYPE_RGB;
            break;
        case EPixelFormat::RGBA16_UNorm:
            BitDepth = 16;
            BytesPerChannel = 2;
            ColorType = PNG_COLOR_TYPE_RGBA;
            break;
        default:
            LOG_ERROR("Unsupported pixel format for PNG export: {}", static_cast<Int32>(Format));
            return False;
        }

        // Open file for writing
        auto File = FFileSystem::OpenFile(I_Path, EFileMode::Write | EFileMode::Binary);
        if (!File || !File->IsOpen())
        {
            LOG_ERROR("Failed to open PNG file for writing: {}", I_Path);
            return False;
        }

        // Create PNG write structures
        png_structp PNGWrite = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!PNGWrite)
        {
            LOG_ERROR("Failed to create PNG write structure: {}", I_Path);
            return False;
        }

        png_infop PNGInfoWrite = png_create_info_struct(PNGWrite);
        if (!PNGInfoWrite)
        {
            LOG_ERROR("Failed to create PNG info structure: {}", I_Path);
            png_destroy_write_struct(&PNGWrite, nullptr);
            return False;
        }

        // Set error handling
        if (setjmp(png_jmpbuf(PNGWrite)))
        {
            LOG_ERROR("Error during PNG writing: {}", I_Path);
            png_destroy_write_struct(&PNGWrite, &PNGInfoWrite);
            return False;
        }

        // Initialize I/O
        png_init_io(PNGWrite, File->GetHandle());

        // Set image header
        png_set_IHDR(PNGWrite, PNGInfoWrite, Width, Height, BitDepth, ColorType,
                     PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        // Write header
        png_write_info(PNGWrite, PNGInfoWrite);

        // Get image data
        const FByte* ImageData = I_Image.GetData();
        const UInt32 RowPitch = I_Image.GetRowPitchBytes();

        // Write image rows
        for (UInt32 Row = 0; Row < Height; ++Row)
        {
            png_write_row(PNGWrite, const_cast<FByte*>(ImageData + Row * RowPitch));
        }

        // Write end
        png_write_end(PNGWrite, PNGInfoWrite);

        // Cleanup
        png_destroy_write_struct(&PNGWrite, &PNGInfoWrite);

        return True;
    }
}
