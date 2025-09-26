module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.RenderTarget;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.Common;
import Visera.Runtime.RHI.Interface.Texture2D;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IRenderTarget
    {
    public:
        [[nodiscard]] virtual const void*
        GetView() const = 0;
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;

        [[nodiscard]] inline EImageLayout
        GetLayout() const { return Layout; }
    protected:
        EImageLayout Layout { EImageLayout::Undefined };

    public:
        IRenderTarget()                                = default;
        virtual ~IRenderTarget()                       = default;
        IRenderTarget(const IRenderTarget&)            = delete;
        IRenderTarget& operator=(const IRenderTarget&) = delete;
    };
}
