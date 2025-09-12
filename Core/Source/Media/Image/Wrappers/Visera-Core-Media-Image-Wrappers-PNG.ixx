module;
#include <Visera-Core.hpp>
#include <png.h>
export module Visera.Core.Media.Image.Wrappers:PNG;
#define VISERA_MODULE_NAME "Core.Media"
import Visera.Core.Log;
import :Interface;

export namespace Visera
{
    class VISERA_CORE_API FPNGImageWrapper : public IImageWrapper
    {
    public:
        virtual auto
        GetWidth()       const -> Int64        override { return Width; };
        virtual auto
        GetHeight()      const -> Int64        override { return Height; };
        virtual auto
        GetColorFormat() const -> EColorFormat override { return ColorFormat; };
        virtual auto
        GetBitDepth()    const -> Int32        override { return BitDepth; };

        void Parse(const char* Path)
        {
            BeginParsing(Path);
            {
                Preprocessing();
                DetectFormat();
                DetectColorSpace();
            }
            EndParsing();
        };

    private:
        png_structp Handle = nullptr;
        png_infop   Info   = nullptr;
        FILE*       File   = nullptr;
        const char* Path   = ""; //[TODO]: Remove?

        EColorFormat ColorFormat = EColorFormat::Invalid;
        Int64  Width    = 0;
        Int64  Height   = 0;
        UInt8  Channels = 0;
        UInt8  BitDepth = 0;
        Int32  SRGBIntent{-1};
        Double Gamma     {1.0};

        void BeginParsing(const char* Path);
        void Preprocessing();
        void DetectFormat();
        void DetectColorSpace();
        void EndParsing();
        
    public:
        FPNGImageWrapper() = default;
    };

    void FPNGImageWrapper::
    BeginParsing(const char* I_Path)
    {
        Path = I_Path;
        File = fopen(Path, "rb");
        if (!File)
        {
            FString ErrorInfo = fmt::format("Failed to read the image ({})! -- throw(SIOFailure)", Path);
            LOG_ERROR("{}", ErrorInfo);
            VISERA_WIP; // throw SIOFailure(ErrorInfo);
        }

        png_byte header[8];
        if (fread(header, 1, 8, File) != 8)
        {
            FString ErrorInfo = fmt::format("Failed to read PNG signature bytes from ({}).", Path);
            LOG_ERROR("{}", ErrorInfo);
            fclose(File);
            VISERA_WIP; // throw SIOFailure(ErrorInfo);
        }

        if (png_sig_cmp(header, 0, 8))
        {
            FString ErrorInfo = fmt::format("The image ({}) is an invalid PNG! -- throw(SIOFailure)", Path);
            LOG_ERROR("{}", ErrorInfo);
            fclose(File);
            VISERA_WIP; // throw SIOFailure(ErrorInfo);
        }

        Handle = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!Handle)
        {
            FString ErrorInfo = fmt::format("Failed to png_create_read_struct({})! -- throw(SIOFailure)", Path);
            LOG_ERROR("{}", ErrorInfo);
            fclose(File);
            VISERA_WIP; // throw SIOFailure(ErrorInfo);
        }

        Info = png_create_info_struct(Handle);
        if (!Info)
        {
            FString ErrorInfo = fmt::format("Failed to png_create_info_struct({})! -- throw(SIOFailure)", Path);
            LOG_ERROR("{}", ErrorInfo);
            png_destroy_read_struct(&Handle, nullptr, nullptr);
            fclose(File);
            VISERA_WIP; // throw SIOFailure(ErrorInfo);
        }

        if (setjmp(png_jmpbuf(Handle)))
        {
            FString ErrorInfo = fmt::format("Error during processing({})! -- throw(SIOFailure)", Path);
            LOG_ERROR("{}", ErrorInfo);
            png_destroy_read_struct(&Handle, &Info, nullptr);
            fclose(File);
            VISERA_WIP; // throw SIOFailure(ErrorInfo);
        }

