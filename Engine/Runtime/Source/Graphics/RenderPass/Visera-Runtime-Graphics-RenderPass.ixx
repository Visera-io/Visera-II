module;
#include <Visera-Runtime.hpp>
export module Visera.Runtime.Graphics.RenderPass;
#define VISERA_MODULE_NAME "Runtime.Graphics"
import Visera.Runtime.Graphics.RenderGraph;
import Visera.Core.Types.Function;

export namespace Visera
{
    /** Abstract render pass interface.
     *  Users implement Setup (declare resource access) and Execute (record GPU commands).
     *  The Graphics layer calls these per-frame to build the RenderGraph. */
    class VISERA_RUNTIME_API IRenderPass
    {
    public:
        virtual ~IRenderPass() = default;

        /** Declare which resources this pass reads/writes via the builder.
         *  Called once per frame before Execute. */
        virtual void Setup(FRDGPassBuilder& Builder) = 0;

        /** Record GPU commands for this pass.
         *  Called once per frame after the RenderGraph is compiled. */
        virtual void Execute(FRDGPassContext& Context) = 0;
    };

    /** Convenience adapter: wraps a pair of lambdas into an IRenderPass. */
    class VISERA_RUNTIME_API FLambdaRenderPass final : public IRenderPass
    {
    public:
        FLambdaRenderPass(TFunction<void(FRDGPassBuilder&)> I_Setup,
                          TFunction<void(FRDGPassContext&)> I_Execute)
            : SetupFunction(std::move(I_Setup))
            , ExecuteFunction(std::move(I_Execute)) {}

        void Setup(FRDGPassBuilder& Builder) override
        { if (SetupFunction) { SetupFunction(Builder); } }

        void Execute(FRDGPassContext& Context) override
        { if (ExecuteFunction) { ExecuteFunction(Context); } }

    private:
        TFunction<void(FRDGPassBuilder&)> SetupFunction;
        TFunction<void(FRDGPassContext&)> ExecuteFunction;
    };
}
