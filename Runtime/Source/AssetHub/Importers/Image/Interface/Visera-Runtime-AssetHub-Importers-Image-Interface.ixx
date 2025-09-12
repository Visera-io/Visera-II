module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.AssetHub.Importers.Image.Interface;
#define VISERA_MODULE_NAME "Runtime.AssetHub"
export import Visera.Runtime.AssetHub.Importers.Image.Common;

export namespace Visera
{
    class VISERA_RUNTIME_API IImageWrapper
    {
    public:
        virtual auto
        GetWidth()       const -> Int64 = 0;
        virtual auto
        GetHeight()      const -> Int64 = 0;
        virtual auto
        GetColorFormat() const -> EColorFormat = 0;
        virtual auto
        GetBitDepth()    const -> Int32        = 0;
        auto
        GetAspectRatio() const -> Float { return static_cast<Float>(GetWidth()) / GetHeight(); };
        auto
        GetDebugName(const char8_t* I_DebugName) const { return DebugName; }

        void
        SetDebugName(const char8_t* I_DebugName) { VISERA_ASSERT(I_DebugName != nullptr); DebugName = I_DebugName; }

        Bool
        IsSRGB()         const { return GetBitDepth() == 8 && GetColorFormat() != EColorFormat::BGRExp; }

    protected:
        const char8_t* DebugName = nullptr;

    public:
        virtual ~IImageWrapper() {}
    };

}