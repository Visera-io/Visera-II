module;
#include <Visera-RHI.hpp>
export module Visera.Runtime.RHI.Resource.Shader;
#define VISERA_MODULE_NAME "Runtime.RHI"
import Visera.Core.Containers.Array;
import Visera.Runtime.RHI.Common;
import Visera.Runtime.RHI.Registry.Item;
import Visera.Runtime.RHI.Vulkan.ShaderModule;

export namespace Visera
{
    class VISERA_RUNTIME_API FRHIShader : public IRHIRegistryItem
    {
    public:
        struct VISERA_RUNTIME_API FCreateInfo
        {
            TArray<FByte>     SPIRV;
            FRHIShaderLayout Reflection;

            Bool operator==(const FCreateInfo&) const = default;
        };

        [[nodiscard]] const FCreateInfo&
        GetInfo() const { return Info; }
        [[nodiscard]] const FRHIShaderLayout&
        GetLayout() const { return Info.Reflection; }
        [[nodiscard]] FVulkanShaderModule*
        GetVulkanShaderModule() { return &ShaderModule; }

    private:
        const FCreateInfo     Info;
        FVulkanShaderModule  ShaderModule;

    public:
        FRHIShader() = delete;
        FRHIShader(FCreateInfo&& I_CreateInfo, FVulkanShaderModule&& I_ShaderModule)
        : Info        {std::move(I_CreateInfo)},
          ShaderModule{std::move(I_ShaderModule)} {}
    };

    using FRHIShaderCreateInfo = FRHIShader::FCreateInfo;
}
