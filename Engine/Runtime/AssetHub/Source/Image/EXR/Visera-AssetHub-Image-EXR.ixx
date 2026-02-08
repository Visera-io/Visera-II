module;
#include <Visera-AssetHub.hpp>
#include <ImfRgbaFile.h>
#include <ImfArray.h>
export module Visera.AssetHub.Image.EXR;
#define VISERA_MODULE_NAME "AssetHub.Image"
export import Visera.AssetHub.Image.Common;
       import Visera.AssetHub.Image.Wrapper;
       import Visera.Core.Types.Half;
       import Visera.Global.Log;

export namespace Visera
{
    class VISERA_ASSETHUB_API FEXRImageWrapper : public IImageWrapper
    {
    public:
        [[nodiscard]] FImage
        Import(const FPath& I_Path) override;

        [[nodiscard]] Bool
        Export(const FImage& I_Image, const FPath& I_Path) override;

    public:
        FEXRImageWrapper() = default;
        ~FEXRImageWrapper() override = default;
    };

    FImage FEXRImageWrapper::
    Import(const FPath& I_Path)
    {
        try
        {
            Imf::RgbaInputFile File(I_Path.GetString().Data());
            const Imath::Box2i DW = File.dataWindow();

            const Int32 Width  = DW.max.x - DW.min.x + 1;
            const Int32 Height = DW.max.y - DW.min.y + 1;

            if (Width <= 0 || Height <= 0)
            {
                LOG_ERROR("Invalid EXR data window ({}x{}): {}", Width, Height, I_Path);
                return {};
            }

            Imf::Array2D<Imf::Rgba> Pixels;
            Pixels.resizeErase(Height, Width);

            File.setFrameBuffer(&Pixels[-DW.min.y][-DW.min.x], 1, Width);
            File.readPixels(DW.min.y, DW.max.y);

            FImage::FCreateInfo CreateInfo
            {
                .Width       = static_cast<UInt32>(Width),
                .Height      = static_cast<UInt32>(Height),
                .Depth       = 1,
                .PixelFormat = EPixelFormat::RGBA16_Float,
                .ColorSpace  = EColorSpace::Linear,
            };

            FImage Image(CreateInfo);

            FByte* Out = Image.AccessData();
            auto*  OutU16 = reinterpret_cast<UInt16*>(Out);

            UInt64 WriteIndex = 0;
            for (Int32 Y = 0; Y < Height; ++Y)
            {
                for (Int32 X = 0; X < Width; ++X)
                {
                    const Imf::Rgba& P = Pixels[Y][X];
                    OutU16[WriteIndex + 0] = P.r.bits();
                    OutU16[WriteIndex + 1] = P.g.bits();
                    OutU16[WriteIndex + 2] = P.b.bits();
                    OutU16[WriteIndex + 3] = P.a.bits();
                    WriteIndex += 4;
                }
            }

            return Image;
        }
        catch (const std::exception& Ex)
        {
            LOG_ERROR("Failed to load EXR {}: {}", I_Path, Ex.what());
            return {};
        }
        catch (...)
        {
            LOG_ERROR("Failed to load EXR {}: unknown error", I_Path);
            return {};
        }
    }

    Bool FEXRImageWrapper::
    Export(const FImage& I_Image, const FPath& I_Path)
    {
        const EPixelFormat Format = I_Image.GetPixelFormat();
        const UInt32 Width = I_Image.GetWidth();
        const UInt32 Height = I_Image.GetHeight();

        // EXR supports float formats (16-bit half or 32-bit float)
        // Determine if we should use half-float or full float
        Bool UseHalfFloat = True;
        switch (Format)
        {
        case EPixelFormat::RGBA16_Float:
            UseHalfFloat = True;
            break;
        case EPixelFormat::RGBA32_Float:
            UseHalfFloat = False;
            break;
        default:
            LOG_ERROR("Unsupported pixel format for EXR export: {} (only RGBA16_Float and RGBA32_Float are supported)", 
                     static_cast<Int32>(Format));
            return False;
        }

        try
        {
            // Prepare pixel data
            Imf::Array2D<Imf::Rgba> Pixels;
            Pixels.resizeErase(Height, Width);

            const FByte* ImageData = I_Image.GetData();
            const UInt32 RowPitch = I_Image.GetRowPitchBytes();

            if (UseHalfFloat)
            {
                // RGBA16_Float: read as UInt16 and convert to half using FHalf
                for (UInt32 Y = 0; Y < Height; ++Y)
                {
                    const UInt16* RowData = reinterpret_cast<const UInt16*>(ImageData + Y * RowPitch);
                    for (UInt32 X = 0; X < Width; ++X)
                    {
                        const UInt32 Index = X * 4;
                        Pixels[Y][X].r = FHalf::FromBits(RowData[Index + 0]).Value;
                        Pixels[Y][X].g = FHalf::FromBits(RowData[Index + 1]).Value;
                        Pixels[Y][X].b = FHalf::FromBits(RowData[Index + 2]).Value;
                        Pixels[Y][X].a = FHalf::FromBits(RowData[Index + 3]).Value;
                    }
                }
            }
            else
            {
                // RGBA32_Float: read as Float and convert to half using FHalf
                for (UInt32 Y = 0; Y < Height; ++Y)
                {
                    const Float* RowData = reinterpret_cast<const Float*>(ImageData + Y * RowPitch);
                    for (UInt32 X = 0; X < Width; ++X)
                    {
                        const UInt32 Index = X * 4;
                        Pixels[Y][X].r = FHalf(RowData[Index + 0]).Value;
                        Pixels[Y][X].g = FHalf(RowData[Index + 1]).Value;
                        Pixels[Y][X].b = FHalf(RowData[Index + 2]).Value;
                        Pixels[Y][X].a = FHalf(RowData[Index + 3]).Value;
                    }
                }
            }

            // Create EXR file
            Imf::RgbaOutputFile File(
                I_Path.GetString().Data(),
                Width, Height,
                Imf::WRITE_RGBA
            );

            // Set frame buffer and write pixels
            File.setFrameBuffer(&Pixels[0][0], 1, Width);
            File.writePixels(Height);

            return True;
        }
        catch (const std::exception& Ex)
        {
            LOG_ERROR("Failed to save EXR {}: {}", I_Path, Ex.what());
            return False;
        }
        catch (...)
        {
            LOG_ERROR("Failed to save EXR {}: unknown error", I_Path);
            return False;
        }
    }
}
