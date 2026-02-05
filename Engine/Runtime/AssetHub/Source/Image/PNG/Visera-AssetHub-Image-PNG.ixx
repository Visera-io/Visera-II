module;
#include <Visera-AssetHub.hpp>
#include <png.h>
export module Visera.AssetHub.Image.PNG;
#define VISERA_MODULE_NAME "AssetHub.Image"
export import Visera.AssetHub.Image.Common;
       import Visera.AssetHub.Image.Wrapper;
       import Visera.Core.OS.FileSystem;
       import Visera.Core.Types.Array;
       import Visera.Global.Log;

export namespace Visera
{
    /**
     * PNG image loader implementing IImageWrapper interface.
     * Internal use only, not exposed to users.
     */
    class VISERA_ASSETHUB_API FPNGImageWrapper : public IImageWrapper
    {
    public:
        [[nodiscard]] TSharedPtr<FImage>
        Import(const FPath& I_Path) override;

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

    TSharedPtr<FImage> FPNGImageWrapper::
    Import(const FPath& I_Path)
    {
        if (!BeginParsing(I_Path))
        {
            LOG_ERROR("Failed to parse the image {}!", I_Path);
            return nullptr;
        }

        Preprocessing();
        DetectFormat();
        DetectColorSpace();

        if (PixelFormat == EPixelFormat::Invalid)
        {
            LOG_ERROR("Invalid pixel format for PNG: {}", I_Path);
            EndParsing();
            return nullptr;
        }

        auto CreateInfo = FImage::FCreateInfo
        {
            .Width      = Width,
            .Height     = Height,
            .Depth      = 1,
            .PixelFormat= PixelFormat,
            .ColorSpace = ColorSpace,
        };

        auto Image = MakeShared<FImage>(CreateInfo);
        if (!Image)
        {
            LOG_ERROR("Failed to create FImage for PNG: {}", I_Path);
            EndParsing();
            return nullptr;
        }

        // Read image data directly into FImage's data buffer
        const UInt32 RowBytes = static_cast<UInt32>(png_get_rowbytes(PNGHandle, PNGInfo));
        FByte* ImageData = Image->AccessData();

        // Read rows directly into FImage's data buffer
        for (UInt32 Row = 0; Row < Height; ++Row)
        {
            png_read_row(PNGHandle, &ImageData[Row * RowBytes], nullptr);
        }

        // Read end of image
        png_read_end(PNGHandle, PNGInfo);

        // Cleanup
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
}
