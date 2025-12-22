module;
#include <Visera-Graphics.hpp>
export module Visera.Graphics.RenderPass.Tonemap;
#define VISERA_MODULE_NAME "Graphics.RenderPass"
import Visera.RHI;
import Visera.Shader;
import Visera.Platform;
import Visera.Runtime.Log;

export namespace Visera
{
    /*
     * shader: Tonemap.slang
     * Entry point: main
     * Bindings (Slang HLSL->SPIR-V mapping):
     *   - Binding 0: StorageImage (LDR output, gLDR, register(u0))
     *   - Binding 1: CombinedImageSampler (HDR input, gHDR, register(t0))
     */
    class VISERA_GRAPHICS_API FTonemapRenderPass
    {
    public:
        VISERA_PUSH_CONSTANT(
            UInt32 Width, Height;
            Float  Exposure;
            UInt32 srgbOut;
        );

        void
        Execute(TSharedRef<FRHICommandBuffer> I_CommandBuffer,
                TSharedRef<FRHIImageView>     I_HDRInput,
                TSharedRef<FRHIImageView>     I_LDROutput,
                TSharedRef<FRHISampler>       I_Sampler,
                UInt32                        I_Width,
                UInt32                        I_Height,
                Float                         I_Exposure = 1.0f,
                Bool                          I_bGammaCorrection = False);

        Float Exposure{1.0f}; // 1.0 = no change or 2^EV
        Bool  bGammaCorrection{False}; // Maps to srgbOut in shader

    private:
        TSharedPtr<FRHIComputePipeline>      Pipeline;
        TSharedPtr<FRHIDescriptorSet>        DescriptorSet;
        FRHIPushConstantRange                PushConstantRange;

    public:
        FTonemapRenderPass();
        ~FTonemapRenderPass();
    };

    FTonemapRenderPass::
    FTonemapRenderPass()
    {
        auto ShaderCode = GShader->Compile(FPath{"Tonemap.slang"}, "main");
        auto ComputeShader = GRHI->CreateShader(ERHIShaderStages::Compute, ShaderCode);
        if (!ComputeShader)
        { LOG_FATAL("Failed to create tonemap compute shader!"); }

        FRHIDescriptorSetLayout DescriptorSetLayout;

        DescriptorSetLayout
            // gLDR (Set:0 Binding 0)
            .AddBinding(FRHIDescriptorSetBinding::
                CombinedImageSampler(0, ERHIShaderStages::Compute))
            // gHDR (Set:0 Binding 1)
            .AddBinding(FRHIDescriptorSetBinding::
                StorageImage(1, ERHIShaderStages::Compute))
        ;
        DescriptorSet = GRHI->CreateDescriptorSet(DescriptorSetLayout);

        PushConstantRange = FRHIPushConstantRange
        {
            .Size   = sizeof(FPushConstantRange),
            .Offset = 0,
            .Stages = ERHIShaderStages::Compute
        };

        FRHIPipelineLayout PipelineLayout;
        PipelineLayout
            .AddPushConstant(PushConstantRange)
            .AddDescriptorSet(0, DescriptorSetLayout)
        ;
        // Create compute pipeline
        FRHIComputePipelineDesc ComputePSO;
        ComputePSO.Layout = PipelineLayout;
        ComputePSO.ComputeShader = ComputeShader;
        Pipeline = GRHI->CreateComputePipeline("Tonemap", ComputePSO);
        if (!Pipeline)
        { LOG_FATAL("Failed to create tonemap compute pipeline!"); }
    }

    FTonemapRenderPass::
    ~FTonemapRenderPass()
    {
        Pipeline.reset();
        DescriptorSet.reset();
    }

    void FTonemapRenderPass::
    Execute(TSharedRef<FRHICommandBuffer> I_CommandBuffer,
            TSharedRef<FRHIImageView>     I_HDRInput,
            TSharedRef<FRHIImageView>     I_LDROutput,
            TSharedRef<FRHISampler>       I_Sampler,
            UInt32                        I_Width,
            UInt32                        I_Height,
            Float                         I_Exposure,
            Bool                          I_bGammaCorrection)
    {
        DescriptorSet->WriteStorageImage(0, I_LDROutput);  // Binding 0: gLDR (RWTexture2D)
        DescriptorSet->WriteCombinedImageSampler(1, I_HDRInput, I_Sampler);  // Binding 1: gHDR (Texture2D)

        I_CommandBuffer->EnterComputePipeline(Pipeline);
        {
            I_CommandBuffer->BindDescriptorSet(0, DescriptorSet);

            FPushConstantRange PushConstants
            {
                .Width      = I_Width,
                .Height     = I_Height,
                .Exposure   = I_Exposure,
                .srgbOut    = I_bGammaCorrection ? 1U : 0U
            };
            I_CommandBuffer->PushConstants(PushConstants);

            // Dispatch compute shader (16x16 thread groups)
            constexpr UInt32 ThreadGroupSize = 16;
            UInt32 GroupCountX = (I_Width  + ThreadGroupSize - 1) / ThreadGroupSize;
            UInt32 GroupCountY = (I_Height + ThreadGroupSize - 1) / ThreadGroupSize;
            I_CommandBuffer->Dispatch(GroupCountX, GroupCountY, 1);
        }
        I_CommandBuffer->LeaveComputePipeline();
    }
}