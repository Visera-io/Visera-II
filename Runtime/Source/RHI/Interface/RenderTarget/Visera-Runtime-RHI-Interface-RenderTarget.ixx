module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.RenderTarget;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.Texture2D;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IRenderTarget
    {
    public:
        [[nodiscard]] inline const void*
        GetView() const { return Handle->GetView(); }
        [[nodiscard]] inline auto
        GetLayout() const { return Handle->GetLayout(); }
        [[nodiscard]] inline TSharedPtr<ITexture2D>
        GetHandle() const { return Handle; };

    protected:
        TSharedPtr<ITexture2D> Handle;

    public:
        IRenderTarget()                                = delete;
        IRenderTarget(TSharedPtr<ITexture2D> I_Handle) : Handle{I_Handle} {}
        virtual ~IRenderTarget()                       = default;
        IRenderTarget(const IRenderTarget&)            = delete;
        IRenderTarget& operator=(const IRenderTarget&) = delete;
    };
}