        png_init_io(Handle, File);
        png_set_sig_bytes(Handle, 8); // We already read 8 bytes from the header
        
        png_read_info(Handle, Info); //MUST read metadata at first
    }

    void FPNGImageWrapper::
    Preprocessing()
    {
        auto ColorType = png_get_color_type(Handle, Info);
        BitDepth  = png_get_bit_depth(Handle, Info);
        // Force to be 8bits (enough for human's visual perception)
        if (ColorType == PNG_COLOR_TYPE_PALETTE)
        {
            LOG_WARN("The image ({}) is a palette and has been converted to RGB!", Path);
            png_set_palette_to_rgb(Handle);
        }
        if (png_get_valid(Handle, Info, PNG_INFO_tRNS))
        {
            LOG_WARN("The image ({}) has tRNS, and has been converted to full alpha!", Path);
            png_set_tRNS_to_alpha(Handle);
        }
        if (BitDepth > 8)
        {
            if (BitDepth != 16) { LOG_ERROR("Unsupported bit depth ({}) of image ({})!", BitDepth, Path); }
            else
            {
                LOG_WARN("The system do NOT support 16bits image ({}), it has be converted to 8bits image!", Path);
                png_set_strip_16(Handle);
            }
        }
        if (BitDepth < 8)
        {
            LOG_WARN("The bit depth of image ({}) < 8bits, and it has been converted to 8bits!", Path);
            if (ColorType == PNG_COLOR_TYPE_GRAY)
            { png_set_expand_gray_1_2_4_to_8(Handle); }
            else
            { png_set_packing(Handle); }
        }
        png_read_update_info(Handle, Info); // Once you update the data, you MUST call "png_read_update_info"

        BitDepth = 8;
        Width    = png_get_image_width(Handle,  Info);
        Height   = png_get_image_height(Handle, Info);
        Channels = png_get_channels(Handle,     Info);
    }

    void FPNGImageWrapper::
    DetectFormat()
    {
        //libpng read PNG images in RGBA order.
        const auto ColorType = png_get_color_type(Handle, Info);
        switch (ColorType)
        {
            case PNG_COLOR_TYPE_RGB:
                ColorFormat = EColorFormat::RGB;
                break;
            case PNG_COLOR_TYPE_RGB_ALPHA:
                ColorFormat = EColorFormat::RGBA;
                break;
            case PNG_COLOR_TYPE_GRAY:
                LOG_WARN("The gray image ({}) is NOT safely supported!", Path);
                ColorFormat = EColorFormat::Gray;
                break;
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                LOG_WARN("The gray alpha image ({}) is NOT safely supported!", Path);
                ColorFormat = EColorFormat::Invalid;
                break;
            default:
                LOG_ERROR("The format of image ({}) is NOT supported!", Path);
                ColorFormat = EColorFormat::Invalid;
        }
    }

    void FPNGImageWrapper::
    DetectColorSpace()
    {
        if (png_get_valid(Handle, Info, PNG_INFO_sRGB))
        {
            png_get_sRGB(Handle, Info, &SRGBIntent);
        }

        if (png_get_valid(Handle, Info, PNG_INFO_gAMA))
        {
            png_get_gAMA(Handle, Info, &Gamma);
            if (SRGBIntent == -1)
            { LOG_WARN("The image ({}) may not be the sRGB but the gAMA was found, set as sRGB!", Path); }
        }
        else
        {
            if (SRGBIntent != -1)
            {
                LOG_WARN("The image ({}) is sRGB but the gAMA was not found in the metadata, set as 1.0/2.2 by default!", Path);
                Gamma = 1.0/2.2;
            }
        }
    }

    void FPNGImageWrapper::
    EndParsing()
    {
        VISERA_ASSERT(File);
        fclose(File);
        Path = "";
        png_destroy_read_struct(&Handle, &Info, nullptr);
    }
}
