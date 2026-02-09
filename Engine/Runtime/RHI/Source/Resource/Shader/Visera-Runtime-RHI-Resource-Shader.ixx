module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.Shader;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Vulkan.ShaderModule;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHIShader : public IRHIRegistryItem
    {
    public:
        using FCreateInfo = FRHIShaderLayout;

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Layout; }
        [[nodiscard]] const FRHIShaderLayout&
        GetLayout() const { return Layout; }
        [[nodiscard]] FVulkanShaderModule*
        GetVulkanShaderModule() { return &ShaderModule; }

    private:
        FCreateInfo           Layout;
        FVulkanShaderModule   ShaderModule;

    public:
        FRHIShader() = delete;
        FRHIShader(FCreateInfo&& I_Layout, FVulkanShaderModule&& I_ShaderModule)
        : Layout     {std::move(I_Layout)},
          ShaderModule{std::move(I_ShaderModule)} {}
    };
}
