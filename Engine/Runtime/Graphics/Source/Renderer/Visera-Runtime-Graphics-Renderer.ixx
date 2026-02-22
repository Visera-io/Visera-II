module;
#include <Visera-Graphics.hpp>
export module Visera.Runtime.Graphics.Renderer;
#define VISERA_MODULE_NAME "Runtime.Graphics"
       import Visera.Runtime.Graphics.Context;
       import Visera.Runtime.Graphics.Scene;
       import Visera.Runtime.RHI;
       import Visera.Core.Types.Pointer;

export namespace Visera
{
    class VISERA_RUNTIME_API IRenderer
    {
    public:
        virtual void Setup() = 0;
        virtual void Render(FRenderContext& IO_Ctx, FRHICommandList& IO_CmdList, const FRHIRenderPassAttachments& I_Attachments) = 0;
        virtual void Teardown() = 0;

        virtual ~IRenderer() = default;

    protected:
        TSharedPtr<FRHI>   RHI;
        TSharedPtr<FScene> Scene;

        IRenderer(TSharedPtr<FRHI> I_RHI, TSharedPtr<FScene> I_Scene)
            : RHI(std::move(I_RHI)), Scene(std::move(I_Scene)) {}

        IRenderer() = default;
        IRenderer(const IRenderer&) = delete;
        IRenderer& operator=(const IRenderer&) = delete;
        IRenderer(IRenderer&&) = default;
        IRenderer& operator=(IRenderer&&) = default;
    };

    namespace Concepts
    {
        template<typename T> concept
        Renderer = std::derived_from<T, IRenderer>;
    }
}
