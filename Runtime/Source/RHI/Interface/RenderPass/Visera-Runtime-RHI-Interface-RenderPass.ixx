module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.RHI.Interface.RenderPass;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Interface.RenderTarget;

namespace Visera::RHI
{
    export class VISERA_RUNTIME_API IRenderPass
    {
    public:
        [[nodiscard]] virtual const void*
        GetHandle() const = 0;
        [[nodiscard]] virtual const void*
        GetPipeline() const = 0;

        [[nodiscard]] TSharedPtr<const IRenderTarget>
        GetRenderTarget() const { return CurrentRenderTarget; }
        void
        SetRenderTarget(TSharedPtr<IRenderTarget> I_RenderTarget) { VISERA_ASSERT(I_RenderTarget && I_RenderTarget->GetHandle()); CurrentRenderTarget = I_RenderTarget; }

    protected:
        TSharedPtr<IRenderTarget> CurrentRenderTarget;

    public:
        IRenderPass()                              = default;
        virtual ~IRenderPass()                     = default;
        IRenderPass(const IRenderPass&)            = delete;
        IRenderPass& operator=(const IRenderPass&) = delete;
    };
}
