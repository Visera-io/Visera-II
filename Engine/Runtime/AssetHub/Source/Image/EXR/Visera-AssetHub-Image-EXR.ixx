module;
#include <Visera-AssetHub.hpp>
#include <ImfRgbaFile.h>
#include <ImfArray.h>
export module Visera.AssetHub.Image.EXR;
#define VISERA_MODULE_NAME "AssetHub.Image"
export import Visera.AssetHub.Image.Common;
       import Visera.AssetHub.Image.Wrapper;
       import Visera.Global.Log;

export namespace Visera
{
    class VISERA_ASSETHUB_API FEXRImageWrapper : public IImageWrapper
    {
    public:
        [[nodiscard]] TSharedPtr<FImage>
        Import(const FPath& I_Path) override;

    private:

    public:
        FEXRImageWrapper() = default;
        ~FEXRImageWrapper() override = default;
    };

    TSharedPtr<FImage> FEXRImageWrapper::
    Import(const FPath& I_Path)
    {
        try
        {
            Imf::RgbaInputFile File(I_Path.GetUTF8Path().Data());
            const Imath::Box2i DW = File.dataWindow();

            const Int32 Width  = DW.max.x - DW.min.x + 1;
            const Int32 Height = DW.max.y - DW.min.y + 1;

            if (Width <= 0 || Height <= 0)
            {
                LOG_ERROR("Invalid EXR data window ({}x{}): {}", Width, Height, I_Path);
                return nullptr;
            }

            Imf::Array2D<Imf::Rgba> Pixels;
            Pixels.resizeErase(Height, Width);

            // The base pointer must be offset by -min for dataWindow
            File.setFrameBuffer(&Pixels[-DW.min.y][-DW.min.x], 1, Width);
            File.readPixels(DW.min.y, DW.max.y);

            auto CreateInfo = FImage::FCreateInfo
            {
                .Width       = static_cast<UInt32>(Width),
                .Height      = static_cast<UInt32>(Height),
                .Depth       = 1,
                .PixelFormat = EPixelFormat::RGBA16_Float,
                .ColorSpace  = EColorSpace::Linear,
            };

            auto Image = MakeShared<FImage>(CreateInfo);
            if (!Image)
            {
                LOG_ERROR("Failed to create FImage for EXR: {}", I_Path);
                return nullptr;
            }

            // Copy as RGBA half-floats (16-bit) into FImage buffer.
            // OpenEXR stores Imath::half components; we store raw half bits.
            FByte* Out = Image->AccessData();
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
            LOG_ERROR("Failed to load EXR \"{}\": {}", I_Path, Ex.what());
            return nullptr;
        }
        catch (...)
        {
            LOG_ERROR("Failed to load EXR \"{}\": unknown error", I_Path);
            return nullptr;
        }
    }
}
