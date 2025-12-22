module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderPass.Tonemap;
#define VISERA_MODULE_NAME "Graphics.RenderPass"
import Visera.RHI;
import Visera.Shader;

export namespace Visera
{
    /*
     * shader: Tonemap.slang
     */
    class VISERA_GRAPHICS_API FTonemapRenderPass
    {
    public:
        VISERA_PUSH_CONSTANT(
            UInt32 Width, Height;
            Float  Exposure{1.0}; // 1.0 = no change or 2^EV
            UInt32 bGammaCorrection{False};
        );


    private:
        TSharedPtr<FRHIRenderPipeline> Pipeline;
        FPushConstantRange             PushConstantRange;

    public:
        FTonemapRenderPass();
        ~FTonemapRenderPass();
    };

    FTonemapRenderPass::
    FTonemapRenderPass()
    {
        FRHIPushConstantRange PushConstantRange
        {
            .Size   = sizeof(FPushConstantRange),
            .Offset = 0,
            .Stages = ERHIShaderStages::All
        };
        FRHIRenderPipelineDesc PSO;
        PSO.Layout.AddPushConstant(PushConstantRange)
        ;
        Pipeline = GRHI->CreateRenderPipeline("Tonemap", PSO);
    }

    FTonemapRenderPass::
    ~FTonemapRenderPass()
    {

    }
}