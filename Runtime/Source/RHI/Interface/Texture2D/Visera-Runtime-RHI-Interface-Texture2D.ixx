module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.Texture2D;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.Common;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API ITexture2D
    {
        friend class ICommandBuffer;
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;
        [[nodiscard]] virtual const void*
        GetView()   const = 0;

        [[nodiscard]] inline EImageLayout
        GetLayout() const { return Layout; }

    protected:
        EImageLayout Layout { EImageLayout::Undefined };

    public:
        ITexture2D()                             = default;
        virtual ~ITexture2D()                    = default;
        ITexture2D(const ITexture2D&)            = delete;
        ITexture2D& operator=(const ITexture2D&) = delete;
    };
}