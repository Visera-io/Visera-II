module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.ComputePass;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Registry.Handle;
import Visera.Runtime.RHI.Vulkan.Pipeline.Compute;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHIComputePass : public IRHIRegistryItem
    {
    public:
        struct VISERA_RUNTIME_API FCreateInfo
        {
            FRHIShaderHandle ComputeShader;

            Bool operator==(const FCreateInfo& I_Other) const
            { return ComputeShader == I_Other.ComputeShader; }
            Bool IsCompatibleWith(const FCreateInfo& I_Other) const
            { return *this == I_Other; }
        };

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        [[nodiscard]] FVulkanComputePipeline*
        GetVulkanComputePipeline() { return &Pipeline; }

    private:
        const FCreateInfo        Info;
        FVulkanComputePipeline   Pipeline;

    public:
        FRHIComputePass() = delete;
        FRHIComputePass(FCreateInfo&& I_CreateInfo, FVulkanComputePipeline&& I_Pipeline)
        : Info    {std::move(I_CreateInfo)},
          Pipeline{std::move(I_Pipeline)} {}
    };

    using FRHIComputePassCreateInfo = FRHIComputePass::FCreateInfo;
}
