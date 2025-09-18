module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Image.Importer;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.Image.Common;
export import Visera.Core.Types.Path;

namespace Visera
{
    export class VISERA_RUNTIME_API IImageImporter
    {
    public:
        [[nodiscard]] virtual auto
        Import(const FPath& I_Path) -> TArray<FByte> = 0;

        [[nodiscard]] virtual auto
        GetWidth()       const -> UInt32 = 0;
        [[nodiscard]] virtual auto
        GetHeight()      const -> UInt32 = 0;
        [[nodiscard]] virtual auto
        GetColorFormat() const -> EColorFormat = 0;
        [[nodiscard]] virtual auto
        GetBitDepth()    const -> UInt8  = 0;
        [[nodiscard]] auto
        GetAspectRatio() const -> Float { return static_cast<Float>(GetWidth()) / GetHeight(); }

        Bool
        IsSRGB() const { return GetBitDepth() == 8 && GetColorFormat() != EColorFormat::BGRExp; }

    public:
        virtual ~IImageImporter() = default;
    };

}