module;
#include <Visera-RHI.hpp>
export module Visera.RHI.Resource.Shader;
#define VISERA_MODULE_NAME "RHI.Resource"
import Visera.RHI.Common;
import Visera.RHI.Vulkan.ShaderModule;

export namespace Visera
{
    class VISERA_RHI_API FRHIShader
    {
    public:
        [[nodiscard]] const FRHIShaderLayout&
        GetLayout() const { return Layout; }
        [[nodiscard]] FVulkanShaderModule*
        GetVulkanShaderModule() { return &ShaderModule; }

    private:
        FRHIShaderLayout    Layout;
        FVulkanShaderModule ShaderModule;
    };
}