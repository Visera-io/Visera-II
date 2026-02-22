module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.RenderPass;
#define VISERA_MODULE_NAME "Runtime.Graphics"
       import Visera.Runtime.Graphics.Context;
       import Visera.Runtime.RHI;

export namespace Visera
{
    class VISERA_RUNTIME_API IRenderPass
    {
    public:
        virtual void Setup() = 0;
        virtual void Execute(FRenderContext& IO_Ctx, FRHICommandList& IO_CmdList) = 0;
        virtual void Teardown() = 0;

        virtual ~IRenderPass() = default;

    protected:
        IRenderPass() = default;
        IRenderPass(const IRenderPass&) = delete;
        IRenderPass& operator=(const IRenderPass&) = delete;
        IRenderPass(IRenderPass&&) = default;
        IRenderPass& operator=(IRenderPass&&) = default;
    };
}
