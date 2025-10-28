module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Image.Wrapper;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.Image.Common;
export import Visera.Core.Types.Path;

namespace Visera
{
    export class VISERA_RUNTIME_API IImageWrapper
    {
    public:
        [[nodiscard]] virtual auto
        Import(const FPath& I_Path) -> TArray<FByte> = 0;

        [[nodiscard]] virtual auto
        GetWidth()       const -> UInt32 = 0;
        [[nodiscard]] virtual auto
        GetHeight()      const -> UInt32 = 0;
        [[nodiscard]] virtual auto
        GetChannels()    const -> UInt8  = 0;
        [[nodiscard]] virtual auto
        GetColorFormat() const -> EColorFormat = 0;
        [[nodiscard]] virtual auto
        GetBitDepth()    const -> UInt8  = 0;
        [[nodiscard]] auto
        GetAspectRatio() const -> Float { return static_cast<Float>(GetWidth()) / GetHeight(); }

        virtual void
        SetWidth(UInt32 I_Width)   = 0;
        virtual void
        SetHeight(UInt32 I_Height) = 0;

        [[nodiscard]] Bool
        IsSRGB() const { return GetBitDepth() == 8 && GetColorFormat() != EColorFormat::BGRExp; }
        [[nodiscard]] Bool
        HasAlpha() const { return GetChannels() == 4; }
        [[nodiscard]] Bool
        IsRGBA() const { return GetColorFormat() == EColorFormat::RGBA; }
        [[nodiscard]] Bool
        IsBGRA() const { return GetColorFormat() == EColorFormat::BGRA; }

    public:
        virtual ~IImageWrapper() = default;
    };

}