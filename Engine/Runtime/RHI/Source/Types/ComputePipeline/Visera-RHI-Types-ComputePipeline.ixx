module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Types.ComputePipeline;
#define VISERA_MODULE_NAME "RHI.Types"
import Visera.RHI.Vulkan.Pipeline.Compute;
import Visera.RHI.Types.PipelineLayout;
import Visera.RHI.Types.Shader;

export namespace Visera
{
    using FRHIComputePipeline = FVulkanComputePipeline;

    class VISERA_RHI_API FRHIComputePipelineDesc
    {
    public:
        FRHIPipelineLayout     Layout;
        TSharedPtr<FRHIShader> ComputeShader;
    };
}