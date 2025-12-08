module;
#include <Visera-Runtime.hpp>
#include <png.h>
export module Visera.Runtime.Media.Image.PNG;
#define VISERA_MODULE_NAME "Runtime.Media"
import Visera.Runtime.Media.Image.Wrapper;
import Visera.Core.Types.Array;
import Visera.Core.Log;

namespace Visera
{
    export class VISERA_RUNTIME_API FPNGImageWrapper : public IImageWrapper
    {
    public:
        [[nodiscard]] auto
        Import(const FPath& I_Path) -> TArray<FByte> override;

        [[nodiscard]] auto
        GetWidth()       const -> UInt32        override { return Width; }
        [[nodiscard]] auto
        GetHeight()      const -> UInt32        override { return Height; }
        [[nodiscard]] auto
        GetChannels()    const -> UInt8        override { return Channels; }
        [[nodiscard]] auto
        GetColorFormat() const -> EColorFormat override { return ColorFormat; }
        [[nodiscard]] auto
        GetBitDepth()    const -> UInt8        override { return BitDepth; }

        void
        SetWidth(UInt32 I_Width)   override { Width = I_Width; }
        void
        SetHeight(UInt32 I_Height) override { Height = I_Height; }

    private:
        png_structp Handle = nullptr;
        png_infop   Info   = nullptr;
        FILE*       File   = nullptr;

        EColorFormat ColorFormat = EColorFormat::Invalid;
        UInt32  Width   = 0;
        UInt32  Height  = 0;
        UInt8  Channels = 0;
        UInt8  BitDepth = 0;
        Int32  SRGBIntent{-1};
        Double Gamma     {1.0};

        Bool BeginParsing(const FPath& I_Path);
        void Preprocessing();
        void DetectFormat();
        void DetectColorSpace();
        void EndParsing();

    public:
        FPNGImageWrapper() = default;
    };

    TArray<FByte> FPNGImageWrapper::
    Import(const FPath& I_Path)
    {
        TArray<FByte> ImageData;

        if (BeginParsing(I_Path))
        {
            Preprocessing();
            DetectFormat();
            DetectColorSpace();

            const UInt64 RowBytes = png_get_rowbytes(Handle, Info);
            ImageData.resize(RowBytes * Height);

            for (UInt32 Row = 0; Row < Height; ++Row)
            {
                png_read_row(Handle, &ImageData[(Row) * RowBytes], nullptr);
                //png_read_row(_PNGPtr, &Data[(Height - Row - 1) * RowBytes], nullptr); // Flip Vertically
            }

            EndParsing();
        }
        else { LOG_ERROR("Failed to parse the image \"{}\"!", I_Path); }

        return ImageData;
    }

    Bool FPNGImageWrapper::
    BeginParsing(const FPath& I_Path)
    {
        auto Path = I_Path.GetUTF8Path();
        LOG_TRACE("Parsing the image \"{}\"", Path);

        File = fopen(Path.c_str(), "rb");
        if (!File)
        {
            LOG_ERROR("Failed to read the image ({})!", Path);
            return False;
        }

        png_byte header[8];
        if (fread(header, 1, 8, File) != 8)
        {
            LOG_ERROR("Failed to read PNG signature bytes from ({})!", Path);
            fclose(File);
            return False;
        }

        if (png_sig_cmp(header, 0, 8))
        {
            LOG_ERROR("The image ({}) is an invalid PNG!", Path);
            fclose(File);
            return False;
        }

        Handle = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (!Handle)
        {
            LOG_ERROR("Failed to png_create_read_struct({})!", Path);
            fclose(File);
            return False;
        }

        Info = png_create_info_struct(Handle);
        if (!Info)
        {
            LOG_ERROR("Failed to png_create_info_struct({})!", Path);
            png_destroy_read_struct(&Handle, nullptr, nullptr);
            fclose(File);
            return False;
        }

        if (setjmp(png_jmpbuf(Handle)))
        {
            LOG_ERROR("Error during processing({})!", Path);
            png_destroy_read_struct(&Handle, &Info, nullptr);
            fclose(File);
            return False;
        }

        png_init_io(Handle, File);
        png_set_sig_bytes(Handle, 8); // We already read 8 bytes from the header

        png_read_info(Handle, Info); //MUST read metadata at first

        return True;
    }

    void FPNGImageWrapper::
    Preprocessing()
    {
        const auto ColorType = png_get_color_type(Handle, Info);
        BitDepth  = png_get_bit_depth(Handle, Info);
        // Force to be 8bits (enough for human's visual perception)
        if (ColorType == PNG_COLOR_TYPE_PALETTE)
        {
            LOG_WARN("The image is a palette and has been converted to RGB!");
            png_set_palette_to_rgb(Handle);
        }
        if (png_get_valid(Handle, Info, PNG_INFO_tRNS))
        {
            LOG_WARN("The image has tRNS, and has been converted to full alpha!");
            png_set_tRNS_to_alpha(Handle);
        }
        if (BitDepth > 8)
        {
            if (BitDepth == 16)
            {
                LOG_WARN("The system do NOT support 16bits image, it has be converted to 8bits image!");
                png_set_strip_16(Handle);
            }
            else { LOG_ERROR("Unsupported bit depth ({}) of image!", BitDepth); }
        }
        if (BitDepth < 8)
        {
            LOG_WARN("The bit depth of image < 8bits, and it has been converted to 8bits!");
            if (ColorType == PNG_COLOR_TYPE_GRAY)
            { png_set_expand_gray_1_2_4_to_8(Handle); }
            else
            { png_set_packing(Handle); }
        }
        if (png_get_channels(Handle, Info) < 4)
        {
            LOG_WARN("Padding the Alpha channel because many GPUs do NOT support RGB(sRGB) format!");
            png_set_add_alpha(Handle, 0xFF, PNG_FILLER_AFTER);
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
                LOG_WARN("The gray image is NOT safely supported!");
                ColorFormat = EColorFormat::Gray;
                break;
            case PNG_COLOR_TYPE_GRAY_ALPHA:
                LOG_WARN("The gray alpha image is NOT safely supported!");
                ColorFormat = EColorFormat::Invalid;
                break;
            default:
                LOG_ERROR("The format of image is NOT supported!");
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
            { LOG_WARN("The image may not be the sRGB but the gAMA was found, set as sRGB!"); }
        }
        else
        {
            if (SRGBIntent != -1)
            {
                Gamma = 1.0/2.2;
                LOG_WARN("The image is sRGB but the gAMA was not found, set as {} by default!", Gamma);
            }
        }
    }

    void FPNGImageWrapper::
    EndParsing()
    {
        VISERA_ASSERT(File);
        fclose(File);
        png_destroy_read_struct(&Handle, &Info, nullptr);
    }
}
